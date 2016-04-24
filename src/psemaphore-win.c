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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA.
 */

#include "plib-private.h"
#include "pmem.h"
#include "psemaphore.h"

#include <stdlib.h>
#include <string.h>

#include <windows.h>

#define P_SEM_SUFFIX		"_p_sem_object"

typedef HANDLE psem_hdl;
#define P_SEM_INVALID_HDL	NULL

struct _PSemaphore {
	pchar			*platform_key;
	psem_hdl		sem_hdl;
	pint			init_val;
};

static pboolean __p_semaphore_create_handle (PSemaphore *sem, PError **error);
static void __p_semaphore_clean_handle (PSemaphore *sem);

static pboolean
__p_semaphore_create_handle (PSemaphore *sem, PError **error)
{
	if (sem == NULL || sem->platform_key == NULL) {
		p_error_set_error_p (error,
				     (pint) P_ERROR_IPC_INVALID_ARGUMENT,
				     0,
				     "Invalid input argument");
		return FALSE;
	}

	/* Multibyte character set must be enabled in MS VS */
	if ((sem->sem_hdl = CreateSemaphoreA (NULL, sem->init_val, MAXLONG, sem->platform_key)) == NULL) {
		p_error_set_error_p (error,
				     (pint) __p_error_get_last_ipc (),
				     __p_error_get_last_error (),
				     "Failed to call CreateSemaphore() to create semaphore");
		return FALSE;
	}

	return TRUE;
}

static void
__p_semaphore_clean_handle (PSemaphore *sem)
{
	if (sem->sem_hdl && !CloseHandle (sem->sem_hdl))
		P_ERROR ("PSemaphore: CloseHandle() failed");

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

	P_UNUSED (mode);

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
	strcpy (new_name, P_SEM_SUFFIX);

	ret->platform_key = __p_ipc_get_platform_key (new_name, FALSE);
	ret->init_val = init_val;

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
	P_UNUSED (sem);
}

P_LIB_API pboolean
p_semaphore_acquire (PSemaphore *sem,
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

	ret = (WaitForSingleObject (sem->sem_hdl, INFINITE) == WAIT_OBJECT_0) ? TRUE : FALSE;

	if (!ret)
		p_error_set_error_p (error,
				     (pint) __p_error_get_last_ipc (),
				     __p_error_get_last_error (),
				     "Failed to call WaitForSingleObject() on semaphore");

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

	ret = ReleaseSemaphore (sem->sem_hdl, 1, NULL) ? TRUE : FALSE;

	if (!ret)
		p_error_set_error_p (error,
				     (pint) __p_error_get_last_ipc (),
				     __p_error_get_last_error (),
				     "Failed to call ReleaseSemaphore() on semaphore");

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
