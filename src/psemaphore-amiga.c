/*
 * The MIT License
 *
 * Copyright (C) 2017-2018 Alexander Saprykin <saprykin.spb@gmail.com>
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
#include "pipc-private.h"

#include <stdlib.h>
#include <string.h>

#include <exec/types.h>
#include <exec/semaphores.h>
#include <proto/exec.h>

#define P_SEM_SUFFIX	"_p_sem_object"
#define P_SEM_PRIV_SIZE	(sizeof (psize))

struct PSemaphore_ {
	struct SignalSemaphore	*sem_shared;
	pchar			*platform_key;
	pboolean		is_owner;
	PSemaphoreAccessMode	mode;
};

static pboolean pp_semaphore_create_handle (PSemaphore *sem, PError **error);

static pboolean
pp_semaphore_create_handle (PSemaphore	*sem,
			    PError	**error)
{
	struct SignalSemaphore	*sem_sys;
	psize			name_len;
	pchar			*name;

	if (P_UNLIKELY (sem == NULL || sem->platform_key == NULL)) {
		p_error_set_error_p (error,
				     (pint) P_ERROR_IPC_INVALID_ARGUMENT,
				     0,
				     "Invalid input argument");
		return FALSE;
	}

	IExec->Forbid ();

	sem_sys = (struct SignalSemaphore *) IExec->FindSemaphore (sem->platform_key);

	if (sem_sys != NULL && sem->mode != P_SEM_ACCESS_CREATE) {
		sem->sem_shared = sem_sys;
	} else {
		if (sem_sys != NULL && sem->mode == P_SEM_ACCESS_CREATE) {
			IExec->RemSemaphore (sem_sys);
			IExec->ObtainSemaphore (sem_sys);
			IExec->ReleaseSemaphore (sem_sys);

			IExec->FreeVec (sem_sys->ss_Link.ln_Name);
			IExec->FreeVec (((psize *) sem_sys) - 1);
		}

		sem_sys = (struct SignalSemaphore *) IExec->AllocVecTags (sizeof (struct SignalSemaphore) + P_SEM_PRIV_SIZE,
									  AVT_Type, MEMF_SHARED,
									  AVT_ClearWithValue, 0,
									  TAG_END);

		if (P_UNLIKELY (sem_sys == NULL)) {
			IExec->Permit ();
			p_error_set_error_p (error,
					     (pint) P_ERROR_IPC_NO_RESOURCES,
					     0,
					     "Failed to call AllocMem() to create semaphore");
			return FALSE;
		}

		name_len = strlen (sem->platform_key);
		name     = (pchar *) IExec->AllocVecTags (name_len + 1,
							  AVT_ClearWithValue, 0,
							  TAG_END);

		if (P_UNLIKELY (name == NULL)) {
			IExec->FreeVec (sem_sys);
			IExec->Permit ();
			p_error_set_error_p (error,
					     (pint) P_ERROR_IPC_NO_RESOURCES,
					     0,
					     "Failed to call AllocMem() to create semaphore name");
			return FALSE;
		}

		memcpy (name, sem->platform_key, name_len);

		/* Leave space in memory for counter */
		sem_sys = (struct SignalSemaphore *) (((psize *) sem_sys) + 1);

		sem_sys->ss_Link.ln_Name = name;
		sem->sem_shared = sem_sys;

		/* Add to system list */
		IExec->AddSemaphore (sem_sys);
	}

	*(((psize *) sem_sys) - 1) += 1;

	IExec->Permit ();

	return TRUE;
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

	sem->is_owner = TRUE;
}

P_LIB_API pboolean
p_semaphore_acquire (PSemaphore	*sem,
		     PError	**error)
{
	if (P_UNLIKELY (sem == NULL)) {
		p_error_set_error_p (error,
				     (pint) P_ERROR_IPC_INVALID_ARGUMENT,
				     0,
				     "Invalid input argument");
		return FALSE;
	}

	IExec->ObtainSemaphore (sem->sem_shared);

	return TRUE;
}

P_LIB_API pboolean
p_semaphore_release (PSemaphore	*sem,
		     PError	**error)
{
	if (P_UNLIKELY (sem == NULL)) {
		p_error_set_error_p (error,
				     (pint) P_ERROR_IPC_INVALID_ARGUMENT,
				     0,
				     "Invalid input argument");
		return FALSE;
	}

	IExec->ReleaseSemaphore (sem->sem_shared);

	return TRUE;
}

P_LIB_API void
p_semaphore_free (PSemaphore *sem)
{
	if (P_UNLIKELY (sem == NULL))
		return;

	if (P_UNLIKELY (sem->sem_shared != NULL)) {
		IExec->Forbid ();

		*(((psize *) sem->sem_shared) - 1) -= 1;

		if (*(((psize *) sem->sem_shared) - 1) == 0 || sem->is_owner == TRUE) {
			IExec->RemSemaphore (sem->sem_shared);
			IExec->ObtainSemaphore (sem->sem_shared);
			IExec->ReleaseSemaphore (sem->sem_shared);

			IExec->FreeVec (sem->sem_shared->ss_Link.ln_Name);
			IExec->FreeVec (((psize *) sem->sem_shared) - 1);
		}

		IExec->Permit ();
	}

	if (P_LIKELY (sem->platform_key != NULL))
		p_free (sem->platform_key);

	p_free (sem);
}
