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

/* TODO: error report system */

#include "plib-private.h"
#include "pmem.h"
#include "pshm.h"
#include "psemaphore.h"

#include <stdlib.h>
#include <string.h>

#include <windows.h>

typedef HANDLE pshm_hdl;
#define P_SHM_INVALID_HDL	NULL

#define P_SHM_SUFFIX		"_p_shm_object"

struct _PShm {
	pchar		*platform_key;
	pshm_hdl	shm_hdl;
	ppointer	addr;
	psize		size;
	PSemaphore	*sem;
	PShmAccessPerms	perms;
};

static pboolean __p_shm_create_handle (PShm *shm);
static void __p_shm_clean_handle (PShm *shm);

static pboolean
__p_shm_create_handle (PShm *shm)
{
	pboolean			is_exists;
	MEMORY_BASIC_INFORMATION	mem_stat;
	DWORD				protect;

	if (shm == NULL || shm->platform_key == NULL)
		return FALSE;

	is_exists = FALSE;

	protect = (shm->perms == P_SHM_ACCESS_READONLY) ? PAGE_READONLY : PAGE_READWRITE;

	/* Multibyte character set must be enabled in MS VS */
	if ((shm->shm_hdl = CreateFileMapping (INVALID_HANDLE_VALUE, NULL, protect, 0, (DWORD) shm->size, shm->platform_key)) == NULL) {
		P_ERROR ("PShm: CreateFleMapping failed");
		__p_shm_clean_handle (shm);
		return FALSE;
	}

	protect = (protect == PAGE_READONLY) ? FILE_MAP_READ : FILE_MAP_ALL_ACCESS;

	if ((shm->addr = MapViewOfFile (shm->shm_hdl, protect, 0, 0, 0)) == NULL) {
		P_ERROR ("PShm: MapViewOfFile failed");
		__p_shm_clean_handle (shm);
		return FALSE;
	}

	if (GetLastError () == ERROR_ALREADY_EXISTS)
		is_exists = TRUE;

	if (!VirtualQuery (shm->addr, &mem_stat, sizeof (mem_stat))) {
		P_ERROR ("PShm: VirtualQuery failed");
		__p_shm_clean_handle (shm);
		return FALSE;
	}

	shm->size = mem_stat.RegionSize;

	if ((shm->sem = p_semaphore_new (shm->platform_key, 1,
					 is_exists ? P_SEM_ACCESS_OPEN : P_SEM_ACCESS_CREATE,
					 NULL)) == NULL) {
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

	if (shm->addr && !UnmapViewOfFile ((char *) shm->addr))
		P_ERROR ("PShm: UnmapViewOfFile failed");

	if (shm->shm_hdl && !CloseHandle (shm->shm_hdl))
		P_ERROR ("PShm: CloseHandle failed");

	if (shm->sem) {
		p_semaphore_free (shm->sem);
		shm->sem = NULL;
	}

	shm->shm_hdl = P_SHM_INVALID_HDL;
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

	ret->platform_key = __p_ipc_get_platform_key (new_name, FALSE);
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
	P_UNUSED (shm);
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

	if (!p_semaphore_acquire (shm->sem, NULL)) {
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

	if (!p_semaphore_release (shm->sem, NULL)) {
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
