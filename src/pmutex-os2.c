/*
 * Copyright (C) 2017 Alexander Saprykin <xelfium@gmail.com>
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

#include "pmem.h"
#include "pmutex.h"

#include <stdlib.h>

#define INCL_DOSSEMAPHORES
#define INCL_DOSERRORS
#include <os2.h>

typedef HMTX mutex_hdl;

struct PMutex_ {
	mutex_hdl	hdl;
};

P_LIB_API PMutex *
p_mutex_new (void)
{
	PMutex *ret;

	if (P_UNLIKELY ((ret = p_malloc0 (sizeof (PMutex))) == NULL)) {
		P_ERROR ("PMutex::p_mutex_new: failed to allocate memory");
		return NULL;
	}

	if (P_UNLIKELY (DosCreateMutexSem (NULL, (PHMTX) &ret->hdl, 0, FALSE) != NO_ERROR)) {
		P_ERROR ("PMutex::p_mutex_new: DosCreateMutexSem() failed");
		p_free (ret);
		return NULL;
	}

	return ret;
}

P_LIB_API pboolean
p_mutex_lock (PMutex *mutex)
{
	APIRET ulrc;

	if (P_UNLIKELY (mutex == NULL))
		return FALSE;

	while ((ulrc = DosRequestMutexSem (mutex->hdl, SEM_INDEFINITE_WAIT)) == ERROR_INTERRUPT)
		;

	if (P_LIKELY (ulrc == NO_ERROR))
		return TRUE;
	else {
		P_ERROR ("PMutex::p_mutex_lock: DosRequestMutexSem() failed");
		return FALSE;
	}
}

P_LIB_API pboolean
p_mutex_trylock (PMutex *mutex)
{
	if (P_UNLIKELY (mutex == NULL))
		return FALSE;

	return (DosRequestMutexSem (mutex->hdl, SEM_IMMEDIATE_RETURN)) == NO_ERROR ? TRUE : FALSE;
}

P_LIB_API pboolean
p_mutex_unlock (PMutex *mutex)
{
	if (P_UNLIKELY (mutex == NULL))
		return FALSE;

	if (P_LIKELY (DosReleaseMutexSem (mutex->hdl) == NO_ERROR))
		return TRUE;
	else {
		P_ERROR ("PMutex::p_mutex_unlock: DosReleaseMutexSem() failed");
		return FALSE;
	}
}

P_LIB_API void
p_mutex_free (PMutex *mutex)
{
	if (P_UNLIKELY (mutex == NULL))
		return;

	if (P_UNLIKELY (DosCloseMutexSem (mutex->hdl) != NO_ERROR))
		P_ERROR ("PMutex::p_mutex_free: DosCloseMutexSem() failed");

	p_free (mutex);
}
