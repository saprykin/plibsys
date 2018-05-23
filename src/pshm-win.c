/*
 * The MIT License
 *
 * Copyright (C) 2010-2016 Alexander Saprykin <saprykin.spb@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * 'Software'), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED 'AS IS', WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include "perror.h"
#include "pmem.h"
#include "psemaphore.h"
#include "pshm.h"
#include "perror-private.h"
#include "pipc-private.h"

#include <stdlib.h>
#include <string.h>

#define P_SHM_INVALID_HDL	NULL
#define P_SHM_SUFFIX		"_p_shm_object"

typedef HANDLE pshm_hdl;

struct PShm_ {
	pchar		*platform_key;
	pshm_hdl	shm_hdl;
	ppointer	addr;
	psize		size;
	PSemaphore	*sem;
	PShmAccessPerms	perms;
};

static pboolean pp_shm_create_handle (PShm *shm, PError **error);
static void pp_shm_clean_handle (PShm *shm);

static pboolean
pp_shm_create_handle (PShm	*shm,
		      PError	**error)
{
	pboolean			is_exists;
	MEMORY_BASIC_INFORMATION	mem_stat;
	DWORD				protect;

	if (P_UNLIKELY (shm == NULL || shm->platform_key == NULL)) {
		p_error_set_error_p (error,
				     (pint) P_ERROR_IPC_INVALID_ARGUMENT,
				     0,
				     "Invalid input argument");
		return FALSE;
	}

	is_exists = FALSE;

	protect = (shm->perms == P_SHM_ACCESS_READONLY) ? PAGE_READONLY : PAGE_READWRITE;

	/* Multibyte character set must be enabled */
	if (P_UNLIKELY ((shm->shm_hdl = CreateFileMappingA (INVALID_HANDLE_VALUE,
							    NULL,
							    protect,
							    0,
							    (DWORD) shm->size,
							    shm->platform_key)) == NULL)) {
		p_error_set_error_p (error,
				     (pint) p_error_get_last_ipc (),
				     p_error_get_last_system (),
				     "Failed to call CreateFileMapping() to create file mapping");
		pp_shm_clean_handle (shm);
		return FALSE;
	}

	protect = (protect == PAGE_READONLY) ? FILE_MAP_READ : FILE_MAP_ALL_ACCESS;

	if (P_UNLIKELY ((shm->addr = MapViewOfFile (shm->shm_hdl, protect, 0, 0, 0)) == NULL)) {
		p_error_set_error_p (error,
				     (pint) p_error_get_last_ipc (),
				     p_error_get_last_system (),
				     "Failed to call MapViewOfFile() to map file to memory");
		pp_shm_clean_handle (shm);
		return FALSE;
	}

	if (p_error_get_last_system () == ERROR_ALREADY_EXISTS)
		is_exists = TRUE;

	if (P_UNLIKELY (VirtualQuery (shm->addr, &mem_stat, sizeof (mem_stat)) == 0)) {
		p_error_set_error_p (error,
				     (pint) p_error_get_last_ipc (),
				     p_error_get_last_system (),
				     "Failed to call VirtualQuery() to get memory map info");
		pp_shm_clean_handle (shm);
		return FALSE;
	}

	shm->size = mem_stat.RegionSize;

	if (P_UNLIKELY ((shm->sem = p_semaphore_new (shm->platform_key, 1,
						     is_exists ? P_SEM_ACCESS_OPEN : P_SEM_ACCESS_CREATE,
						     error)) == NULL)) {
		pp_shm_clean_handle (shm);
		return FALSE;
	}

	return TRUE;
}

static void
pp_shm_clean_handle (PShm *shm)
{
	if (P_UNLIKELY (shm->addr != NULL && UnmapViewOfFile ((char *) shm->addr) == 0))
		P_ERROR ("PShm::pp_shm_clean_handle: UnmapViewOfFile() failed");

	if (P_UNLIKELY (shm->shm_hdl != P_SHM_INVALID_HDL && CloseHandle (shm->shm_hdl) == 0))
		P_ERROR ("PShm::pp_shm_clean_handle: CloseHandle() failed");

	if (P_LIKELY (shm->sem != NULL)) {
		p_semaphore_free (shm->sem);
		shm->sem = NULL;
	}

	shm->shm_hdl = P_SHM_INVALID_HDL;
	shm->addr    = NULL;
	shm->size    = 0;
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

	ret->platform_key = p_ipc_get_platform_key (new_name, FALSE);
	ret->perms        = perms;
	ret->size         = size;

	p_free (new_name);

	if (P_UNLIKELY (pp_shm_create_handle (ret, error) == FALSE)) {
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
	P_UNUSED (shm);
}

P_LIB_API void
p_shm_free (PShm *shm)
{
	if (P_UNLIKELY (shm == NULL))
		return;

	pp_shm_clean_handle (shm);

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
