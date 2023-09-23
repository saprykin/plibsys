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
#include "patomic.h"
#include "puthread.h"
#include "puthread-private.h"

#ifndef P_OS_UNIXWARE
#  include "pmutex.h"
#endif

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <thread.h>

#ifdef P_OS_UNIXWARE
#  define PLIBSYS_THREAD_MIN_PRIO 0
#  define PLIBSYS_THREAD_MAX_PRIO 126
#else
#  define PLIBSYS_THREAD_MIN_PRIO 0
#  define PLIBSYS_THREAD_MAX_PRIO 127 
#endif

typedef thread_t puthread_hdl;

struct PUThread_ {
	PUThreadBase	base;
	puthread_hdl	hdl;
};

struct PUThreadKey_ {
	thread_key_t	*key;
	PDestroyFunc	free_func;
};

#ifndef P_OS_UNIXWARE
static PMutex *pp_uthread_tls_mutex = NULL;
#endif

static pint pp_uthread_get_unix_priority (PUThreadPriority prio);
static thread_key_t * pp_uthread_get_tls_key (PUThreadKey *key);

static pint
pp_uthread_get_unix_priority (PUThreadPriority prio)
{
	pint lowBound, upperBound;

	lowBound   = (pint) P_UTHREAD_PRIORITY_IDLE;
	upperBound = (pint) P_UTHREAD_PRIORITY_TIMECRITICAL;

	return ((pint) prio - lowBound) *
	       (PLIBSYS_THREAD_MAX_PRIO - PLIBSYS_THREAD_MIN_PRIO) / upperBound +
	       PLIBSYS_THREAD_MIN_PRIO;
}

static thread_key_t *
pp_uthread_get_tls_key (PUThreadKey *key)
{
	thread_key_t *thread_key;

	thread_key = (thread_key_t *) p_atomic_pointer_get ((ppointer) &key->key);

	if (P_LIKELY (thread_key != NULL))
		return thread_key;

#ifndef P_OS_UNIXWARE
	p_mutex_lock (pp_uthread_tls_mutex);

	thread_key = key->key;

	if (P_LIKELY (thread_key == NULL)) {
#endif
		if (P_UNLIKELY ((thread_key = p_malloc0 (sizeof (thread_key_t))) == NULL)) {
			P_ERROR ("PUThread::pp_uthread_get_tls_key: failed to allocate memory");
#ifndef P_OS_UNIXWARE
			p_mutex_unlock (pp_uthread_tls_mutex);
#endif
			return NULL;
		}

		if (P_UNLIKELY (thr_keycreate (thread_key, key->free_func) != 0)) {
			P_ERROR ("PUThread::pp_uthread_get_tls_key: thr_keycreate() failed");
			p_free (thread_key);
#ifndef P_OS_UNIXWARE
			p_mutex_unlock (pp_uthread_tls_mutex);
#endif
			return NULL;
		}
#ifdef P_OS_UNIXWARE
		if (P_UNLIKELY (p_atomic_pointer_compare_and_exchange ((ppointer) &key->key,
								       NULL,
								       (ppointer) thread_key) == FALSE)) {
			if (P_UNLIKELY (thr_keydelete (*thread_key) != 0)) {
				P_ERROR ("PUThread::pp_uthread_get_tls_key: thr_keydelete() failed");
				p_free (thread_key);
				return NULL;
			}

			p_free (thread_key);

			thread_key = key->key;
		}
#else
		key->key = thread_key;
	}

	p_mutex_unlock (pp_uthread_tls_mutex);
#endif

	return thread_key;
}

void
p_uthread_init_internal (void)
{
#ifndef P_OS_UNIXWARE
	if (P_LIKELY (pp_uthread_tls_mutex == NULL))
		pp_uthread_tls_mutex = p_mutex_new ();
#endif
}

void
p_uthread_shutdown_internal (void)
{
#ifndef P_OS_UNIXWARE
	if (P_LIKELY (pp_uthread_tls_mutex != NULL)) {
		p_mutex_free (pp_uthread_tls_mutex);
		pp_uthread_tls_mutex = NULL;
	}
#endif
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
	pint32		flags;
	psize		min_stack;

	if (P_UNLIKELY ((ret = p_malloc0 (sizeof (PUThread))) == NULL)) {
		P_ERROR ("PUThread::p_uthread_create_internal: failed to allocate memory");
		return NULL;
	}

	if (stack_size > 0) {
#ifdef P_OS_UNIXWARE
		min_stack = thr_minstack ();	
#else
		min_stack = thr_min_stack ();
#endif

		if (P_UNLIKELY (stack_size < min_stack))
			stack_size = min_stack;
	}

	flags = THR_SUSPENDED;
	flags |= joinable ? 0 : THR_DETACHED;

	if (P_UNLIKELY (thr_create (NULL, stack_size, func, ret, flags, &ret->hdl) != 0)) {
		P_ERROR ("PUThread::p_uthread_create_internal: thr_create() failed");
		p_free (ret);
		return NULL;
	}

	if (P_UNLIKELY (thr_setprio (ret->hdl, pp_uthread_get_unix_priority (prio)) != 0))
		P_WARNING ("PUThread::p_uthread_create_internal: thr_setprio() failed");

	if (P_UNLIKELY (thr_continue (ret->hdl) != 0)) {
		P_ERROR ("PUThread::p_uthread_create_internal: thr_continue() failed");
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
	thr_exit (P_INT_TO_POINTER (0));
}

void
p_uthread_wait_internal (PUThread *thread)
{
	if (P_UNLIKELY (thr_join (thread->hdl, NULL, NULL) != 0))
		P_ERROR ("PUThread::p_uthread_wait_internal: thr_join() failed");
}

void
p_uthread_set_name_internal (PUThread *thread)
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
	thr_yield ();
}

P_LIB_API pboolean
p_uthread_set_priority (PUThread		*thread,
			PUThreadPriority	prio)
{
	if (P_UNLIKELY (thread == NULL))
		return FALSE;

	if (P_UNLIKELY (thr_setprio (thread->hdl, pp_uthread_get_unix_priority (prio)) != 0)) {
		P_WARNING ("PUThread::p_uthread_set_priority: thr_setprio() failed");
		return FALSE;
	}

	thread->base.prio = prio;

	return TRUE;
}

P_LIB_API P_HANDLE
p_uthread_current_id (void)
{
	return (P_HANDLE) ((psize) thr_self ());
}

P_LIB_API PUThreadKey *
p_uthread_local_new (PDestroyFunc free_func)
{
	PUThreadKey *ret;

	if (P_UNLIKELY ((ret = p_malloc0 (sizeof (PUThreadKey))) == NULL)) {
		P_ERROR ("PUThread::p_uthread_local_new: failed to allocate memory");
		return NULL;
	}

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
	thread_key_t	*tls_key;
	ppointer	ret = NULL;

	if (P_UNLIKELY (key == NULL))
		return ret;

	tls_key = pp_uthread_get_tls_key (key);

	if (P_LIKELY (tls_key != NULL)) {
		if (P_UNLIKELY (thr_getspecific (*tls_key, &ret) != 0))
			P_ERROR ("PUThread::p_uthread_get_local: thr_getspecific() failed");
	}

	return ret;
}

P_LIB_API void
p_uthread_set_local (PUThreadKey	*key,
		     ppointer		value)
{
	thread_key_t *tls_key;

	if (P_UNLIKELY (key == NULL))
		return;

	tls_key = pp_uthread_get_tls_key (key);

	if (P_LIKELY (tls_key != NULL)) {
		if (P_UNLIKELY (thr_setspecific (*tls_key, value) != 0))
			P_ERROR ("PUThread::p_uthread_set_local: thr_setspecific() failed");
	}
}

P_LIB_API void
p_uthread_replace_local	(PUThreadKey	*key,
			 ppointer	value)
{
	thread_key_t	*tls_key;
	ppointer	old_value = NULL;

	if (P_UNLIKELY (key == NULL))
		return;

	tls_key = pp_uthread_get_tls_key (key);

	if (P_UNLIKELY (tls_key == NULL))
		return;

	if (P_UNLIKELY (thr_getspecific (*tls_key, &old_value) != 0)) {
		P_ERROR ("PUThread::p_uthread_replace_local: thr_getspecific() failed");
		return;
	}

	if (old_value != NULL && key->free_func != NULL)
		key->free_func (old_value);

	if (P_UNLIKELY (thr_setspecific (*tls_key, value) != 0))
		P_ERROR ("PUThread::p_uthread_replace_local: thr_setspecific() failed");
}
