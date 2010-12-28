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

#include <stdlib.h>
#include <winsock2.h>
#include <windows.h>

typedef HANDLE mutex_hdl;

struct _PMutex {
	mutex_hdl	hdl;
};

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

