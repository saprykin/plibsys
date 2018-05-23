/*
 * The MIT License
 *
 * Copyright (C) 2010-2017 Alexander Saprykin <saprykin.spb@gmail.com>
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

#ifdef P_OS_SCO
#  include "pmutex.h"
#endif

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <time.h>

#ifdef PLIBSYS_HAS_POSIX_SCHEDULING
#  ifndef P_OS_VMS
#    include <sched.h>
#  endif
#endif

/* Some systems without native pthreads may lack some of the constants,
 * leave them zero as we are not going to use them anyway */

#ifndef PTHREAD_CREATE_JOINABLE
#  define PTHREAD_CREATE_JOINABLE	0
#endif

#ifndef PTHREAD_CREATE_DETACHED
#  define PTHREAD_CREATE_DETACHED	0
#endif

#ifdef PLIBSYS_HAS_POSIX_SCHEDULING
#  ifndef PTHREAD_INHERIT_SCHED
#    define PTHREAD_INHERIT_SCHED	0
#  endif

#  ifndef PTHREAD_EXPLICIT_SCHED
#    define PTHREAD_EXPLICIT_SCHED	0
#  endif

/* Old Linux kernels may lack a definition */
#  if defined (P_OS_LINUX) && !defined (SCHED_IDLE)
#    define SCHED_IDLE 5
#  endif
#endif

typedef pthread_t puthread_hdl;

struct PUThread_ {
	PUThreadBase	base;
	puthread_hdl	hdl;
};

struct PUThreadKey_ {
	pthread_key_t	*key;
	PDestroyFunc	free_func;
};

#ifdef P_OS_SCO
static PMutex *pp_uthread_tls_mutex = NULL;
#endif

#ifdef PLIBSYS_HAS_POSIX_SCHEDULING
static pboolean pp_uthread_get_unix_priority (PUThreadPriority prio, int *sched_policy, int *sched_priority);
#endif

static pthread_key_t * pp_uthread_get_tls_key (PUThreadKey *key);

#ifdef PLIBSYS_HAS_POSIX_SCHEDULING
static pboolean
pp_uthread_get_unix_priority (PUThreadPriority prio, int *sched_policy, int *sched_priority)
{
	pint	lowBound, upperBound;
	pint	prio_min, prio_max;
	pint	native_prio;

#ifdef SCHED_IDLE
	if (prio == P_UTHREAD_PRIORITY_IDLE) {
		*sched_policy = SCHED_IDLE;
		*sched_priority = 0;
		return TRUE;
	}

	lowBound = (pint) P_UTHREAD_PRIORITY_LOWEST;
#else
	lowBound = (pint) P_UTHREAD_PRIORITY_IDLE;
#endif
	upperBound = (pint) P_UTHREAD_PRIORITY_TIMECRITICAL;

	prio_min = sched_get_priority_min (*sched_policy);
	prio_max = sched_get_priority_max (*sched_policy);

	if (P_UNLIKELY (prio_min == -1 || prio_max == -1))
		return FALSE;

	native_prio = ((pint) prio - lowBound) * (prio_max - prio_min) / upperBound + prio_min;

	if (P_UNLIKELY (native_prio > prio_max))
		native_prio = prio_max;

	if (P_UNLIKELY (native_prio < prio_min))
		native_prio = prio_min;

	*sched_priority = native_prio;

	return TRUE;
}
#endif

static pthread_key_t *
pp_uthread_get_tls_key (PUThreadKey *key)
{
	pthread_key_t *thread_key;

	thread_key = (pthread_key_t *) p_atomic_pointer_get ((ppointer) &key->key);

	if (P_LIKELY (thread_key != NULL))
		return thread_key;

#ifdef P_OS_SCO
	p_mutex_lock (pp_uthread_tls_mutex);

	thread_key = key->key;

	if (P_LIKELY (thread_key == NULL)) {
#endif
		if (P_UNLIKELY ((thread_key = p_malloc0 (sizeof (pthread_key_t))) == NULL)) {
			P_ERROR ("PUThread::pp_uthread_get_tls_key: failed to allocate memory");
#ifdef P_OS_SCO
			p_mutex_unlock (pp_uthread_tls_mutex);
#endif
			return NULL;
		}

		if (P_UNLIKELY (pthread_key_create (thread_key, key->free_func) != 0)) {
			P_ERROR ("PUThread::pp_uthread_get_tls_key: pthread_key_create() failed");
			p_free (thread_key);
#ifdef P_OS_SCO
			p_mutex_unlock (pp_uthread_tls_mutex);
#endif
			return NULL;
		}

#ifndef P_OS_SCO
		if (P_UNLIKELY (p_atomic_pointer_compare_and_exchange ((ppointer) &key->key,
								       NULL,
								       (ppointer) thread_key) == FALSE)) {
			if (P_UNLIKELY (pthread_key_delete (*thread_key) != 0)) {
				P_ERROR ("PUThread::pp_uthread_get_tls_key: pthread_key_delete() failed");
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
#ifdef P_OS_SCO
	if (P_LIKELY (pp_uthread_tls_mutex == NULL))
		pp_uthread_tls_mutex = p_mutex_new ();
#endif
}

void
p_uthread_shutdown_internal (void)
{
#ifdef P_OS_SCO
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
	PUThread		*ret;
	pthread_attr_t		attr;
	pint			create_code;
#ifdef PLIBSYS_HAS_POSIX_SCHEDULING
	struct sched_param	sched;
	pint			native_prio;
	pint			sched_policy;
#endif

#if defined (PLIBSYS_HAS_POSIX_STACKSIZE) && defined (_SC_THREAD_STACK_MIN)
	plong			min_stack;
#endif

	if (P_UNLIKELY ((ret = p_malloc0 (sizeof (PUThread))) == NULL)) {
		P_ERROR ("PUThread::p_uthread_create_internal: failed to allocate memory");
		return NULL;
	}

	ret->base.joinable = joinable;

	if (P_UNLIKELY (pthread_attr_init (&attr) != 0)) {
		P_ERROR ("PUThread::p_uthread_create_internal: pthread_attr_init() failed");
		p_free (ret);
		return NULL;
	}

	if (P_UNLIKELY (pthread_attr_setdetachstate (&attr,
						     joinable ? PTHREAD_CREATE_JOINABLE
							      : PTHREAD_CREATE_DETACHED) != 0)) {
		P_ERROR ("PUThread::p_uthread_create_internal: pthread_attr_setdetachstate() failed");
		pthread_attr_destroy (&attr);
		p_free (ret);
		return NULL;
	}

#ifdef PLIBSYS_HAS_POSIX_SCHEDULING
	if (prio == P_UTHREAD_PRIORITY_INHERIT) {
		if (P_UNLIKELY (pthread_attr_setinheritsched (&attr, PTHREAD_INHERIT_SCHED) != 0))
			P_WARNING ("PUThread::p_uthread_create_internal: pthread_attr_setinheritsched() failed");
	} else {
		if (P_LIKELY (pthread_attr_getschedpolicy (&attr, &sched_policy) == 0)) {
			if (P_LIKELY (pp_uthread_get_unix_priority (prio,
								     &sched_policy,
								     &native_prio) == TRUE)) {
				memset (&sched, 0, sizeof (sched));
				sched.sched_priority = native_prio;

				if (P_LIKELY (pthread_attr_setinheritsched (&attr, PTHREAD_EXPLICIT_SCHED) != 0 ||
					      pthread_attr_setschedpolicy (&attr, sched_policy) != 0 ||
					      pthread_attr_setschedparam (&attr, &sched) != 0))
					P_WARNING ("PUThread::p_uthread_create_internal: failed to set priority");
			} else
				P_WARNING ("PUThread::p_uthread_create_internal: pp_uthread_get_unix_priority() failed");
		} else
			P_WARNING ("PUThread::p_uthread_create_internal: pthread_attr_getschedpolicy() failed");
	}
#endif

#ifdef PLIBSYS_HAS_POSIX_STACKSIZE
#  ifdef _SC_THREAD_STACK_MIN
	if (stack_size > 0) {
		min_stack = (plong) sysconf (_SC_THREAD_STACK_MIN);

		if (P_LIKELY (min_stack > 0)) {
			if (P_UNLIKELY (stack_size < (psize) min_stack))
				stack_size = (psize) min_stack;
		} else
			P_WARNING ("PUThread::p_uthread_create_internal: sysconf() with _SC_THREAD_STACK_MIN failed");

		if (P_UNLIKELY (pthread_attr_setstacksize (&attr, stack_size) != 0))
			P_WARNING ("PUThread::p_uthread_create_internal: pthread_attr_setstacksize() failed");
	}
#  endif
#endif

	create_code = pthread_create (&ret->hdl, &attr, func, ret);

#ifdef EPERM
	if (create_code == EPERM) {
#  ifdef PLIBSYS_HAS_POSIX_SCHEDULING
		pthread_attr_setinheritsched (&attr, PTHREAD_INHERIT_SCHED);
#  endif
		create_code = pthread_create (&ret->hdl, &attr, func, ret);
	}
#endif

	if (P_UNLIKELY (create_code != 0)) {
		P_ERROR ("PUThread::p_uthread_create_internal: pthread_create() failed");
		pthread_attr_destroy (&attr);
		p_free (ret);
		return NULL;
	}

	ret->base.prio = prio;
	pthread_attr_destroy (&attr);

	return ret;
}

void
p_uthread_exit_internal (void)
{
	pthread_exit (P_INT_TO_POINTER (0));
}

void
p_uthread_wait_internal (PUThread *thread)
{
	if (P_UNLIKELY (pthread_join (thread->hdl, NULL) != 0))
		P_ERROR ("PUThread::p_uthread_wait_internal: pthread_join() failed");
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
#ifdef PLIBSYS_HAS_POSIX_SCHEDULING
	struct sched_param	sched;
	pint			policy;
	pint			native_prio;
#endif

	if (P_UNLIKELY (thread == NULL))
		return FALSE;

#ifdef PLIBSYS_HAS_POSIX_SCHEDULING
	if (P_UNLIKELY (pthread_getschedparam (thread->hdl, &policy, &sched) != 0)) {
		P_ERROR ("PUThread::p_uthread_set_priority: pthread_getschedparam() failed");
		return FALSE;
	}

	if (P_UNLIKELY (pp_uthread_get_unix_priority (prio, &policy, &native_prio) == FALSE)) {
		P_ERROR ("PUThread::p_uthread_set_priority: pp_uthread_get_unix_priority() failed");
		return FALSE;
	}

	memset (&sched, 0, sizeof (sched));
	sched.sched_priority = native_prio;

	if (P_UNLIKELY (pthread_setschedparam (thread->hdl, policy, &sched) != 0)) {
		P_ERROR ("PUThread::p_uthread_set_priority: pthread_setschedparam() failed");
		return FALSE;
	}
#endif

	thread->base.prio = prio;
	return TRUE;
}

P_LIB_API P_HANDLE
p_uthread_current_id (void)
{
	return (P_HANDLE) ((psize) pthread_self ());
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
	pthread_key_t	*tls_key;
#ifdef P_OS_SCO
	ppointer	value;
#endif

	if (P_UNLIKELY (key == NULL))
		return NULL;

	if (P_UNLIKELY ((tls_key = pp_uthread_get_tls_key (key)) == NULL))
		return NULL;

#ifdef P_OS_SCO
	if (P_UNLIKELY (pthread_getspecific (*tls_key, &value) != 0))
		return NULL;

	return value;
#else
	return pthread_getspecific (*tls_key);
#endif
}

P_LIB_API void
p_uthread_set_local (PUThreadKey	*key,
		     ppointer		value)
{
	pthread_key_t *tls_key;

	if (P_UNLIKELY (key == NULL))
		return;

	tls_key = pp_uthread_get_tls_key (key);

	if (P_LIKELY (tls_key != NULL)) {
		if (P_UNLIKELY (pthread_setspecific (*tls_key, value) != 0))
			P_ERROR ("PUThread::p_uthread_set_local: pthread_setspecific() failed");
	}
}

P_LIB_API void
p_uthread_replace_local	(PUThreadKey	*key,
			 ppointer	value)
{
	pthread_key_t	*tls_key;
	ppointer	old_value;

	if (P_UNLIKELY (key == NULL))
		return;

	tls_key = pp_uthread_get_tls_key (key);

	if (P_UNLIKELY (tls_key == NULL))
		return;

#ifdef P_OS_SCO
	if (P_UNLIKELY (pthread_getspecific (*tls_key, &old_value) != 0))
		return;
#else
	old_value = pthread_getspecific (*tls_key);
#endif

	if (old_value != NULL && key->free_func != NULL)
		key->free_func (old_value);

	if (P_UNLIKELY (pthread_setspecific (*tls_key, value) != 0))
		P_ERROR ("PUThread::p_uthread_replace_local: pthread_setspecific() failed");
}
