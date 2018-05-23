/*
 * The MIT License
 *
 * Copyright (C) 2010-2018 Alexander Saprykin <saprykin.spb@gmail.com>
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
#include "perror-private.h"
#include "pipc-private.h"

#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <fcntl.h>
#include <semaphore.h>
#include <errno.h>

#define P_SEM_SUFFIX		"_p_sem_object"

typedef sem_t psem_hdl;

/* On some HP-UX versions it may not be defined */
#ifndef SEM_FAILED
#  define SEM_FAILED ((sem_t *) -1)
#endif

#ifdef P_OS_SOLARIS
#  define P_SEM_INVALID_HDL	(sem_t *) -1
#else
#  define P_SEM_INVALID_HDL	SEM_FAILED
#endif

struct PSemaphore_ {
	pboolean		sem_created;
	pchar			*platform_key;
#if defined (P_OS_VMS) && (PLIBSYS_SIZEOF_VOID_P == 4)
#  pragma __pointer_size 64
#endif
	psem_hdl		*sem_hdl;
#if defined (P_OS_VMS) && (PLIBSYS_SIZEOF_VOID_P == 4)
#  pragma __pointer_size 32
#endif
	PSemaphoreAccessMode	mode;
	pint			init_val;
};

static pboolean pp_semaphore_create_handle (PSemaphore *sem, PError **error);
static void pp_semaphore_clean_handle (PSemaphore *sem);

static pboolean
pp_semaphore_create_handle (PSemaphore	*sem,
			    PError	**error)
{
	pint init_val;

	if (P_UNLIKELY (sem == NULL || sem->platform_key == NULL)) {
		p_error_set_error_p (error,
				     (pint) P_ERROR_IPC_INVALID_ARGUMENT,
				     0,
				     "Invalid input argument");
		return FALSE;
	}

	init_val = sem->init_val;

	/* Solaris may interrupt sem_open() call */
	while ((sem->sem_hdl = sem_open (sem->platform_key,
					 O_CREAT | O_EXCL,
					 0660,
					 init_val)) == P_SEM_INVALID_HDL &&
		p_error_get_last_system () == EINTR)
	;

	if (sem->sem_hdl == P_SEM_INVALID_HDL) {
		if (p_error_get_last_system () == EEXIST) {
			if (sem->mode == P_SEM_ACCESS_CREATE)
				sem_unlink (sem->platform_key);
			else
				init_val = 0;

			while ((sem->sem_hdl = sem_open (sem->platform_key,
							 0,
							 0,
							 init_val)) == P_SEM_INVALID_HDL &&
				p_error_get_last_system () == EINTR)
			;
		}
	} else
		sem->sem_created = TRUE;

	if (P_UNLIKELY (sem->sem_hdl == P_SEM_INVALID_HDL)) {
		p_error_set_error_p (error,
				     (pint) p_error_get_last_ipc (),
				     p_error_get_last_system (),
				     "Failed to call sem_open() to create semaphore");
		pp_semaphore_clean_handle (sem);
		return FALSE;
	}

	return TRUE;
}

static void
pp_semaphore_clean_handle (PSemaphore *sem)
{
	if (P_UNLIKELY (sem->sem_hdl != P_SEM_INVALID_HDL &&
			sem_close (sem->sem_hdl) == -1))
		P_ERROR ("PSemaphore::pp_semaphore_clean_handle: sem_close() failed");

	if (sem->sem_hdl != P_SEM_INVALID_HDL &&
	    sem->sem_created == TRUE &&
	    sem_unlink (sem->platform_key) == -1)
		P_ERROR ("PSemaphore::pp_semaphore_clean_handle: sem_unlink() failed");

	sem->sem_created = FALSE;
	sem->sem_hdl = P_SEM_INVALID_HDL;
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

#if defined (P_OS_IRIX) || defined (P_OS_TRU64)
	/* IRIX and Tru64 prefer filename styled IPC names */
	ret->platform_key = p_ipc_get_platform_key (new_name, FALSE);
#else
	ret->platform_key = p_ipc_get_platform_key (new_name, TRUE);
#endif
	ret->init_val = init_val;
	ret->mode = mode;

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
	pboolean	ret;
	pint		res;

	if (P_UNLIKELY (sem == NULL)) {
		p_error_set_error_p (error,
				     (pint) P_ERROR_IPC_INVALID_ARGUMENT,
				     0,
				     "Invalid input argument");
		return FALSE;
	}

	while ((res = sem_wait (sem->sem_hdl)) == -1 && p_error_get_last_system () == EINTR)
		;

	ret = (res == 0);

	if (P_UNLIKELY (ret == FALSE))
		p_error_set_error_p (error,
				     (pint) p_error_get_last_ipc (),
				     p_error_get_last_system (),
				     "Failed to call sem_wait() on semaphore");

	return ret;
}

P_LIB_API pboolean
p_semaphore_release (PSemaphore	*sem,
		     PError	**error)
{
	pboolean ret;

	if (P_UNLIKELY (sem == NULL)) {
		p_error_set_error_p (error,
				     (pint) P_ERROR_IPC_INVALID_ARGUMENT,
				     0,
				     "Invalid input argument");
		return FALSE;
	}

	ret = (sem_post (sem->sem_hdl) == 0);

	if (P_UNLIKELY (ret == FALSE))
		p_error_set_error_p (error,
				     (pint) p_error_get_last_ipc (),
				     p_error_get_last_system (),
				     "Failed to call sem_post() on semaphore");

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
