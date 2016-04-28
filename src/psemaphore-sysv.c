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

#include "plibsys-private.h"
#include "pmem.h"
#include "psemaphore.h"

#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <sys/ipc.h>

#define P_SEM_SUFFIX		"_p_sem_object"

struct sembuf sem_lock = {0, -1, SEM_UNDO};
struct sembuf sem_unlock = {0, 1, SEM_UNDO};

typedef union _p_semun {
	pint		val;
	struct semid_ds	*buf;
	pushort		*array;
} p_semun;

typedef int psem_hdl;
#define P_SEM_INVALID_HDL	-1

struct _PSemaphore {
	pboolean		file_created;
	pboolean		sem_created;
	key_t			unix_key;
	pchar			*platform_key;
	psem_hdl		sem_hdl;
	PSemaphoreAccessMode	mode;
	pint			init_val;
};

static pboolean __p_semaphore_create_handle (PSemaphore *sem, PError **error);
static void __p_semaphore_clean_handle (PSemaphore *sem);

static pboolean
__p_semaphore_create_handle (PSemaphore *sem, PError **error)
{
	pint	built;
	p_semun	semun_op;

	if (sem == NULL || sem->platform_key == NULL) {
		p_error_set_error_p (error,
				     (pint) P_ERROR_IPC_INVALID_ARGUMENT,
				     0,
				     "Invalid input argument");
		return FALSE;
	}

	if ((built = __p_ipc_unix_create_key_file (sem->platform_key)) == -1) {
		p_error_set_error_p (error,
				     (pint) __p_error_get_last_ipc (),
				     __p_error_get_last_error (),
				     "Failed to create key file");
		__p_semaphore_clean_handle (sem);
		return FALSE;
	} else if (built == 0)
		sem->file_created = TRUE;

	if ((sem->unix_key = __p_ipc_unix_get_ftok_key (sem->platform_key)) == -1) {
		p_error_set_error_p (error,
				     (pint) __p_error_get_last_ipc (),
				     __p_error_get_last_error (),
				     "Failed to get unique IPC key");
		__p_semaphore_clean_handle (sem);
		return FALSE;
	}

	if ((sem->sem_hdl = semget (sem->unix_key, 1, IPC_CREAT | IPC_EXCL | 0660)) == P_SEM_INVALID_HDL) {
		if (__p_error_get_last_error () == EEXIST)
			sem->sem_hdl = semget (sem->unix_key, 1, 0660);
	} else {
		sem->sem_created = TRUE;

		/* Maybe key file left after the crash, so take it */
		sem->file_created = (built == 1);
	}

	if (sem->sem_hdl == P_SEM_INVALID_HDL) {
		p_error_set_error_p (error,
				     (pint) __p_error_get_last_ipc (),
				     __p_error_get_last_error (),
				     "Failed to call semget() to create semaphore");
		__p_semaphore_clean_handle (sem);
		return FALSE;
	}

	if (sem->sem_created || sem->mode == P_SEM_ACCESS_CREATE) {
		semun_op.val = sem->init_val;

		if (semctl (sem->sem_hdl, 0, SETVAL, semun_op) == -1) {
			p_error_set_error_p (error,
					     (pint) __p_error_get_last_ipc (),
					     __p_error_get_last_error (),
					     "Failed to set semaphore initial value with semctl()");
			__p_semaphore_clean_handle (sem);
			return FALSE;
		}
	}

	return TRUE;
}

static void
__p_semaphore_clean_handle (PSemaphore *sem)
{
	if (sem->sem_hdl != P_SEM_INVALID_HDL && sem->sem_created && semctl (sem->sem_hdl, 0, IPC_RMID) == -1)
		P_ERROR ("PSemaphore: failed to perform IPC_RMID");

	if (sem->file_created && sem->platform_key && unlink (sem->platform_key) == -1)
		P_ERROR ("PSemaphore: failed to remove key file");

	sem->file_created = FALSE;
	sem->sem_created = FALSE;
	sem->unix_key = -1;
	sem->sem_hdl = P_SEM_INVALID_HDL;
}

P_LIB_API PSemaphore *
p_semaphore_new (const pchar		*name,
		 pint			init_val,
		 PSemaphoreAccessMode	mode,
		 PError			**error)
{
	PSemaphore *ret;
	pchar *new_name;

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

	ret->platform_key = __p_ipc_get_platform_key (new_name, FALSE);
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

	while ((res = semop (sem->sem_hdl, &sem_lock, 1)) == -1 && __p_error_get_last_error () == EINTR)
		;

	ret = (res == 0);

	if (!ret && (__p_error_get_last_error () == EIDRM || __p_error_get_last_error () == EINVAL)) {
		P_WARNING ("PSemaphore: trying to recreate");
		__p_semaphore_clean_handle (sem);

		if (!__p_semaphore_create_handle (sem, error))
			return FALSE;

		while ((res = semop (sem->sem_hdl, &sem_lock, 1)) == -1 && __p_error_get_last_error () == EINTR)
			;

		ret = (res == 0);
	}

	if (!ret)
		p_error_set_error_p (error,
				     (pint) __p_error_get_last_ipc (),
				     __p_error_get_last_error (),
				     "Failed to call semop() on semaphore");

	return ret;
}

P_LIB_API pboolean
p_semaphore_release (PSemaphore *sem,
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

	while ((res = semop (sem->sem_hdl, &sem_unlock, 1)) == -1 && __p_error_get_last_error () == EINTR)
		;

	ret = (res == 0);

	if (!ret && (__p_error_get_last_error () == EIDRM || __p_error_get_last_error () == EINVAL)) {
		P_WARNING ("PSemaphore: trying to recreate");
		__p_semaphore_clean_handle (sem);

		if (!__p_semaphore_create_handle (sem, error))
			return FALSE;

		return TRUE;
	}

	if (!ret)
		p_error_set_error_p (error,
				     (pint) __p_error_get_last_ipc (),
				     __p_error_get_last_error (),
				     "Failed to call semop() on semaphore");

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
