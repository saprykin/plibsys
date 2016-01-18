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

#include "plib-private.h"
#include "pmem.h"
#include "psemaphore.h"

#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <windows.h>

#define P_SEM_SUFFIX		"_p_sem_object"
#define P_SEM_ERROR_BUF_SIZE	255

typedef HANDLE psem_hdl;
#define P_SEM_INVALID_HDL	NULL

struct _PSemaphore {
	pchar			*platform_key;
	psem_hdl		sem_hdl;
	PSemaphoreError		error;
	pchar			error_str[P_SEM_ERROR_BUF_SIZE];
	pint			init_val;
};

static pboolean __p_semaphore_create_handle (PSemaphore *sem);
static void __p_semaphore_clean_handle (PSemaphore *sem);

static pboolean
__p_semaphore_create_handle (PSemaphore *sem)
{
	if (sem == NULL || sem->platform_key == NULL)
		return FALSE;

	/* Multibyte character set must be enabled in MS VS */
	if ((sem->sem_hdl = CreateSemaphore (NULL, sem->init_val, MAXLONG, sem->platform_key)) == NULL) {
		P_ERROR ("PSemaphore: CreateSemaphore failed");
		return FALSE;
	}

	return TRUE;
}

static void
__p_semaphore_clean_handle (PSemaphore *sem)
{
	if (sem == NULL)
		return;

	if (sem->sem_hdl && !CloseHandle (sem->sem_hdl))
		P_ERROR ("PSemaphore: CloseHandle failed");

	sem->sem_hdl = P_SEM_INVALID_HDL;
	sem->error = P_SEM_ERROR_NONE;
	sem->error_str[0] = '\0';
}


P_LIB_API PSemaphore *
p_semaphore_new (const pchar *name,  pint init_val, PSemaphoreAccessMode mode)
{
	PSemaphore	*ret;
	pchar		*new_name;

	P_UNUSED (mode);

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
	strcpy (new_name, P_SEM_SUFFIX);

	ret->platform_key = __p_ipc_get_platform_key (new_name, FALSE);
	ret->init_val = init_val;

	p_free (new_name);

	if (!__p_semaphore_create_handle (ret)) {
		P_ERROR ("PSemaphore: failed to create system handle");
		p_semaphore_free (ret);
		return NULL;
	}

	return ret;
}

P_LIB_API void
p_semaphore_take_ownership (PSemaphore *sem)
{
	P_UNUSED (sem);
}

P_LIB_API pboolean
p_semaphore_acquire (PSemaphore *sem)
{
	pboolean ret;

	if (sem == NULL)
		return FALSE;

	ret = (WaitForSingleObject (sem->sem_hdl, INFINITE) == WAIT_OBJECT_0) ? TRUE : FALSE;

	if (!ret)
		P_ERROR ("PSemaphore: failed to acquire lock");

	return ret;
}

P_LIB_API pboolean
p_semaphore_release (PSemaphore *sem)
{
	pboolean ret;

	if (sem == NULL)
		return FALSE;

	ret = ReleaseSemaphore (sem->sem_hdl, 1, NULL) ? TRUE : FALSE;

	if (!ret)
		P_ERROR ("PSemaphore: failed to release lock");

	return ret;
}

P_LIB_API void
p_semaphore_free (PSemaphore *sem)
{
	if (sem == NULL)
		return;

	__p_semaphore_clean_handle (sem);

	if (sem->platform_key)
		p_free (sem->platform_key);

	p_free (sem);
}
