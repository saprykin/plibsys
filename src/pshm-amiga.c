/*
 * The MIT License
 *
 * Copyright (C) 2018 Alexander Saprykin <saprykin.spb@gmail.com>
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
#include "pipc-private.h"

#include <string.h>

#include <exec/types.h>
#include <proto/exec.h>

#define P_SHM_NAMESPACE	"pshm"
#define P_SHM_SUFFIX	"_p_shm_object"
#define P_SHM_PRIV_SIZE	(2 * sizeof (psize))

struct PShm_ {
	pboolean	is_owner;
	pchar		*platform_key;
	ppointer	addr;
	psize		size;
	PSemaphore	*sem;
	PShmAccessPerms	perms;
};

static PErrorIPC pp_shm_get_ipc_error (puint32 err_code);
static pboolean pp_shm_create_handle (PShm *shm, PError **error);

static PErrorIPC
pp_shm_get_ipc_error (puint32 err_code)
{
	if (err_code == ANMERROR_NOERROR)
		return P_ERROR_IPC_NONE;
	else if (err_code == ANMERROR_NOMEMORY)
		return P_ERROR_IPC_NO_RESOURCES;
	else if (err_code == ANMERROR_DUPLICATENAME)
		return P_ERROR_IPC_EXISTS;
	else if (err_code == ANMERROR_PARAMETER)
		return P_ERROR_IPC_INVALID_ARGUMENT;
	else
		return P_ERROR_IPC_FAILED;
}

static pboolean
pp_shm_create_handle (PShm	*shm,
		      PError	**error)
{
	ppointer	mem_area;
	puint32		err_code;
	pboolean	is_exists;

	if (P_UNLIKELY (shm == NULL || shm->platform_key == NULL)) {
		p_error_set_error_p (error,
				     (pint) P_ERROR_IPC_INVALID_ARGUMENT,
				     0,
				     "Invalid input argument");
		return FALSE;
	}

	IExec->Forbid ();

	mem_area = IExec->FindNamedMemory (P_SHM_NAMESPACE, shm->platform_key);

	if (mem_area == NULL) {
		mem_area = IExec->AllocNamedMemoryTags (shm->size + P_SHM_PRIV_SIZE,
							P_SHM_NAMESPACE,
							shm->platform_key,
							ANMT_Error, &err_code,
							TAG_END);

		if (P_UNLIKELY (mem_area == NULL)) {
			IExec->Permit ();
			p_error_set_error_p (error,
				     (pint) pp_shm_get_ipc_error (err_code),
				     (pint) err_code,
				     "Failed to call AllocNamedMemoryTags() to create memory segment");
			return FALSE;
		}

		memset (mem_area, 0, shm->size + P_SHM_PRIV_SIZE);

		/* Set size and counter */
		*((psize *) mem_area)     = shm->size;
		*((psize *) mem_area + 1) = 1;

		is_exists = FALSE;
	} else {
		*((psize *) mem_area + 1) += 1;

		shm->size = *((psize *) mem_area);
		is_exists = TRUE;
	}

	shm->addr = ((pchar *) mem_area) + P_SHM_PRIV_SIZE;

	if (P_UNLIKELY ((shm->sem = p_semaphore_new (shm->platform_key, 1,
						     is_exists ? P_SEM_ACCESS_OPEN : P_SEM_ACCESS_CREATE,
						     error)) == NULL)) {
		IExec->FreeNamedMemory (P_SHM_NAMESPACE, shm->platform_key);
		IExec->Permit ();
		return FALSE;
	}

	IExec->Permit ();

	return TRUE;
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
	if (P_UNLIKELY (shm == NULL))
		return;

	shm->is_owner = TRUE;
	p_semaphore_take_ownership (shm->sem);
}

P_LIB_API void
p_shm_free (PShm *shm)
{
	if (P_UNLIKELY (shm == NULL))
		return;

	if (shm->addr != NULL) {
		IExec->Forbid ();

		*((psize *) shm->addr - 1) -= 1;

		if (shm->is_owner || *((psize *) shm->addr - 1) == 0) {
			p_semaphore_free (shm->sem);
			shm->sem = NULL;

			IExec->FreeNamedMemory (P_SHM_NAMESPACE, shm->platform_key);			
		}

		IExec->Permit ();
	}

	shm->is_owner = FALSE;
	shm->addr     = NULL;
	shm->size     = 0;

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
