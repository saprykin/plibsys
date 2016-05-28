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
#include "puthread.h"
#include "puthread-private.h"

#include <stdlib.h>

struct PUThread_ {
	PUThreadBase	base;
	pint		hdl;
};

void
p_uthread_init_internal (void)
{
}

void
p_uthread_shutdown_internal (void)
{
}

void
p_uthread_win32_thread_detach (void)
{
}

PUThread *
p_uthread_create_internal (PUThreadFunc		func,
			   pboolean		joinable,
			   PUThreadPriority	prio,
			   psize		stack_size)
{
	PUThread	*ret;

	P_UNUSED (stack_size);

	if (P_UNLIKELY ((ret = p_malloc0 (sizeof (PUThread))) == NULL)) {
		P_ERROR ("PUThread: failed to allocate memory");
		return NULL;
	}

	ret->hdl           = -1;
	ret->base.joinable = joinable;
	ret->base.prio     = prio;

	ret->base.func (ret);

	return ret;
}

void
p_uthread_exit_internal (void)
{
}

void
p_uthread_wait_internal (PUThread *thread)
{
	P_UNUSED (thread);
}

void
p_uthread_free_internal (PUThread *thread)
{
	p_free (thread);
}

P_LIB_API void
p_uthread_yield (void)
{
}

P_LIB_API pboolean
p_uthread_set_priority (PUThread		*thread,
			PUThreadPriority	prio)
{
	if (P_UNLIKELY (thread == NULL))
		return FALSE;

	thread->base.prio = prio;

	return FALSE;
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
