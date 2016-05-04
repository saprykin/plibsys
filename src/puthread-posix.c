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

/* Some systems without native pthreads may lack some of the constants,
 * leave them zero as we are not going to use them anyway */

#ifndef PTHREAD_CREATE_JOINABLE
#  define PTHREAD_CREATE_JOINABLE 0
#endif

#ifndef PTHREAD_CREATE_DETACHED
#  define PTHREAD_CREATE_DETACHED 0
#endif

#ifndef PTHREAD_SCOPE_SYSTEM
#  define PTHREAD_SCOPE_SYSTEM 0
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

#if defined (POSIX_MIN_PRIORITY) && defined (POSIX_MAX_PRIORITY)
#  define P_HAVE_PRIORITIES
#endif

#ifdef P_HAVE_PRIORITIES
static int p_uthread_priority_map[P_UTHREAD_PRIORITY_HIGHEST + 1];
#endif

void
__p_uthread_init (void)
{
#ifdef P_HAVE_PRIORITIES
	int			min_prio, max_prio, normal_prio;
	int			policy;
	struct sched_param	sched;

	min_prio	= 0;
	max_prio	= 0;
	normal_prio	= 0;

# ifdef P_OS_FREEBSD
	/* FreeBSD threads use different priority values from the POSIX_
	 * defines so we just set them here. The corresponding macros
	 * PTHREAD_MIN_PRIORITY and PTHREAD_MAX_PRIORITY are implied to be
	 * exported by the docs, but they aren't.
	 */
	min_prio = 0;
	max_prio = 31;
# else /* !P_OS_FREEBSD */
	min_prio = POSIX_MIN_PRIORITY;
	max_prio = POSIX_MAX_PRIORITY;
# endif /* !P_OS_FREEBSD */
	pthread_getschedparam (pthread_self (), &policy, &sched);
	normal_prio = sched.sched_priority;

	p_uthread_priority_map[P_UTHREAD_PRIORITY_LOWEST]	= min_prio;
	p_uthread_priority_map[P_UTHREAD_PRIORITY_LOW]		= (min_prio * 6 + normal_prio * 4) / 10;
	p_uthread_priority_map[P_UTHREAD_PRIORITY_NORMAL]	= normal_prio;
	p_uthread_priority_map[P_UTHREAD_PRIORITY_HIGH]		= (normal_prio + max_prio * 2) / 3;
	p_uthread_priority_map[P_UTHREAD_PRIORITY_HIGHEST]	= max_prio;
#endif /* POSIX_MIN_PRIORITY && POSIX_MAX_PRIORITY */
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
#ifdef P_HAVE_PRIORITIES
	struct sched_param	sched;
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

#ifdef P_HAVE_PRIORITIES
	if (prio > P_UTHREAD_PRIORITY_HIGHEST || prio < P_UTHREAD_PRIORITY_LOWEST)
		prio = P_UTHREAD_PRIORITY_NORMAL;

	if (pthread_attr_getschedparam (&attr, &sched) == 0) {
		sched.sched_priority = p_uthread_priority_map[prio];

		if (pthread_attr_setschedparam (&attr, &sched) != 0)
			P_WARNING ("PUThread: failed to call pthread_attr_setschedparam(), "
				   "maybe you don't have enough rights");
	} else
		P_WARNING ("PUThread: failed to call pthread_attr_getschedparam()");
#endif

	if (pthread_create (&ret->hdl, &attr, func, data) != 0) {
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
	return p_uthread_create_full (func, data, joinable, P_UTHREAD_PRIORITY_NORMAL);
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

P_LIB_API pint
p_uthread_set_priority (PUThread		*thread,
			PUThreadPriority	prio)
{
#ifdef P_HAVE_PRIORITIES
	struct sched_param	sched;
	int			policy;
#endif

	if (thread == NULL)
		return -1;

#ifdef P_HAVE_PRIORITIES
	if (prio > P_UTHREAD_PRIORITY_HIGHEST || prio < P_UTHREAD_PRIORITY_LOWEST) {
		P_WARNING ("PUThread: trying to assign wrong thread priority");
		return -1;
	}

	if (pthread_getschedparam (thread->hdl, &policy, &sched) != 0) {
		P_ERROR ("PUThread: failed to call(2) pthread_getschedparam()");
		return -1;
	}

	sched.sched_priority = p_uthread_priority_map [prio];

	if (pthread_setschedparam (thread->hdl, policy, &sched) != 0) {
		P_ERROR ("PUThread: failed to call(2) pthread_setschedparam(), "
			 "maybe you don't have enough rights");
		return -1;
	}
#endif

	thread->prio = prio;
	return 0;
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
