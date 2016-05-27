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
#include "pshm.h"
#include "psemaphore.h"

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <errno.h>

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

static pboolean __p_shm_create_handle (PShm *shm, PError **error);
static void __p_shm_clean_handle (PShm *shm);

static pboolean
__p_shm_create_handle (PShm	*shm,
		       PError	**error)
{
	pboolean	is_exists;
	pint		fd, flags;
	struct stat	stat_buf;

	if (P_UNLIKELY (shm == NULL || shm->platform_key == NULL)) {
		p_error_set_error_p (error,
				     (pint) P_ERROR_IPC_INVALID_ARGUMENT,
				     0,
				     "Invalid input argument");
		return FALSE;
	}

	is_exists = FALSE;

	while ((fd = shm_open (shm->platform_key, O_CREAT | O_EXCL | O_RDWR, 0660)) == P_SHM_INVALID_HDL
	       && p_error_get_last_error () == EINTR)
	;

	if (fd == P_SHM_INVALID_HDL) {
		if (p_error_get_last_error () == EEXIST) {
			is_exists = TRUE;

			while ((fd = shm_open (shm->platform_key, O_RDWR, 0660)) == P_SHM_INVALID_HDL
			       && p_error_get_last_error () == EINTR)
			;
		}
	} else
		shm->shm_created = TRUE;

	if (P_UNLIKELY (fd == P_SHM_INVALID_HDL)) {
		p_error_set_error_p (error,
				     (pint) p_error_get_last_ipc (),
				     p_error_get_last_error (),
				     "Failed to call shm_open() to create memory segment");
		__p_shm_clean_handle (shm);
		return FALSE;
	}

	/* Try to get size of the existing file descriptor */
	if (is_exists) {
		if (P_UNLIKELY (fstat (fd, &stat_buf) == -1)) {
			p_error_set_error_p (error,
					     (pint) p_error_get_last_ipc (),
					     p_error_get_last_error (),
					     "Failed to call fstat() to get memory segment size");
			close (fd);
			__p_shm_clean_handle (shm);
			return FALSE;
		}

		shm->size = stat_buf.st_size;
	} else {
		if (P_UNLIKELY ((ftruncate (fd, shm->size)) == -1)) {
			p_error_set_error_p (error,
					     (pint) p_error_get_last_ipc (),
					     p_error_get_last_error (),
					     "Failed to call ftruncate() to set memory segment size");
			close (fd);
			__p_shm_clean_handle (shm);
			return FALSE;
		}
	}

	flags = (shm->perms == P_SHM_ACCESS_READONLY) ? PROT_READ : PROT_READ | PROT_WRITE;

	if (P_UNLIKELY ((shm->addr = mmap (NULL, shm->size, flags, MAP_SHARED, fd, 0)) == (void *) -1)) {
		p_error_set_error_p (error,
				     (pint) p_error_get_last_ipc (),
				     p_error_get_last_error (),
				     "Failed to call mmap() to map memory segment");
		shm->addr = NULL;
		close (fd);
		__p_shm_clean_handle (shm);
		return FALSE;
	}

	if (P_UNLIKELY (close (fd) == -1))
		P_WARNING ("PShm: failed to close file descriptor");

	if (P_UNLIKELY ((shm->sem = p_semaphore_new (shm->platform_key, 1,
						     is_exists ? P_SEM_ACCESS_OPEN : P_SEM_ACCESS_CREATE,
						     error)) == NULL)) {
		__p_shm_clean_handle (shm);
		return FALSE;
	}

	return TRUE;
}

static void
__p_shm_clean_handle (PShm *shm)
{
	if (P_UNLIKELY (shm->addr != NULL && munmap (shm->addr, shm->size) == -1))
		P_ERROR ("PShm: failed to unmap shared memory with munmap()");

	if (shm->shm_created == TRUE && shm_unlink (shm->platform_key) == -1)
		P_ERROR ("PShm: failed to unlink shared memory with shm_unlink()");

	if (P_LIKELY (shm->sem != NULL)) {
		p_semaphore_free (shm->sem);
		shm->sem         = NULL;
	}

	shm->shm_created = FALSE;
	shm->addr        = NULL;
	shm->size        = 0;
}

P_LIB_API PShm *
p_shm_new (const pchar		*name,
	   psize		size,
	   PShmAccessPerms	perms,
	   PError		**error)
{
	PShm	*ret;
	pchar	*new_name;

	if (P_UNLIKELY (name == NULL)) {
		p_error_set_error_p (error,
				     (pint) P_ERROR_IPC_INVALID_ARGUMENT,
				     0,
				     "Invalid input argument");
		return NULL;
	}

	if (P_UNLIKELY ((ret = p_malloc0 (sizeof (PShm))) == NULL)) {
		p_error_set_error_p (error,
				     (pint) P_ERROR_IPC_NO_RESOURCES,
				     0,
				     "Failed to allocate memory for shared segment");
		return NULL;
	}

	if (P_UNLIKELY ((new_name = p_malloc0 (strlen (name) + strlen (P_SHM_SUFFIX) + 1)) == NULL)) {
		p_error_set_error_p (error,
				     (pint) P_ERROR_IPC_NO_RESOURCES,
				     0,
				     "Failed to allocate memory for segment name");
		p_shm_free (ret);
		return NULL;
	}

	strcpy (new_name, name);
	strcat (new_name, P_SHM_SUFFIX);

#ifdef P_OS_IRIX
	/* IRIX uses filename styled IPC names */
	ret->platform_key = __p_ipc_get_platform_key (new_name, FALSE);
#else
	ret->platform_key = __p_ipc_get_platform_key (new_name, TRUE);
#endif
	ret->perms = perms;
	ret->size  = size;

	p_free (new_name);

	if (P_UNLIKELY (__p_shm_create_handle (ret, error) == FALSE)) {
		p_shm_free (ret);
		return NULL;
	}

	if (P_LIKELY (ret->size > size && size != 0))
		ret->size = size;

	return ret;
}

P_LIB_API void
p_shm_take_ownership (PShm *shm)
{
	if (P_UNLIKELY (shm == NULL))
		return;

	shm->shm_created = TRUE;
	p_semaphore_take_ownership (shm->sem);
}

P_LIB_API void
p_shm_free (PShm *shm)
{
	if (P_UNLIKELY (shm == NULL))
		return;

	__p_shm_clean_handle (shm);

	if (P_LIKELY (shm->platform_key != NULL))
		p_free (shm->platform_key);

	p_free (shm);
}

P_LIB_API pboolean
p_shm_lock (PShm	*shm,
	    PError	**error)
{
	if (P_UNLIKELY (shm == NULL)) {
		p_error_set_error_p (error,
				     (pint) P_ERROR_IPC_INVALID_ARGUMENT,
				     0,
				     "Invalid input argument");
		return FALSE;
	}

	return p_semaphore_acquire (shm->sem, error);
}

P_LIB_API pboolean
p_shm_unlock (PShm	*shm,
	      PError	**error)
{
	if (P_UNLIKELY (shm == NULL)) {
		p_error_set_error_p (error,
				     (pint) P_ERROR_IPC_INVALID_ARGUMENT,
				     0,
				     "Invalid input argument");
		return FALSE;
	}

	return p_semaphore_release (shm->sem, error);
}

P_LIB_API ppointer
p_shm_get_address (const PShm *shm)
{
	if (P_UNLIKELY (shm == NULL))
		return NULL;

	return shm->addr;
}

P_LIB_API psize
p_shm_get_size (const PShm *shm)
{
	if (P_UNLIKELY (shm == NULL))
		return 0;

	return shm->size;
}
