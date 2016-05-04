/*
 * Copyright (C) 2010-2016 Alexander Saprykin <xelfium@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

#include "plibsys-private.h"
#include "pmem.h"
#include "psemaphore.h"

#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <fcntl.h>
#include <semaphore.h>
#include <errno.h>

#define P_SEM_SUFFIX		"_p_sem_object"

typedef sem_t psem_hdl;

/* On some HP-UX versions it may not be defined */
#ifndef SEM_FAILED
#define SEM_FAILED ((sem_t *) -1)
#endif

#ifdef P_OS_SOLARIS
#define P_SEM_INVALID_HDL	(sem_t *) -1
#else
#define P_SEM_INVALID_HDL	SEM_FAILED
#endif

struct _PSemaphore {
	pboolean		sem_created;
	pchar			*platform_key;
	psem_hdl		*sem_hdl;
	PSemaphoreAccessMode	mode;
	pint			init_val;
};

static pboolean __p_semaphore_create_handle (PSemaphore *sem, PError **error);
static void __p_semaphore_clean_handle (PSemaphore *sem);

static pboolean
__p_semaphore_create_handle (PSemaphore *sem,
			     PError	**error)
{
	if (sem == NULL || sem->platform_key == NULL) {
		p_error_set_error_p (error,
				     (pint) P_ERROR_IPC_INVALID_ARGUMENT,
				     0,
				     "Invalid input argument");
		return FALSE;
	}

	/* Solaris may interrupt sem_open() call */
	while ((sem->sem_hdl = sem_open (sem->platform_key, O_CREAT | O_EXCL, 0660, sem->init_val)) == P_SEM_INVALID_HDL
		&& __p_error_get_last_error () == EINTR)
	;

	if (sem->sem_hdl == P_SEM_INVALID_HDL) {
		if (__p_error_get_last_error () == EEXIST) {
			if (sem->mode == P_SEM_ACCESS_CREATE)
				sem_unlink (sem->platform_key);

			while ((sem->sem_hdl = sem_open (sem->platform_key, 0, 0, 0)) == P_SEM_INVALID_HDL
				&& __p_error_get_last_error () == EINTR)
			;
		}
	} else
		sem->sem_created = TRUE;

	if (sem->sem_hdl == P_SEM_INVALID_HDL) {
		p_error_set_error_p (error,
				     (pint) __p_error_get_last_ipc (),
				     __p_error_get_last_error (),
				     "Failed to call sem_open() to create semaphore");
		__p_semaphore_clean_handle (sem);
		return FALSE;
	}

	return TRUE;
}

static void
__p_semaphore_clean_handle (PSemaphore *sem)
{
	if (sem->sem_hdl != P_SEM_INVALID_HDL && sem_close (sem->sem_hdl) == -1)
		P_ERROR ("PSemaphore: failed to perform sem_close()");

	if (sem->sem_hdl != P_SEM_INVALID_HDL && sem->sem_created && sem_unlink (sem->platform_key) == -1)
		P_ERROR ("PSemaphore: failed to perform sem_unlink()");

	sem->sem_created = FALSE;
	sem->sem_hdl = P_SEM_INVALID_HDL;
}

P_LIB_API PSemaphore *
p_semaphore_new (const pchar		*name,
		 pint			init_val,
		 PSemaphoreAccessMode	mode,
		 PError			**error)
{
	PSemaphore	*ret;
	pchar		*new_name;

	if (name == NULL || init_val < 0) {
		p_error_set_error_p (error,
				     (pint) P_ERROR_IPC_INVALID_ARGUMENT,
				     0,
				     "Invalid input argument");
		return NULL;
	}

	if ((ret = p_malloc0 (sizeof (PSemaphore))) == NULL) {
		p_error_set_error_p (error,
				     (pint) P_ERROR_IPC_NO_RESOURCES,
				     0,
				     "Failed to allocate memory for semaphore");
		return NULL;
	}

	if ((new_name = p_malloc0 (strlen (name) + strlen (P_SEM_SUFFIX) + 1)) == NULL) {
		p_error_set_error_p (error,
				     (pint) P_ERROR_IPC_NO_RESOURCES,
				     0,
				     "Failed to allocate memory for semaphore");
		p_free (ret);
		return NULL;
	}

	strcpy (new_name, name);
	strcat (new_name, P_SEM_SUFFIX);

#ifdef P_OS_IRIX
	/* IRIX uses filename styled IPC names */
	ret->platform_key = __p_ipc_get_platform_key (new_name, FALSE);
#else
	ret->platform_key = __p_ipc_get_platform_key (new_name, TRUE);
#endif
	ret->init_val = init_val;
	ret->mode = mode;

	p_free (new_name);

	if (!__p_semaphore_create_handle (ret, error)) {
		p_semaphore_free (ret);
		return NULL;
	}

	return ret;
}

P_LIB_API void
p_semaphore_take_ownership (PSemaphore *sem)
{
	if (sem == NULL)
		return;

	sem->sem_created = TRUE;
}

P_LIB_API pboolean
p_semaphore_acquire (PSemaphore *sem,
		     PError	**error)
{
	pboolean	ret;
	pint		res;

	if (sem == NULL) {
		p_error_set_error_p (error,
				     (pint) P_ERROR_IPC_INVALID_ARGUMENT,
				     0,
				     "Invalid input argument");
		return FALSE;
	}

	while ((res = sem_wait (sem->sem_hdl)) == -1 && __p_error_get_last_error () == EINTR)
		;

	ret = (res == 0);

	if (!ret)
		p_error_set_error_p (error,
				     (pint) __p_error_get_last_ipc (),
				     __p_error_get_last_error (),
				     "Failed to call sem_wait() on semaphore");

	return ret;
}

P_LIB_API pboolean
p_semaphore_release (PSemaphore *sem,
		     PError	**error)
{
	pboolean ret;

	if (sem == NULL) {
		p_error_set_error_p (error,
				     (pint) P_ERROR_IPC_INVALID_ARGUMENT,
				     0,
				     "Invalid input argument");
		return FALSE;
	}

	ret = (sem_post (sem->sem_hdl) == 0);

	if (!ret)
		p_error_set_error_p (error,
				     (pint) __p_error_get_last_ipc (),
				     __p_error_get_last_error (),
				     "Failed to call sem_post() on semaphore");

	return ret;
}

P_LIB_API void
p_semaphore_free (PSemaphore *sem)
{
	if (sem == NULL)
		return;

	__p_semaphore_clean_handle (sem);

	if (sem->platform_key)
		p_free (sem->platform_key);

	p_free (sem);
}
