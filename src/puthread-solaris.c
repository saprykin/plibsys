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

/* Threads for Sun Solaris */

#include "pmem.h"
#include "puthread.h"

#include <thread.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

typedef thread_t puthread_hdl;

struct _PUThread {
	puthread_hdl		hdl;
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

P_LIB_API PUThread *
p_uthread_create_full (PUThreadFunc	func,
		       ppointer		data,
		       pboolean		joinable,
		       PUThreadPriority prio)
{
	PUThread	*ret;
	pint32		flags;

	if (!func)
		return NULL;

	if ((ret = p_malloc0 (sizeof (PUThread))) == NULL) {
		P_ERROR ("PUThread: failed to allocate memory");
		return NULL;
	}

	/* On some old Solaris systems (e.g. 2.5.1) non-bound threads are not
	 * timesliced by kernel, so we need to ensure that thread will be
	 * scheduled by using THR_BOUND flag */
	flags = THR_BOUND;
	flags |= joinable ? 0 : THR_DETACHED;

	if (thr_create (NULL, 0, func, data, flags, &ret->hdl) != 0) {
		P_ERROR ("PUThread: failed to create Solaris thread");
		p_uthread_free (ret);
		return NULL;
	}

	ret->joinable	= joinable;
	ret->prio	= prio;

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
	thr_exit (P_INT_TO_POINTER (code));

	/* To smile a compiler */
#ifndef P_CC_SUN
	while (1);
#endif
}

P_LIB_API pint
p_uthread_join (PUThread *thread)
{
	ppointer ret;

	if (!thread || !thread->joinable)
		return -1;

	if (thr_join (thread->hdl, NULL, &ret) != 0) {
		P_ERROR ("PUThread: failed to join Solaris thread");
		p_uthread_free (ret);
		return -1;
	}

	return P_POINTER_TO_INT (ret);
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
	thr_yield ();
}

P_LIB_API pint
p_uthread_set_priority (PUThread		*thread,
			PUThreadPriority	prio)
{
	P_WARNING ("PUThread: priorities for bound threads are not implemented in Solaris");

	if (thread == NULL)
		return -1;

	thread->prio = prio;

	return 0;
}

P_LIB_API P_HANDLE
p_uthread_current_id (void)
{
	return (P_HANDLE) thr_self ();
}
