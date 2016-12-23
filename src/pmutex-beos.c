/*
 * Copyright (C) 2016 Alexander Saprykin <xelfium@gmail.com>
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

#include <kernel/OS.h>

typedef sem_id mutex_hdl;

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

	if (P_UNLIKELY ((ret->hdl = create_sem (1, "")) < B_OK)) {
		P_ERROR ("PMutex::p_mutex_new: create_sem() failed");
		p_free (ret);
		return NULL;
	}

	return ret;
}

P_LIB_API pboolean
p_mutex_lock (PMutex *mutex)
{
	status_t ret_status;

	if (P_UNLIKELY (mutex == NULL))
		return FALSE;

	while ((ret_status = acquire_sem (mutex->hdl)) == B_INTERRUPTED)
		;

	if (P_LIKELY (ret_status == B_NO_ERROR))
		return TRUE;
	else {
		P_ERROR ("PMutex::p_mutex_lock: acquire_sem() failed");
		return FALSE;
	}
}

P_LIB_API pboolean
p_mutex_trylock (PMutex *mutex)
{
	if (P_UNLIKELY (mutex == NULL))
		return FALSE;

	return (acquire_sem_etc (mutex->hdl, 1, B_RELATIVE_TIMEOUT, 0)) == B_NO_ERROR ? TRUE : FALSE;
}

P_LIB_API pboolean
p_mutex_unlock (PMutex *mutex)
{
	if (P_UNLIKELY (mutex == NULL))
		return FALSE;

	if (P_LIKELY (release_sem (mutex->hdl) == B_NO_ERROR))
		return TRUE;
	else {
		P_ERROR ("PMutex::p_mutex_unlock: release_sem() failed");
		return FALSE;
	}
}

P_LIB_API void
p_mutex_free (PMutex *mutex)
{
	if (P_UNLIKELY (mutex == NULL))
		return;

	if (P_UNLIKELY (delete_sem (mutex->hdl) != B_NO_ERROR))
		P_ERROR ("PMutex::p_mutex_free: delete_sem() failed");

	p_free (mutex);
}
