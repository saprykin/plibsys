/*
 * The MIT License
 *
 * Copyright (C) 2010-2019 Alexander Saprykin <saprykin.spb@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * 'Software'), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED 'AS IS', WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include "pmem.h"
#include "puthread.h"
#include "puthread-private.h"

#include <stdlib.h>

struct PUThread_ {
	pint	hdl;
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
	P_UNUSED (func);
	P_UNUSED (joinable);
	P_UNUSED (prio);
	P_UNUSED (stack_size);

	return NULL;
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
p_uthread_set_name_internal (PUThread *thread)
{
	P_UNUSED (thread);
}

void
p_uthread_free_internal (PUThread *thread)
{
	P_UNUSED (thread);
}

P_LIB_API void
p_uthread_yield (void)
{
}

P_LIB_API pboolean
p_uthread_set_priority (PUThread		*thread,
			PUThreadPriority	prio)
{
	P_UNUSED (thread);
	P_UNUSED (prio);

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
