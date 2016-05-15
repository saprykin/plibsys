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

#ifndef P_OS_UNIXWARE
#  include "pmutex.h"
#endif

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

struct _PUThreadKey {
	thread_key_t		*key;
	PDestroyFunc		free_func;
};

#ifndef P_OS_UNIXWARE
static PMutex *__tls_mutex = NULL;
#endif

void
__p_uthread_init (void)
{
#ifndef P_OS_UNIXWARE
	if (__tls_mutex == NULL)
		__tls_mutex = p_mutex_new ();
#endif
}

void
__p_uthread_shutdown (void)
{
#ifndef P_OS_UNIXWARE
	if (__tls_mutex != NULL) {
		p_mutex_free (__tls_mutex);
		__tls_mutex = NULL;
	}
#endif
}

void
__p_uthread_win32_thread_detach (void)
{
}

static thread_key_t *
__p_uthread_get_tls_key (PUThreadKey *key)
{
	thread_key_t *thread_key;

	thread_key = (thread_key_t *) p_atomic_pointer_get ((ppointer) &key->key);

	if (thread_key != NULL)
		return thread_key;

#ifndef P_OS_UNIXWARE
	p_mutex_lock (__tls_mutex);

	if (thread_key == NULL) {
#endif
		if ((thread_key = p_malloc0 (sizeof (thread_key_t))) == NULL) {
			P_ERROR ("PUThread: failed to allocate memory for a TLS key");
#ifndef P_OS_UNIXWARE
			p_mutex_unlock (__tls_mutex);
#endif
			return NULL;
		}

		if (thr_keycreate (thread_key, key->free_func) != 0) {
			P_ERROR ("PUThread: failed to call thr_keycreate()");
			p_free (thread_key);
#ifndef P_OS_UNIXWARE
			p_mutex_unlock (__tls_mutex);
#endif
			return NULL;
		}
#ifdef P_OS_UNIXWARE
		if (!p_atomic_pointer_compare_and_exchange ((ppointer) &key->key,
							    NULL,
							    (ppointer) thread_key)) {
			if (thr_keydelete (*thread_key) != 0) {
				P_ERROR ("PUThread: failed to call thr_keydelete()");
				p_free (thread_key);
				return NULL;
			}

			p_free (thread_key);

			thread_key = key->key;
		}
#else
		key->key = thread_key;
	}

	p_mutex_unlock (__tls_mutex);
#endif

	return thread_key;
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

	flags = 0 : THR_DETACHED;

	if (thr_create (NULL, 0, func, data, flags, &ret->hdl) != 0) {
		P_ERROR ("PUThread: failed to call thr_create()");
		p_uthread_free (ret);
		return NULL;
	}

	ret->joinable	= joinable;
	ret->prio	= prio;

	return ret;
}

P_LIB_API PUThread *
p_uthread_create (PUThreadFunc	func,
		  ppointer	data,
		  pboolean	joinable)
{
	/* All checks will be inside */
	return p_uthread_create_full (func, data, joinable, P_UTHREAD_PRIORITY_INHERIT);
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
		P_ERROR ("PUThread: failed to call thr_join()");
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

P_LIB_API pboolean
p_uthread_set_priority (PUThread		*thread,
			PUThreadPriority	prio)
{
	P_WARNING ("PUThread: priorities for bound threads are not implemented");

	if (thread == NULL)
		return FALSE;

	thread->prio = prio;

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
	thread_key_t	*tls_key;
	ppointer	ret = NULL;

	if (key == NULL)
		return ret;

	tls_key = __p_uthread_get_tls_key (key);

	if (tls_key != NULL) {
		if (thr_getspecific (*tls_key, &ret) != 0)
			P_ERROR ("PUThread: failed to call thr_getspecific()");
	}

	return ret;
}

P_LIB_API void
p_uthread_set_local (PUThreadKey	*key,
		     ppointer		value)
{
	thread_key_t *tls_key;

	if (key == NULL)
		return;

	tls_key = __p_uthread_get_tls_key (key);

	if (tls_key != NULL) {
		if (thr_setspecific (*tls_key, value) != 0)
			P_ERROR ("PUThread: failed to call thr_setspecific()");
	}
}

P_LIB_API void
p_uthread_replace_local	(PUThreadKey	*key,
			 ppointer	value)
{
	thread_key_t	*tls_key;
	ppointer	old_value = NULL;

	if (key == NULL)
		return;

	tls_key = __p_uthread_get_tls_key (key);

	if (tls_key == NULL)
		return;

	if (thr_getspecific (*tls_key, &old_value) != 0) {
		P_ERROR ("PUThread: failed to call(2) thr_getspecific()");
		return;
	}

	if (old_value != NULL && key->free_func != NULL)
		key->free_func (old_value);

	if (thr_setspecific (*tls_key, value) != 0)
		P_ERROR ("PUThread: failed to call(2) thr_setspecific()");
}
