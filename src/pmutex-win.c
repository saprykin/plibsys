/* 
 * 19.11.2010
 * Copyright (C) 2010 Alexander Saprykin <xelfium@gmail.com>
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

#include "pmem.h"
#include "pmutex.h"
#include "patomic.h"

#include <stdlib.h>
#include <winsock2.h>
#include <windows.h>

typedef HANDLE mutex_hdl;

struct _PMutex {
	mutex_hdl	hdl;
};

static PMutex *p_once_mutex = NULL;

void
_p_mutex_win_init (void)
{
	if (p_once_mutex == NULL)
		p_once_mutex = p_mutex_new ();
}

void
_p_mutex_win_shutdown (void)
{
	if (p_once_mutex != NULL) {
		p_mutex_free (p_once_mutex);
		p_once_mutex = NULL;
	}
}

#define p_static_mutex_get_mutex_impl(mutex) \
	(p_atomic_pointer_get (mutex) ? *(mutex) : \
	_p_static_mutex_get_mutex (mutex))
	
static PMutex*
_p_static_mutex_get_mutex (PMutex **mutex)
{
	PMutex *result;

	result = p_atomic_pointer_get (mutex);

	if (!result) {
		p_mutex_lock (p_once_mutex);
		result = *mutex;
		
		if (!result) {
			result = p_mutex_new ();
			p_atomic_pointer_set (mutex, result);
        	}
		
		p_mutex_unlock (p_once_mutex);
	}

	return result;
}

P_LIB_API PMutex *
p_mutex_new (void)
{
	PMutex *ret;

	if ((ret = p_malloc0 (sizeof (PMutex))) == NULL) {
		P_ERROR ("PMutex: failed to allocate memory");
		return NULL;
	}

	if ((ret->hdl = CreateMutex (NULL, FALSE, NULL)) == NULL) {
		P_ERROR ("PMutex: failed to initialize mutex object");
		p_free (ret);
		return NULL;
	}

	return ret;
}

P_LIB_API pboolean
p_mutex_lock (PMutex *mutex)
{
	if (!mutex)
		return FALSE;

	if (WaitForSingleObject (mutex->hdl, INFINITE) == WAIT_OBJECT_0)
		return TRUE;
	else {
		P_ERROR ("PMutex: failed to lock mutex object");
		return FALSE;
	}
}

P_LIB_API pboolean
p_mutex_trylock (PMutex *mutex)
{
	if (!mutex)
		return FALSE;

	if (WaitForSingleObject (mutex->hdl, 0) == WAIT_OBJECT_0)
		return TRUE;
	else {
		P_ERROR ("PMutex: failed to try lock mutex object");
		return FALSE;
	}
}

P_LIB_API pboolean
p_mutex_unlock (PMutex *mutex)
{
	if (!mutex)
		return FALSE;

	if (ReleaseMutex (mutex->hdl))
		return TRUE;
	else {
		P_ERROR ("PMutex: failed to unlock mutex object");
		return FALSE;
	}
}

P_LIB_API void
p_mutex_free (PMutex *mutex)
{
	if (!mutex)
		return;

	if (!CloseHandle (mutex->hdl))
		P_ERROR ("PMutex: error while closing handle");

	p_free (mutex);
}
P_LIB_API void
p_static_mutex_init (PStaticMutex *mutex)
{
	static const PStaticMutex init_mutex = P_STATIC_MUTEX_INIT;

	if (mutex == NULL)
		return;

	*mutex = init_mutex;
}

P_LIB_API void
p_static_mutex_free (PStaticMutex* mutex)
{
	PMutex **runtime_mutex;

	if (mutex == NULL)
		return;

	runtime_mutex = ((PMutex **) mutex);

	if (*runtime_mutex != NULL)
		p_mutex_free (*runtime_mutex);

	*runtime_mutex = NULL;
}

