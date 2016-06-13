/*
 * Copyright (C) 2010-2016 Alexander Saprykin <xelfium@gmail.com>
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, see <http://www.gnu.org/licenses/>.
 */

#include "pmem.h"
#include "psemaphore.h"
#include "perror-private.h"
#include "pipc-private.h"

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
#define P_SEM_INVALID_HDL	-1

struct sembuf sem_lock = {0, -1, SEM_UNDO};
struct sembuf sem_unlock = {0, 1, SEM_UNDO};

typedef union p_semun_ {
	pint		val;
	struct semid_ds	*buf;
	pushort		*array;
} p_semun;

typedef int psem_hdl;

struct PSemaphore_ {
	pboolean		file_created;
	pboolean		sem_created;
	key_t			unix_key;
	pchar			*platform_key;
	psem_hdl		sem_hdl;
	PSemaphoreAccessMode	mode;
	pint			init_val;
};

static pboolean pp_semaphore_create_handle (PSemaphore *sem, PError **error);
static void pp_semaphore_clean_handle (PSemaphore *sem);

static pboolean
pp_semaphore_create_handle (PSemaphore *sem, PError **error)
{
	pint	built;
	p_semun	semun_op;

	if (P_UNLIKELY (sem == NULL || sem->platform_key == NULL)) {
		p_error_set_error_p (error,
				     (pint) P_ERROR_IPC_INVALID_ARGUMENT,
				     0,
				     "Invalid input argument");
		return FALSE;
	}

	if (P_UNLIKELY ((built = p_ipc_unix_create_key_file (sem->platform_key)) == -1)) {
		p_error_set_error_p (error,
				     (pint) p_error_get_last_ipc (),
				     p_error_get_last_error (),
				     "Failed to create key file");
		pp_semaphore_clean_handle (sem);
		return FALSE;
	} else if (built == 0)
		sem->file_created = TRUE;

	if (P_UNLIKELY ((sem->unix_key = p_ipc_unix_get_ftok_key (sem->platform_key)) == -1)) {
		p_error_set_error_p (error,
				     (pint) p_error_get_last_ipc (),
				     p_error_get_last_error (),
				     "Failed to get unique IPC key");
		pp_semaphore_clean_handle (sem);
		return FALSE;
	}

	if ((sem->sem_hdl = semget (sem->unix_key,
				    1,
				    IPC_CREAT | IPC_EXCL | 0660)) == P_SEM_INVALID_HDL) {
		if (p_error_get_last_error () == EEXIST)
			sem->sem_hdl = semget (sem->unix_key, 1, 0660);
	} else {
		sem->sem_created = TRUE;

		/* Maybe key file left after the crash, so take it */
		sem->file_created = (built == 1);
	}

	if (P_UNLIKELY (sem->sem_hdl == P_SEM_INVALID_HDL)) {
		p_error_set_error_p (error,
				     (pint) p_error_get_last_ipc (),
				     p_error_get_last_error (),
				     "Failed to call semget() to create semaphore");
		pp_semaphore_clean_handle (sem);
		return FALSE;
	}

	if (sem->sem_created == TRUE || sem->mode == P_SEM_ACCESS_CREATE) {
		semun_op.val = sem->init_val;

		if (P_UNLIKELY (semctl (sem->sem_hdl, 0, SETVAL, semun_op) == -1)) {
			p_error_set_error_p (error,
					     (pint) p_error_get_last_ipc (),
					     p_error_get_last_error (),
					     "Failed to set semaphore initial value with semctl()");
			pp_semaphore_clean_handle (sem);
			return FALSE;
		}
	}

	return TRUE;
}

static void
pp_semaphore_clean_handle (PSemaphore *sem)
{
	if (sem->sem_hdl != P_SEM_INVALID_HDL &&
	    sem->sem_created == TRUE &&
	    semctl (sem->sem_hdl, 0, IPC_RMID) == -1)
		P_ERROR ("PSemaphore: failed to perform IPC_RMID");

	if (sem->file_created == TRUE &&
	    sem->platform_key != NULL &&
	    unlink (sem->platform_key) == -1)
		P_ERROR ("PSemaphore: failed to remove key file");

	sem->file_created = FALSE;
	sem->sem_created  = FALSE;
	sem->unix_key     = -1;
	sem->sem_hdl      = P_SEM_INVALID_HDL;
}

P_LIB_API PSemaphore *
p_semaphore_new (const pchar		*name,
		 pint			init_val,
		 PSemaphoreAccessMode	mode,
		 PError			**error)
{
	PSemaphore	*ret;
	pchar		*new_name;

	if (P_UNLIKELY (name == NULL || init_val < 0)) {
		p_error_set_error_p (error,
				     (pint) P_ERROR_IPC_INVALID_ARGUMENT,
				     0,
				     "Invalid input argument");
		return NULL;
	}

	if (P_UNLIKELY ((ret = p_malloc0 (sizeof (PSemaphore))) == NULL)) {
		p_error_set_error_p (error,
				     (pint) P_ERROR_IPC_NO_RESOURCES,
				     0,
				     "Failed to allocate memory for semaphore");
		return NULL;
	}

	if (P_UNLIKELY ((new_name = p_malloc0 (strlen (name) + strlen (P_SEM_SUFFIX) + 1)) == NULL)) {
		p_error_set_error_p (error,
				     (pint) P_ERROR_IPC_NO_RESOURCES,
				     0,
				     "Failed to allocate memory for semaphore");
		p_free (ret);
		return NULL;
	}

	strcpy (new_name, name);
	strcat (new_name, P_SEM_SUFFIX);

	ret->platform_key = p_ipc_get_platform_key (new_name, FALSE);
	ret->init_val     = init_val;
	ret->mode         = mode;

	p_free (new_name);

	if (P_UNLIKELY (pp_semaphore_create_handle (ret, error) == FALSE)) {
		p_semaphore_free (ret);
		return NULL;
	}

	return ret;
}

P_LIB_API void
p_semaphore_take_ownership (PSemaphore *sem)
{
	if (P_UNLIKELY (sem == NULL))
		return;

	sem->sem_created = TRUE;
}

P_LIB_API pboolean
p_semaphore_acquire (PSemaphore *sem,
		     PError	**error)
{
	pboolean	ret;
	pint		res;

	if (P_UNLIKELY (sem == NULL)) {
		p_error_set_error_p (error,
				     (pint) P_ERROR_IPC_INVALID_ARGUMENT,
				     0,
				     "Invalid input argument");
		return FALSE;
	}

	while ((res = semop (sem->sem_hdl, &sem_lock, 1)) == -1 &&
		p_error_get_last_error () == EINTR)
		;

	ret = (res == 0);

	if (P_UNLIKELY (ret == FALSE &&
			(p_error_get_last_error () == EIDRM ||
			 p_error_get_last_error () == EINVAL))) {
		P_WARNING ("PSemaphore: trying to recreate");
		pp_semaphore_clean_handle (sem);

		if (P_UNLIKELY (pp_semaphore_create_handle (sem, error) == FALSE))
			return FALSE;

		while ((res = semop (sem->sem_hdl, &sem_lock, 1)) == -1 &&
			p_error_get_last_error () == EINTR)
			;

		ret = (res == 0);
	}

	if (P_UNLIKELY (ret == FALSE))
		p_error_set_error_p (error,
				     (pint) p_error_get_last_ipc (),
				     p_error_get_last_error (),
				     "Failed to call semop() on semaphore");

	return ret;
}

P_LIB_API pboolean
p_semaphore_release (PSemaphore *sem,
		     PError	**error)
{
	pboolean	ret;
	pint		res;

	if (P_UNLIKELY (sem == NULL)) {
		p_error_set_error_p (error,
				     (pint) P_ERROR_IPC_INVALID_ARGUMENT,
				     0,
				     "Invalid input argument");
		return FALSE;
	}

	while ((res = semop (sem->sem_hdl, &sem_unlock, 1)) == -1 &&
		p_error_get_last_error () == EINTR)
		;

	ret = (res == 0);

	if (P_UNLIKELY (ret == FALSE &&
			(p_error_get_last_error () == EIDRM ||
			 p_error_get_last_error () == EINVAL))) {
		P_WARNING ("PSemaphore: trying to recreate");
		pp_semaphore_clean_handle (sem);

		if (P_UNLIKELY (pp_semaphore_create_handle (sem, error) == FALSE))
			return FALSE;

		return TRUE;
	}

	if (P_UNLIKELY (ret == FALSE))
		p_error_set_error_p (error,
				     (pint) p_error_get_last_ipc (),
				     p_error_get_last_error (),
				     "Failed to call semop() on semaphore");

	return ret;
}

P_LIB_API void
p_semaphore_free (PSemaphore *sem)
{
	if (P_UNLIKELY (sem == NULL))
		return;

	pp_semaphore_clean_handle (sem);

	if (P_LIKELY (sem->platform_key != NULL))
		p_free (sem->platform_key);

	p_free (sem);
}
