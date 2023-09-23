/*
 * The MIT License
 *
 * Copyright (C) 2016-2019 Alexander Saprykin <saprykin.spb@gmail.com>
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
#include "patomic.h"
#include "puthread.h"
#include "puthread-private.h"

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <atheos/threads.h>
#include <atheos/tld.h>

typedef thread_id puthread_hdl;

struct PUThread_ {
	PUThreadBase	base;
	puthread_hdl	hdl;
};

struct PUThreadKey_ {
	pint		key;
	PDestroyFunc	free_func;
};

static pint pp_uthread_get_atheos_priority (PUThreadPriority prio);
static pint pp_uthread_get_tls_key (PUThreadKey *key);

static pint
pp_uthread_get_atheos_priority (PUThreadPriority prio)
{
	switch (prio) {
		case P_UTHREAD_PRIORITY_INHERIT:
		{
			thread_info thr_info;
			
			memset (&thr_info, 0, sizeof (thr_info));

			if (P_UNLIKELY (get_thread_info (get_thread_id (NULL), &thr_info) != 0)) {
				P_WARNING ("PUThread::pp_uthread_get_atheos_priority: failed to get thread info");
				return NORMAL_PRIORITY;
			} else
				return thr_info.ti_priority;
		}

		case P_UTHREAD_PRIORITY_IDLE:
			return IDLE_PRIORITY;
		case P_UTHREAD_PRIORITY_LOWEST:
			return LOW_PRIORITY / 2;
		case P_UTHREAD_PRIORITY_LOW:
			return LOW_PRIORITY;
		case P_UTHREAD_PRIORITY_NORMAL:
			return NORMAL_PRIORITY;
		case P_UTHREAD_PRIORITY_HIGH:
			return DISPLAY_PRIORITY;
		case P_UTHREAD_PRIORITY_HIGHEST:
			return URGENT_DISPLAY_PRIORITY;
		case P_UTHREAD_PRIORITY_TIMECRITICAL:
			return REALTIME_PRIORITY;
	}
}

static pint
pp_uthread_get_tls_key (PUThreadKey *key)
{
	pint thread_key;

	thread_key = p_atomic_int_get ((const volatile pint *) &key->key);

	if (P_LIKELY (thread_key >= 0))
		return thread_key;

	if (P_UNLIKELY ((thread_key = alloc_tld (key->free_func)) < 0)) {
		P_ERROR ("PUThread::pp_uthread_get_tls_key: alloc_tld() failed");
		return -1;
	}

	if (P_UNLIKELY (p_atomic_int_compare_and_exchange ((volatile pint *) &key->key,
							   -1,
							   thread_key) == FALSE)) {
		if (P_UNLIKELY (free_tld (thread_key) != 0)) {
			P_ERROR ("PUThread::pp_uthread_get_tls_key: free_tld() failed");
			return -1;
		}

		thread_key = key->key;
	}

	return thread_key;
}

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
	PUThread *ret;

	if (P_UNLIKELY ((ret = p_malloc0 (sizeof (PUThread))) == NULL)) {
		P_ERROR ("PUThread::p_uthread_create_internal: failed to allocate memory");
		return NULL;
	}

	if (P_UNLIKELY ((ret->hdl = spawn_thread ("",
						  func,
						  pp_uthread_get_atheos_priority (prio),
						  stack_size,
						  ret)) < 0)) {
		P_ERROR ("PUThread::p_uthread_create_internal: spawn_thread() failed");
		p_free (ret);
		return NULL;
	}

	if (P_UNLIKELY (resume_thread (ret->hdl) != 0)) {
		P_ERROR ("PUThread::p_uthread_create_internal: resume_thread() failed");
		p_free (ret);
		return NULL;
	}

	ret->base.joinable = joinable;
	ret->base.prio     = prio;

	return ret;
}

void
p_uthread_exit_internal (void)
{
	exit_thread (0);
}

void
p_uthread_wait_internal (PUThread *thread)
{
	wait_for_thread (thread->hdl);
}

void
p_uthread_set_name_internal (PUThread *thread)
{
	pchar    *thr_name   = NULL;
	psize    namelen     = 0;
	pint     res         = 0;
	pboolean is_alloc    = FALSE;

	thr_name = thread->base.name;
	namelen  = strlen (thr_name);

	if (namelen > OS_NAME_LENGTH - 1) {
		if (P_UNLIKELY ((thr_name = p_malloc0 (namelen + 1)) == NULL)) {
			P_ERROR ("PUThread::p_uthread_set_name_internal: failed to allocate memory");
			return;
		}

		memcpy (thr_name, thread->base.name, OS_NAME_LENGTH - 1);

		is_alloc = TRUE;
	}

	if (rename_thread (thread->hdl, thr_name) != 0)
		P_WARNING ("PUThread::p_uthread_set_name_internal: failed to set thread system name");

	if (is_alloc == TRUE)
		p_free (thr_name);
}

void
p_uthread_free_internal (PUThread *thread)
{
	p_free (thread);
}

P_LIB_API void
p_uthread_yield (void)
{
	sched_yield ();
}

P_LIB_API pboolean
p_uthread_set_priority (PUThread		*thread,
			PUThreadPriority	prio)
{
	if (P_UNLIKELY (thread == NULL))
		return FALSE;

	set_thread_priority (thread->hdl, pp_uthread_get_atheos_priority (prio));

	thread->base.prio = prio;

	return TRUE;
}

P_LIB_API P_HANDLE
p_uthread_current_id (void)
{
	return (P_HANDLE) ((psize) get_thread_id (NULL));
}

P_LIB_API PUThreadKey *
p_uthread_local_new (PDestroyFunc free_func)
{
	PUThreadKey *ret;

	if (P_UNLIKELY ((ret = p_malloc0 (sizeof (PUThreadKey))) == NULL)) {
		P_ERROR ("PUThread::p_uthread_local_new: failed to allocate memory");
		return NULL;
	}

	ret->key       = -1;
	ret->free_func = free_func;

	return ret;
}

P_LIB_API void
p_uthread_local_free (PUThreadKey *key)
{
	if (P_UNLIKELY (key == NULL))
		return;

	p_free (key);
}

P_LIB_API ppointer
p_uthread_get_local (PUThreadKey *key)
{
	pint tls_key;

	if (P_UNLIKELY (key == NULL))
		return NULL;

	tls_key = pp_uthread_get_tls_key (key);

	if (P_LIKELY (tls_key >= 0))
		return get_tld (tls_key);

	return NULL;
}

P_LIB_API void
p_uthread_set_local (PUThreadKey	*key,
		     ppointer		value)
{
	pint tls_key;

	if (P_UNLIKELY (key == NULL))
		return;

	tls_key = pp_uthread_get_tls_key (key); 

	if (tls_key >= 0)
		set_tld (tls_key, value);
}

P_LIB_API void
p_uthread_replace_local	(PUThreadKey	*key,
			 ppointer	value)
{
	pint		tls_key;
	ppointer	old_value;

	if (P_UNLIKELY (key == NULL))
		return;

	tls_key = pp_uthread_get_tls_key (key);

	if (P_UNLIKELY (tls_key < 0))
		return;

	old_value = get_tld (tls_key);

	if (old_value != NULL && key->free_func != NULL)
		key->free_func (old_value);

	set_tld (tls_key, value);
}
