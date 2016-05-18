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
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

#include "pmem.h"
#include "pmutex.h"

#include <stdlib.h>
#include <synch.h>
#include <thread.h>

typedef mutex_t mutex_hdl;

struct _PMutex {
	mutex_hdl	hdl;
};

P_LIB_API PMutex *
p_mutex_new (void)
{
	PMutex *ret;

	if ((P_UNLIKELY (ret = p_malloc0 (sizeof (PMutex))) == NULL)) {
		P_ERROR ("PLMutex: failed to allocate memory");
		return NULL;
	}

	mutex_init (&ret->hdl, USYNC_THREAD, NULL);

	return ret;
}

P_LIB_API pboolean
p_mutex_lock (PMutex *mutex)
{
	if (P_UNLIKELY (mutex == NULL))
		return FALSE;

	if (P_LIKELY (mutex_lock (&mutex->hdl) == 0))
		return TRUE;
	else {
		P_ERROR ("PLMutex: failed to lock mutex object");
		return FALSE;
	}
}

P_LIB_API pboolean
p_mutex_trylock (PMutex *mutex)
{
	if (P_UNLIKELY (mutex == NULL))
		return FALSE;

	return (mutex_trylock (&mutex->hdl) == 0) ? TRUE : FALSE;
}

P_LIB_API pboolean
p_mutex_unlock (PMutex *mutex)
{
	if (P_UNLIKELY (mutex == NULL))
		return FALSE;

	if (P_LIKELY (mutex_unlock (&mutex->hdl) == 0))
		return TRUE;
	else {
		P_ERROR ("PLMutex: failed to unlock mutex object");
		return FALSE;
	}
}

P_LIB_API void
p_mutex_free (PMutex *mutex)
{
	if (P_UNLIKELY (mutex == NULL))
		return;

	mutex_destroy (&mutex->hdl);

	p_free (mutex);
}
