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

#include "pmem.h"
#include "puthread.h"

#include <stdlib.h>

struct _PUThread {
	pint			hdl;
	pboolean		joinable;
	PUThreadPriority	prio;
};

void
__p_uthread_init (void)
{
}

void
__p_uthread_shutdown (void)
{
}

void
__p_uthread_win32_thread_detach (void)
{
}

P_LIB_API PUThread *
p_uthread_create_full (PUThreadFunc	func,
		       ppointer		data,
		       pboolean		joinable,
		       PUThreadPriority	prio)
{
	PUThread		*ret;

	if (!func)
		return NULL;

	if ((ret = p_malloc0 (sizeof (PUThread))) == NULL) {
		P_ERROR ("PUThread: failed to allocate memory");
		return NULL;
	}

	ret->hdl = -1;
	ret->joinable = joinable;
	ret->prio = prio;

	func (data);

	return ret;
}

P_LIB_API PUThread *
p_uthread_create (PUThreadFunc		func,
		  ppointer		data,
		  pboolean		joinable)
{
	/* All checks will be inside */
	return p_uthread_create_full (func, data, joinable, P_UTHREAD_PRIORITY_NORMAL);
}

P_LIB_API void
p_uthread_exit (pint code)
{
	P_INT_TO_POINTER (code);
}

P_LIB_API pint
p_uthread_join (PUThread *thread)
{
	ppointer ret;

	if (!thread || !thread->joinable)
		return -1;

	return thread->hdl;
}

P_LIB_API void
p_uthread_free (PUThread *thread)
{
	if (!thread)
		return;

	p_free (thread);
}

P_LIB_API void
p_uthread_yield (void)
{
}

P_LIB_API pint
p_uthread_set_priority (PUThread		*thread,
			PUThreadPriority	prio)
{
	if (thread == NULL)
		return -1;

	thread->prio = prio;

	return -1;
}

P_LIB_API P_HANDLE
p_uthread_current_id (void)
{
	return (P_HANDLE) 0;
}

P_LIB_API PUThreadKey *
p_uthread_local_new (PDestroyFunc free_func)
{
	P_UNUSED (free_func);
	return NULL;
}

P_LIB_API void
p_uthread_local_free (PUThreadKey *key)
{
	P_UNUSED (key);
}

P_LIB_API ppointer
p_uthread_get_local (PUThreadKey *key)
{
	P_UNUSED (key);
}

P_LIB_API void
p_uthread_set_local (PUThreadKey	*key,
		     ppointer		value)
{
	P_UNUSED (key);
	P_UNUSED (value);
}

P_LIB_API void
p_uthread_replace_local	(PUThreadKey	*key,
			 ppointer	value)
{
	P_UNUSED (key);
	P_UNUSED (value);
}
