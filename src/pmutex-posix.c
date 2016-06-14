/*
 * Copyright (C) 2010-2016 Alexander Saprykin <xelfium@gmail.com>
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
#include <pthread.h>

typedef pthread_mutex_t mutex_hdl;

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

	if (P_UNLIKELY (pthread_mutex_init (&ret->hdl, NULL) != 0)) {
		P_ERROR ("PMutex::p_mutex_new: pthread_mutex_init() failed");
		p_free (ret);
		return NULL;
	}

	return ret;
}

P_LIB_API pboolean
p_mutex_lock (PMutex *mutex)
{
	if (P_UNLIKELY (mutex == NULL))
		return FALSE;

	if (P_LIKELY (pthread_mutex_lock (&mutex->hdl) == 0))
		return TRUE;
	else {
		P_ERROR ("PMutex::p_mutex_lock: pthread_mutex_lock() failed");
		return FALSE;
	}
}

P_LIB_API pboolean
p_mutex_trylock (PMutex *mutex)
{
	if (P_UNLIKELY (mutex == NULL))
		return FALSE;

	return (pthread_mutex_trylock (&mutex->hdl) == 0) ? TRUE : FALSE;
}

P_LIB_API pboolean
p_mutex_unlock (PMutex *mutex)
{
	if (P_UNLIKELY (mutex == NULL))
		return FALSE;

	if (P_LIKELY (pthread_mutex_unlock (&mutex->hdl) == 0))
		return TRUE;
	else {
		P_ERROR ("PMutex::p_mutex_unlock: pthread_mutex_unlock() failed");
		return FALSE;
	}
}

P_LIB_API void
p_mutex_free (PMutex *mutex)
{
	if (P_UNLIKELY (mutex == NULL))
		return;

	if (P_UNLIKELY (pthread_mutex_destroy (&mutex->hdl) != 0))
		P_ERROR ("PMutex::p_mutex_free: pthread_mutex_destroy() failed");

	p_free (mutex);
}
