/*
 * Copyright (C) 2017-2018 Alexander Saprykin <xelfium@gmail.com>
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

struct PSemaphore_ {
	struct SignalSemaphore	*sem_shared;
	pchar			*platform_key;
	pboolean		sem_created;
	PSemaphoreAccessMode	mode;
};

static pboolean pp_semaphore_create_handle (PSemaphore *sem, PError **error);
static void pp_semaphore_clean_handle (PSemaphore *sem);

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
		sem->sem_created = FALSE;
		sem->sem_shared  = sem_sys;
	} else {
		if (sem_sys != NULL && sem->mode == P_SEM_ACCESS_CREATE) {
			IExec->RemSemaphore ((struct SignalSemaphore *) sem_sys);
			IExec->ObtainSemaphore ((struct SignalSemaphore *) sem_sys);
			IExec->ReleaseSemaphore ((struct SignalSemaphore *) sem_sys);

			IExec->FreeVec (sem_sys->ss_Link.ln_Name);
			IExec->FreeVec (sem_sys);
		}

		sem_sys = (struct SignalSemaphore *) IExec->AllocVecTags (sizeof (struct SignalSemaphore),
									  AVT_Type, MEMF_SHARED,
									  AVT_ClearWithValue, 0,
									  TAG_END);

		if (P_UNLIKELY (sem_sys == NULL)) {
			IExec->Permit ();
			p_error_set_error_p (error,
					     (pint) P_ERROR_IPC_NO_RESOURCES,
					     0,
					     "Failed to call AllocMem() to create semaphore");
			pp_semaphore_clean_handle (sem);
			return FALSE;
		}

		sem_sys->ss_Link.ln_Pri  = 0;

		name_len = strlen (sem->platform_key);
		name     = (pchar *) IExec->AllocVecTags (name_len + 1,
							  AVT_ClearWithValue, 0,
							  TAG_END);

		if (P_UNLIKELY (name == NULL)) {
			IExec->Permit ();
			p_error_set_error_p (error,
					     (pint) P_ERROR_IPC_NO_RESOURCES,
					     0,
					     "Failed to call AllocMem() to create semaphore name");
			pp_semaphore_clean_handle (sem);
			return FALSE;
		}

		memcpy (name, sem->platform_key, name_len);

		sem_sys->ss_Link.ln_Name = name;

		sem->sem_shared  = sem_sys;
		sem->sem_created = sem->mode == P_SEM_ACCESS_CREATE ? TRUE : FALSE;

		/* Add to system list */
		IExec->AddSemaphore (sem_sys);
	}

	IExec->Permit ();

	return TRUE;
}

static void
pp_semaphore_clean_handle (PSemaphore *sem)
{
	if (sem->sem_shared == NULL)
		return;

	if (sem->sem_created == TRUE) {
		if (sem->sem_shared->ss_Link.ln_Name != NULL)
			IExec->FreeVec (sem->sem_shared->ss_Link.ln_Name);

		IExec->FreeVec (sem->sem_shared);
	}

	sem->sem_created = FALSE;
	sem->sem_shared  = NULL;
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

	sem->sem_created = TRUE;
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

	if (sem->sem_created == TRUE && sem->sem_shared != NULL) {
		IExec->RemSemaphore ((struct SignalSemaphore *) sem->sem_shared);
		IExec->ObtainSemaphore ((struct SignalSemaphore *) sem->sem_shared);
		IExec->ReleaseSemaphore ((struct SignalSemaphore *) sem->sem_shared);
	}

	pp_semaphore_clean_handle (sem);

	if (P_LIKELY (sem->platform_key != NULL))
		p_free (sem->platform_key);

	p_free (sem);
}
