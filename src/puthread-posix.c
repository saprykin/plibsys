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
#include "patomic.h"
#include "puthread.h"

#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>

/* Some systems without native pthreads may lack some of the constants,
 * leave them zero as we are not going to use them anyway */

#ifndef PTHREAD_CREATE_JOINABLE
#  define PTHREAD_CREATE_JOINABLE	0
#endif

#ifndef PTHREAD_CREATE_DETACHED
#  define PTHREAD_CREATE_DETACHED	0
#endif

#ifndef PTHREAD_SCOPE_SYSTEM
#  define PTHREAD_SCOPE_SYSTEM		0
#endif

#ifdef PLIBSYS_HAS_SCHEDULING
#  ifndef PTHREAD_INHERIT_SCHED
#    define PTHREAD_INHERIT_SCHED	0
#  endif

#  ifndef PTHREAD_EXPLICIT_SCHED
#    define PTHREAD_EXPLICIT_SCHED	0
#  endif
#endif

/* Old Linux kernels may lack a definition */
#if defined (P_OS_LINUX) && !defined (SCHED_IDLE)
#define SCHED_IDLE 5
#endif

typedef pthread_t puthread_hdl;

struct _PUThread {
	puthread_hdl		hdl;
	pboolean		joinable;
	PUThreadPriority	prio;
};

struct _PUThreadKey {
	pthread_key_t		*key;
	PDestroyFunc		free_func;
};

#ifdef PLIBSYS_HAS_SCHEDULING
static pboolean
__puthread_get_unix_priority (PUThreadPriority prio, int *sched_policy, int *sched_priority)
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

	lowBound = ((pint) P_UTHREAD_PRIORITY_LOWEST;
#else
	lowBound = (pint) P_UTHREAD_PRIORITY_IDLE;
#endif
	upperBound = (pint) P_UTHREAD_PRIORITY_TIMECRITICAL;

	prio_min = sched_get_priority_min (*sched_policy);
	prio_max = sched_get_priority_max (*sched_policy);

	if (prio_min == -1 || prio_max == -1)
		return FALSE;

	native_prio = ((pint) prio - lowBound) * (prio_max - prio_min) / upperBound + prio_min;

	if (native_prio > prio_max)
		native_prio = prio_max;

	if (native_prio < prio_min)
		native_prio = prio_min;

	*sched_priority = native_prio;

	return TRUE;
}
#endif

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

static pthread_key_t *
__p_uthread_get_tls_key (PUThreadKey *key)
{
	pthread_key_t *thread_key;

	thread_key = (pthread_key_t *) p_atomic_pointer_get ((ppointer) &key->key);

	if (thread_key == NULL) {
		if ((thread_key = p_malloc0 (sizeof (pthread_key_t))) == NULL) {
			P_ERROR ("PUThread: failed to allocate memory for a TLS key");
			return NULL;
		}

		if (pthread_key_create (thread_key, key->free_func) != 0) {
			P_ERROR ("PUThread: failed to call pthread_key_create()");
			p_free (thread_key);
			return NULL;
		}

		if (!p_atomic_pointer_compare_and_exchange ((ppointer) &key->key,
							    NULL,
							    (ppointer) thread_key)) {
			if (pthread_key_delete (*thread_key) != 0) {
				P_ERROR ("PUThread: failed to call pthread_key_delete()");
				p_free (thread_key);
				return NULL;
			}

			p_free (thread_key);

			thread_key = key->key;
		}
	}

	return thread_key;
}

P_LIB_API PUThread *
p_uthread_create_full (PUThreadFunc	func,
		       ppointer		data,
		       pboolean		joinable,
		       PUThreadPriority	prio)
{
	PUThread		*ret;
	pthread_attr_t		attr;
#ifdef PLIBSYS_HAS_SCHEDULING
	struct sched_param	sched;
	pint			native_prio;
	pint			sched_policy;
	pint			create_code;
#endif

	if (!func)
		return NULL;

	if ((ret = p_malloc0 (sizeof (PUThread))) == NULL) {
		P_ERROR ("PUThread: failed to allocate memory");
		return NULL;
	}

	ret->joinable = joinable;

	if (pthread_attr_init (&attr) != 0) {
		P_ERROR ("PUThread: failed to call pthread_attr_init()");
		p_free (ret);
		return NULL;
	}

	if (pthread_attr_setdetachstate (&attr, joinable ? PTHREAD_CREATE_JOINABLE : PTHREAD_CREATE_DETACHED) != 0) {
		P_ERROR ("PUThread: failed to call pthread_attr_setdetachstate()");
		pthread_attr_destroy (&attr);
		p_free (ret);
		return NULL;
	}

	/* We need this for some old systems where non-bound threads are not timesliced,
	 * see puthread-solaris.c for more explanation */
	if (pthread_attr_setscope (&attr, PTHREAD_SCOPE_SYSTEM) != 0) {
		/* Some systems may fail here due to lack of implementation */
		P_WARNING ("PUThread: failed to call pthread_attr_setscope()");
	}

#ifdef PLIBSYS_HAS_SCHEDULING
	if (prio == P_UTHREAD_PRIORITY_INHERIT) {
		if (pthread_attr_setinheritsched (&attr, PTHREAD_INHERIT_SCHED) != 0)
			P_WARNING ("PUThread: failed to call pthread_attr_setinheritsched()");
	} else {
		if (pthread_attr_getschedpolicy (&attr, &sched_policy) == 0) {
			if (__puthread_get_unix_priority (prio, &sched_policy, &native_prio) == TRUE) {
				sched.sched_priority = native_prio;

				if (pthread_attr_setinheritsched (&attr, PTHREAD_EXPLICIT_SCHED) != 0 ||
				    pthread_attr_setschedpolicy (&attr, sched_policy) != 0 ||
				    pthread_attr_setschedparam (&attr, &sched) != 0)
					P_WARNING ("PUThread: failed to set thread priority");
			} else
				P_WARNING ("PUThread: failed to get native thread priority");
		} else
			P_WARNING ("PUThread: failed to call pthread_attr_getschedpolicy()");
	}
#endif

	create_code = pthread_create (&ret->hdl, &attr, func, data);

#ifdef EPERM
	if (create_code == EPERM) {
#  ifdef PLIBSYS_HAS_SCHEDULING
		pthread_attr_setinheritsched (&attr, PTHREAD_INHERIT_SCHED);
#  endif
		create_code = pthread_create (&ret->hdl, &attr, func, data);
	}
#endif

	if (create_code != 0) {
		P_ERROR ("PUThread: failed to call pthread_create()");
		pthread_attr_destroy (&attr);
		p_free (ret);
		return NULL;
	}

	ret->prio = prio;
	pthread_attr_destroy (&attr);

	return ret;
}

P_LIB_API PUThread *
p_uthread_create (PUThreadFunc		func,
		  ppointer		data,
		  pboolean		joinable)
{
	/* All checks will be inside */
	return p_uthread_create_full (func, data, joinable, P_UTHREAD_PRIORITY_INHERIT);
}

P_LIB_API void
p_uthread_exit (pint code)
{
	pthread_exit (P_INT_TO_POINTER (code));

	/* To smile a compiler */
	while (1);
}

P_LIB_API pint
p_uthread_join (PUThread *thread)
{
	ppointer ret;

	if (!thread || !thread->joinable)
		return -1;

	if (pthread_join (thread->hdl, &ret) != 0) {
		P_ERROR ("PUThread: failed to call pthread_join()");
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
	sched_yield ();
}

P_LIB_API pboolean
p_uthread_set_priority (PUThread		*thread,
			PUThreadPriority	prio)
{
#ifdef PLIBSYS_HAS_SCHEDULING
	struct sched_param	sched;
	pint			policy;
	pint			native_prio;
#endif

	if (thread == NULL)
		return FALSE;

#ifdef PLIBSYS_HAS_SCHEDULING
	if (pthread_getschedparam (thread->hdl, &policy, &sched) != 0) {
		P_ERROR ("PUThread: failed to call pthread_getschedparam()");
		return FALSE;
	}

	if (__puthread_get_unix_priority (prio, &policy, &native_prio) == FALSE) {
		P_ERROR ("PUThread: failed to get native thread priority (2)");
		return FALSE;
	}

	sched.sched_priority = native_prio;

	if (pthread_setschedparam (thread->hdl, policy, &sched) != 0) {
		P_ERROR ("PUThread: failed to call pthread_setschedparam()");
		return FALSE;
	}
#endif

	thread->prio = prio;
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

	if ((ret = p_malloc0 (sizeof (PUThreadKey))) == NULL) {
		P_ERROR ("PUThread: failed to allocate memory for PUThreadKey");
		return NULL;
	}

	ret->free_func = free_func;

	return ret;
}

P_LIB_API void
p_uthread_local_free (PUThreadKey *key)
{
	if (key == NULL)
		return;

	p_free (key);
}

P_LIB_API ppointer
p_uthread_get_local (PUThreadKey *key)
{
	pthread_key_t *tls_key;

	if (key == NULL)
		return NULL;

	tls_key = __p_uthread_get_tls_key (key);

	return tls_key == NULL ? NULL : pthread_getspecific (*tls_key);
}

P_LIB_API void
p_uthread_set_local (PUThreadKey	*key,
		     ppointer		value)
{
	pthread_key_t *tls_key;

	if (key == NULL)
		return;

	tls_key = __p_uthread_get_tls_key (key);

	if (tls_key != NULL) {
		if (pthread_setspecific (*tls_key, value) != 0)
			P_ERROR ("PUThread: failed to call pthread_setspecific()");
	}
}

P_LIB_API void
p_uthread_replace_local	(PUThreadKey	*key,
			 ppointer	value)
{
	pthread_key_t	*tls_key;
	ppointer	old_value;

	if (key == NULL)
		return;

	tls_key = __p_uthread_get_tls_key (key);

	if (tls_key == NULL)
		return;

	old_value = pthread_getspecific (*tls_key);

	if (old_value != NULL && key->free_func != NULL)
		key->free_func (old_value);

	if (pthread_setspecific (*tls_key, value) != 0)
		P_ERROR ("PUThread: failed to call(2) pthread_setspecific()");
}
