/*
 * 08.11.2010
 * Copyright (C) 2010 Alexander Saprykin <xelfium@gmail.com>
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

#include "pmem.h"
#include "psemaphore.h"
#include "psha1.h"

#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <sys/ipc.h>

#ifndef P_OS_WIN
extern pchar *	p_ipc_unix_get_temp_dir		(void);
extern pint	p_ipc_unix_create_key_file	(const pchar		*file_name);
extern pint	p_ipc_unix_get_ftok_key		(const pchar		*file_name);
#endif

extern pchar *	p_ipc_get_platform_key		(const pchar		*name,
						 pboolean		posix);

#define P_SEM_SUFFIX		"_p_sem_object"
#define P_SEM_ERROR_BUF_SIZE	255

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
	PSemaphoreError		error;
	pchar			error_str[P_SEM_ERROR_BUF_SIZE];
	pint			init_val;
};

static pboolean create_handle (PSemaphore *sem);
static void clean_handle (PSemaphore *sem);

static pboolean
create_handle (PSemaphore *sem)
{
	pint	built;
	p_semun	semun_op;

	if (sem == NULL || sem->platform_key == NULL)
		return FALSE;

	if ((built = p_ipc_unix_create_key_file (sem->platform_key)) == -1) {
		P_ERROR ("PSemaphore: failed to create key file");
		clean_handle (sem);
		return FALSE;
	} else if (built == 0)
		sem->file_created = TRUE;

	if ((sem->unix_key = p_ipc_unix_get_ftok_key (sem->platform_key)) == -1) {
		P_ERROR ("PSemaphore: failed to get unique IPC key");
		clean_handle (sem);
		return FALSE;
	}

	if ((sem->sem_hdl = semget (sem->unix_key, 1, IPC_CREAT | IPC_EXCL | 0660)) == P_SEM_INVALID_HDL) {
		if (errno == EEXIST)
			sem->sem_hdl = semget (sem->unix_key, 1, 0660);
	} else {
		sem->sem_created = TRUE;

		/* Maybe key file left after the crash, so take it */
		sem->file_created = (built == 1);
	}

	if (sem->sem_hdl == P_SEM_INVALID_HDL) {
		P_ERROR ("PSemaphore: semget failed");
		clean_handle (sem);
		return FALSE;
	}

	if (sem->sem_created || sem->mode == P_SEM_ACCESS_CREATE) {
		semun_op.val = sem->init_val;
		if (semctl (sem->sem_hdl, 0, SETVAL, semun_op) == -1) {
			P_ERROR ("PSemaphore: failed to set initial value");
			clean_handle (sem);
			return FALSE;
		}
	}

	return TRUE;
}

static void
clean_handle (PSemaphore *sem)
{
	if (sem == NULL)
		return;

	if (sem->sem_hdl != P_SEM_INVALID_HDL && sem->sem_created && semctl (sem->sem_hdl, 0, IPC_RMID) == -1)
		P_ERROR ("PSemaphore: failed to perform IPC_RMID");

	if (sem->file_created && sem->platform_key && unlink (sem->platform_key) == -1)
		P_ERROR ("PSemaphore: failed to remove key file");

	sem->file_created = FALSE;
	sem->sem_created = FALSE;
	sem->unix_key = -1;
	sem->sem_hdl = P_SEM_INVALID_HDL;
	sem->error = P_SEM_ERROR_NONE;
	sem->error_str[0] = '\0';
}


P_LIB_API PSemaphore *
p_semaphore_new (const pchar *name,  pint init_val, PSemaphoreAccessMode mode)
{
	PSemaphore *ret;
	pchar *new_name;

	if (name == NULL || init_val < 0)
		return NULL;

	if ((ret = p_malloc0 (sizeof (PSemaphore))) == NULL) {
		P_ERROR ("PSemaphore: failed to allocate memory");
		return NULL;
	}

	if ((new_name = p_malloc0 (strlen (name) + strlen (P_SEM_SUFFIX) + 1)) == NULL) {
		P_ERROR ("PSemaphore: failed to allocate memory");
		p_free (ret);
		return NULL;
	}

	strcpy (new_name, name);
	strcat (new_name, P_SEM_SUFFIX);

	ret->platform_key = p_ipc_get_platform_key (new_name, FALSE);
	ret->init_val = init_val;
	ret->mode = mode;

	p_free (new_name);

	if (!create_handle (ret)) {
		P_ERROR ("PSemaphore: failed to create system handle");
		p_semaphore_free (ret);
		return NULL;
	}

	return ret;
}

P_LIB_API pboolean
p_semaphore_acquire (PSemaphore *sem)
{
	pboolean	ret;
	pint		res;

	if (sem == NULL)
		return FALSE;

	while ((res = semop (sem->sem_hdl, &sem_lock, 1)) == -1 && errno == EINTR)
		;

	ret = (res == 0);

	if (!ret && (errno == EIDRM || errno == EINVAL)) {
		P_WARNING ("PSemaphore: trying to recreate");
		clean_handle (sem);

		if (!create_handle (sem))
			return FALSE;

		ret = p_semaphore_acquire (sem);
	}

	if (!ret)
		P_ERROR ("PSemaphore: failed to acquire lock");

	return ret;
}

P_LIB_API pboolean
p_semaphore_release (PSemaphore *sem)
{
	pboolean	ret;
	pint		res;

	if (sem == NULL)
		return FALSE;

	while ((res = semop (sem->sem_hdl, &sem_unlock, 1)) == -1 && errno == EINTR)
		;

	ret = (res == 0);

	if (!ret && (errno == EIDRM || errno == EINVAL)) {
		P_WARNING ("PSemaphore: trying to recreate");
		clean_handle (sem);

		if (!create_handle (sem))
			return FALSE;

		return TRUE;
	}

	if (!ret)
		P_ERROR ("PSemaphore: failed to release lock");

	return ret;
}

P_LIB_API void
p_semaphore_free (PSemaphore *sem)
{
	if (sem == NULL)
		return;

	clean_handle (sem);

	if (sem->platform_key)
		p_free (sem->platform_key);

	p_free (sem);
}

