/*
 * Copyright (C) 2016 Alexander Saprykin <xelfium@gmail.com>
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, see <http://www.gnu.org/licenses/>.
 */

#include "pmem.h"
#include "patomic.h"
#include "pmutex.h"
#include "puthread.h"
#include "puthread-private.h"

#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include <kernel/OS.h>
#include <kernel/scheduler.h>
#include <support/TLS.h>

typedef thread_id puthread_hdl;

struct PUThread_ {
	PUThreadBase	base;
	puthread_hdl	hdl;
};

struct PUThreadKey_ {
	pint		key;
	PDestroyFunc	free_func;
};

typedef struct PUThreadDestructor_ PUThreadDestructor;

struct PUThreadDestructor_ {
	pint			key_idx;
	PDestroyFunc		free_func;
	PUThreadDestructor	*next;
};

static PUThreadDestructor * volatile pp_uthread_tls_destructors = NULL;

static PMutex *pp_uthread_tls_mutex = NULL;

static pint pp_uthread_get_beos_priority (PUThreadPriority prio);
static pint pp_uthread_get_tls_key (PUThreadKey *key);
static void pp_uthread_clean_destructors (void);

static pint
pp_uthread_get_beos_priority (PUThreadPriority prio)
{
	switch (prio) {
		case P_UTHREAD_PRIORITY_INHERIT:
		{
			thread_info thr_info;
			
			memset (&thr_info, 0, sizeof (thr_info));

			if (P_UNLIKELY (get_thread_info (find_thread (NULL), &thr_info) != B_OK)) {
				P_WARNING ("PUThread::pp_uthread_get_beos_priority: failed to get thread info");
				return B_NORMAL_PRIORITY;
			} else
				return thr_info.priority;
		}

		case P_UTHREAD_PRIORITY_IDLE:
			return B_LOW_PRIORITY;
		case P_UTHREAD_PRIORITY_LOWEST:
			return B_NORMAL_PRIORITY / 4;
		case P_UTHREAD_PRIORITY_LOW:
			return B_NORMAL_PRIORITY / 2;
		case P_UTHREAD_PRIORITY_NORMAL:
			return B_NORMAL_PRIORITY;
		case P_UTHREAD_PRIORITY_HIGH:
			return B_DISPLAY_PRIORITY;
		case P_UTHREAD_PRIORITY_HIGHEST:
			return B_URGENT_DISPLAY_PRIORITY;
		case P_UTHREAD_PRIORITY_TIMECRITICAL:
			return B_REAL_TIME_PRIORITY;
	}
}

static pint
pp_uthread_get_tls_key (PUThreadKey *key)
{
	pint thread_key;

	thread_key = p_atomic_int_get ((const volatile pint *) &key->key);

	if (P_LIKELY (thread_key >= 0))
		return thread_key;

	p_mutex_lock (pp_uthread_tls_mutex);

	thread_key = key->key;

	if (P_LIKELY (thread_key == -1)) {
		PUThreadDestructor *destr = NULL;

		if (key->free_func != NULL) {
			if (P_UNLIKELY ((destr = p_malloc0 (sizeof (PUThreadDestructor))) == NULL)) {
				P_ERROR ("PUThread::pp_uthread_get_tls_key: failed to allocate memory");
				p_mutex_unlock (pp_uthread_tls_mutex);
				return -1;
			}
		}

		if (P_UNLIKELY ((thread_key = tls_allocate ()) < 0)) {
			P_ERROR ("PUThread::pp_uthread_get_tls_key: tls_allocate() failed");
			p_free (destr);
			p_mutex_unlock (pp_uthread_tls_mutex);
			return -1;
		}

		if (destr != NULL) {
			destr->key_idx   = thread_key;
			destr->free_func = key->free_func;
			destr->next      = pp_uthread_tls_destructors;

			/* At the same time thread exit could be performed at there is no
			 * lock for the global destructor list */
			if (P_UNLIKELY (p_atomic_pointer_compare_and_exchange ((void * volatile *) &pp_uthread_tls_destructors,
									       (void *) destr->next,
									       (void *) destr) == FALSE)) {
				P_ERROR ("PUThread::pp_uthread_get_tls_key: p_atomic_pointer_compare_and_exchange() failed");
				p_free (destr);
				p_mutex_unlock (pp_uthread_tls_mutex);
				return -1;
			}
		}

		key->key = thread_key;
	}

	p_mutex_unlock (pp_uthread_tls_mutex);

	return thread_key;
}

static void 
pp_uthread_clean_destructors (void)
{
	pboolean was_called;

	do {
		PUThreadDestructor *destr;

		was_called = FALSE;

		destr = (PUThreadDestructor *) p_atomic_pointer_get ((const void * volatile *) &pp_uthread_tls_destructors);

		while (destr != NULL) {
			ppointer value;

			value = tls_get (destr->key_idx);

			if (value != NULL && destr->free_func != NULL) {
				tls_set (destr->key_idx, NULL);
				destr->free_func (value);
				was_called = TRUE;
			}

			destr = destr->next;
		}
	} while (was_called);
}

void
p_uthread_init_internal (void)
{
	if (P_LIKELY (pp_uthread_tls_mutex == NULL))
		pp_uthread_tls_mutex = p_mutex_new ();
}

void
p_uthread_shutdown_internal (void)
{
	PUThreadDestructor *destr;

	pp_uthread_clean_destructors ();

	destr = pp_uthread_tls_destructors;

	while (destr != NULL) {
		PUThreadDestructor *next_destr = destr->next;

		p_free (destr);
		destr = next_destr;
	}

	pp_uthread_tls_destructors = NULL;

	if (P_LIKELY (pp_uthread_tls_mutex != NULL)) {
		p_mutex_free (pp_uthread_tls_mutex);
		pp_uthread_tls_mutex = NULL;
	}
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

	P_UNUSED (stack_size);

	if (P_UNLIKELY ((ret = p_malloc0 (sizeof (PUThread))) == NULL)) {
		P_ERROR ("PUThread::p_uthread_create_internal: failed to allocate memory");
		return NULL;
	}

	if (P_UNLIKELY ((ret->hdl = spawn_thread ((thread_func) func,
						  "",
						  pp_uthread_get_beos_priority (prio),
						  ret)) < B_OK)) {
		P_ERROR ("PUThread::p_uthread_create_internal: spawn_thread() failed");
		p_free (ret);
		return NULL;
	}

	if (P_UNLIKELY (resume_thread (ret->hdl) != B_OK)) {
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
	pp_uthread_clean_destructors ();
	exit_thread (0);
}

void
p_uthread_wait_internal (PUThread *thread)
{
	status_t exit_value;

	wait_for_thread (thread->hdl, &exit_value);
}

void
p_uthread_free_internal (PUThread *thread)
{
	p_free (thread);
}

P_LIB_API void
p_uthread_yield (void)
{
	snooze ((bigtime_t) 0);
}

P_LIB_API pboolean
p_uthread_set_priority (PUThread		*thread,
			PUThreadPriority	prio)
{
	if (P_UNLIKELY (thread == NULL))
		return FALSE;

	if (set_thread_priority (thread->hdl, pp_uthread_get_beos_priority (prio)) < B_OK) {
		P_ERROR ("PUThread::p_uthread_create_internal: set_thread_priority() failed");
		return FALSE;
	}

	thread->base.prio = prio;

	return TRUE;
}

P_LIB_API P_HANDLE
p_uthread_current_id (void)
{
	return (P_HANDLE) ((psize) find_thread (NULL));
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
		return tls_get (tls_key);

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
		tls_set (tls_key, value);
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

	old_value = tls_get (tls_key);

	if (old_value != NULL && key->free_func != NULL)
		key->free_func (old_value);

	tls_set (tls_key, value);
}
