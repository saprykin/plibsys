/*
 * Copyright (C) 2010-2013 Alexander Saprykin <xelfium@gmail.com>
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

/* TODO: error report system */

#include "plib-private.h"
#include "pmem.h"
#include "pshm.h"
#include "psemaphore.h"

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>

#define P_SHM_SUFFIX		"_p_shm_object"
#define P_SHM_INVALID_HDL	-1

struct _PShm {
	pboolean	shm_created;
	pchar		*platform_key;
	ppointer	addr;
	psize		size;
	PSemaphore	*sem;
	PShmAccessPerms perms;
};

static pboolean __p_shm_create_handle (PShm *shm);
static void __p_shm_clean_handle (PShm *shm);

static pboolean
__p_shm_create_handle (PShm *shm)
{
	pboolean	is_exists;
	pint		fd, flags;
	struct stat	stat_buf;

	if (shm == NULL || shm->platform_key == NULL)
		return FALSE;

	is_exists = FALSE;

	while ((fd = shm_open (shm->platform_key, O_CREAT | O_EXCL | O_RDWR, 0660)) == P_SHM_INVALID_HDL
	       && errno == EINTR)
	;

	if (fd == P_SHM_INVALID_HDL) {
		if (errno == EEXIST) {
			is_exists = TRUE;

			while ((fd = shm_open (shm->platform_key, O_RDWR, 0660)) == P_SHM_INVALID_HDL
			       && errno == EINTR)
			;

			if (fd == P_SHM_INVALID_HDL) {
				P_ERROR ("PShm: shm_open failed");
				__p_shm_clean_handle (shm);
				return FALSE;
			}
		}
	} else
		shm->shm_created = TRUE;

	/* Try to get size of the existing file descriptor */
	if (is_exists) {
		if (fstat (fd, &stat_buf) == -1) {
			P_ERROR ("PShm: failed to get region size");
			close (fd);
			__p_shm_clean_handle (shm);
			return FALSE;
		}

		shm->size = stat_buf.st_size;
	} else {
		if ((ftruncate (fd, shm->size)) == -1) {
			P_ERROR ("PShm: failed to truncate file");
			close (fd);
			__p_shm_clean_handle (shm);
			return FALSE;
		}
	}

	flags = (shm->perms == P_SHM_ACCESS_READONLY) ? PROT_READ : PROT_READ | PROT_WRITE;

	if ((shm->addr = mmap (NULL, shm->size, flags, MAP_SHARED, fd, 0)) == (void *) -1) {
		P_ERROR ("PShm: mmap failed");
		shm->addr = NULL;
		close (fd);
		__p_shm_clean_handle (shm);
		return FALSE;
	}

	if (close (fd) == -1)
		P_WARNING ("PShm: failed to close file descriptor");

	if ((shm->sem = p_semaphore_new (shm->platform_key, 1,
					 is_exists ? P_SEM_ACCESS_OPEN : P_SEM_ACCESS_CREATE)) == NULL) {
		P_ERROR ("PShm: failed create PSemaphore object");
		__p_shm_clean_handle (shm);
		return FALSE;
	}

	return TRUE;
}

static void
__p_shm_clean_handle (PShm *shm)
{
	if (shm == NULL)
		return;

	if (shm->addr != NULL && munmap (shm->addr, shm->size) == -1)
		P_ERROR ("PShm: failed to unmap shared memory");

	if (shm->shm_created && shm_unlink (shm->platform_key) == -1)
		P_ERROR ("PShm: failed to unlink shared memory");

	shm->shm_created = FALSE;

	if (shm->sem) {
		p_semaphore_free (shm->sem);
		shm->sem = NULL;
	}

	shm->addr = NULL;
	shm->size = 0;
}

P_LIB_API PShm *
p_shm_new (const pchar		*name,
	   psize		size,
	   PShmAccessPerms	perms)
{
	PShm *ret;
	pchar *new_name;


	if (name == NULL)
		return NULL;

	if ((ret = p_malloc0 (sizeof (PShm))) == NULL) {
		P_ERROR ("PShm: failed to allocate memory");
		return NULL;
	}

	if ((new_name = p_malloc0 (strlen (name) + strlen (P_SHM_SUFFIX) + 1)) == NULL) {
		P_ERROR ("PShm: failed to allocate memory");
		p_shm_free (ret);
		return NULL;
	}

	strcpy (new_name, name);
	strcat (new_name, P_SHM_SUFFIX);

	ret->platform_key = __p_ipc_get_platform_key (new_name, TRUE);
	ret->perms = perms;
	ret->size = size;

	p_free (new_name);

	if (!__p_shm_create_handle (ret)) {
		P_ERROR ("PShm: failed to create system handle");
		p_shm_free (ret);
		return NULL;
	}

	if (ret->size > size && size != 0)
		ret->size = size;

	return ret;
}

P_LIB_API void
p_shm_take_ownership (PShm *shm)
{
	if (shm == NULL)
		return;

	shm->shm_created = TRUE;
	p_semaphore_take_ownership (shm->sem);
}

P_LIB_API void
p_shm_free (PShm *shm)
{
	if (shm == NULL)
		return;

	__p_shm_clean_handle (shm);

	if (shm->platform_key)
		p_free (shm->platform_key);

	p_free (shm);
}

P_LIB_API pboolean
p_shm_lock (PShm *shm)
{
	if (shm == NULL)
		return FALSE;

	if (!p_semaphore_acquire (shm->sem)) {
		P_ERROR ("PShm: failed to lock memory");
		return FALSE;
	} else
		return TRUE;
}

P_LIB_API pboolean
p_shm_unlock (PShm *shm)
{
	if (shm == NULL)
		return FALSE;

	if (!p_semaphore_release (shm->sem)) {
		P_ERROR ("PShm: failed to unlock memory");
		return FALSE;
	} else
		return TRUE;
}

P_LIB_API ppointer
p_shm_get_address (const PShm *shm)
{
	if (shm == NULL)
		return NULL;

	return shm->addr;
}

P_LIB_API psize
p_shm_get_size (const PShm *shm)
{
	if (shm == NULL)
		return 0;

	return shm->size;
}
