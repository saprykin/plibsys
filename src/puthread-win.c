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
#include "pmutex.h"
#include "patomic.h"
#include "plibsys-private.h"

#include <stdlib.h>
#include <time.h>
#include <string.h>

#include <windows.h>

typedef HANDLE puthread_hdl;

struct _PUThread {
	__PUThreadBase		base;
	puthread_hdl		hdl;
};

struct _PUThreadKey {
	DWORD			key_idx;
	PDestroyFunc		free_func;
};

typedef struct __PUThreadDestructor _PUThreadDestructor;

struct __PUThreadDestructor {
	DWORD			key_idx;
	PDestroyFunc		free_func;
	_PUThreadDestructor	*next;
};

static _PUThreadDestructor * volatile __tls_destructors = NULL;
static PMutex *__tls_mutex = NULL;

static DWORD
__p_uthread_get_tls_key (PUThreadKey *key)
{
	DWORD tls_key = key->key_idx;

	if (P_LIKELY (tls_key != TLS_OUT_OF_INDEXES))
		return tls_key;

	p_mutex_lock (__tls_mutex);

	tls_key = key->key_idx;

	if (P_LIKELY (tls_key == TLS_OUT_OF_INDEXES)) {
		_PUThreadDestructor *destr = NULL;

		tls_key = TlsAlloc ();

		if (P_UNLIKELY (tls_key == TLS_OUT_OF_INDEXES)) {
			P_ERROR ("PUThread: failed to call TlsAlloc()");
			p_mutex_unlock (__tls_mutex);
			return TLS_OUT_OF_INDEXES;
		}

		if (key->free_func != NULL) {
			if (P_UNLIKELY ((destr = p_malloc0 (sizeof (_PUThreadDestructor))) == NULL)) {
				P_ERROR ("PUThread: failed to allocate memory for a TLS destructor");

				if (P_UNLIKELY (TlsFree (tls_key) == 0))
					P_ERROR ("PUThread: failed to call TlsFree()");

				p_mutex_unlock (__tls_mutex);
				return TLS_OUT_OF_INDEXES;
			}

			destr->key_idx   = tls_key;
			destr->free_func = key->free_func;
			destr->next      = __tls_destructors;

			/* At the same time thread exit could be performed at there is no
			 * lock for the global destructor list */
			if (P_UNLIKELY (p_atomic_pointer_compare_and_exchange ((PVOID volatile *) &__tls_destructors,
									       (PVOID) destr->next,
									       (PVOID) destr) == FALSE)) {
				P_ERROR ("PUThread: failed to setup a TLS key destructor");

				if (P_UNLIKELY (TlsFree (tls_key) == 0))
					P_ERROR ("PUThread: failed to call(2) TlsFree()");

				p_free (destr);

				p_mutex_unlock (__tls_mutex);
				return TLS_OUT_OF_INDEXES;
			}
		}

		key->key_idx = tls_key;
	}

	p_mutex_unlock (__tls_mutex);

	return tls_key;
}

void
__p_uthread_win32_thread_detach (void)
{
	pboolean was_called;

	do {
		_PUThreadDestructor *destr;

		was_called = FALSE;

		destr = (_PUThreadDestructor *) p_atomic_pointer_get ((const PVOID volatile *) &__tls_destructors);

		while (destr != NULL) {
			ppointer value;

			value = TlsGetValue (destr->key_idx);

			if (value != NULL && destr->free_func != NULL) {
				TlsSetValue (destr->key_idx, NULL);
				destr->free_func (value);
				was_called = TRUE;
			}

			destr = destr->next;
		}
	} while (was_called);
}

void
__p_uthread_init_internal (void)
{
	if (P_LIKELY (__tls_mutex == NULL))
		__tls_mutex = p_mutex_new ();
}

void
__p_uthread_shutdown_internal (void)
{
	_PUThreadDestructor *destr;

	if (P_LIKELY (__tls_mutex != NULL)) {
		p_mutex_free (__tls_mutex);
		__tls_mutex = NULL;
	}

	__p_uthread_win32_thread_detach ();

	destr = __tls_destructors;

	while (destr != NULL) {
		_PUThreadDestructor *next_destr = destr->next;

		TlsFree (destr->key_idx);
		p_free (destr);

		destr = next_destr;
	}

	__tls_destructors = NULL;
}

PUThread *
__p_uthread_create_internal (PUThreadFunc	func,
			     pboolean		joinable,
			     PUThreadPriority	prio)
{
	PUThread	*ret;

	if (P_UNLIKELY ((ret = p_malloc0 (sizeof (PUThread))) == NULL)) {
		P_ERROR ("PUThread: failed to allocate memory");
		return NULL;
	}

	if (P_UNLIKELY ((ret->hdl = CreateThread (NULL,
						  0,
						  (LPTHREAD_START_ROUTINE) func,
						  ret,
						  CREATE_SUSPENDED,
						  NULL)) == NULL)) {
		P_ERROR ("PUThread: failed to call CreateThread()");
		p_free (ret);
		return NULL;
	}

	ret->base.joinable = joinable;

	p_uthread_set_priority (ret, prio);

	if (P_UNLIKELY (ResumeThread (ret->hdl) == (DWORD) -1)) {
		P_ERROR ("PUThread: failed to call ResumeThread()");
		CloseHandle (ret->hdl);
		p_free (ret);
	}

	return ret;
}

void
__p_uthread_free_internal (PUThread *thread)
{
	CloseHandle (thread->hdl);
	p_free (thread);
}

P_LIB_API void
p_uthread_exit (pint code)
{
	ExitThread ((DWORD) code);
}

P_LIB_API pint
p_uthread_join (PUThread *thread)
{
	DWORD exit_code;

	if (P_UNLIKELY (thread == NULL))
		return -1;

	if (thread->base.joinable == FALSE)
		return -1;

	if (P_UNLIKELY ((WaitForSingleObject (thread->hdl, INFINITE)) != WAIT_OBJECT_0)) {
		P_ERROR ("PUThread: failed to call WaitForSingleObject() to join a thread");
		return -1;
	}

	if (P_UNLIKELY (GetExitCodeThread (thread->hdl, &exit_code) == 0)) {
		P_ERROR ("PUThread: failed to call GetExitCodeThread()");
		return -1;
	}

	return exit_code;
}

P_LIB_API void
p_uthread_yield (void)
{
	Sleep (0);
}

P_LIB_API pboolean
p_uthread_set_priority (PUThread		*thread,
			PUThreadPriority	prio)
{
	pint native_prio;

	if (P_UNLIKELY (thread == NULL))
		return FALSE;

	switch (prio) {
	case P_UTHREAD_PRIORITY_IDLE:
		native_prio = THREAD_PRIORITY_IDLE;
		break;
	case P_UTHREAD_PRIORITY_LOWEST:
		native_prio = THREAD_PRIORITY_LOWEST;
		break;
	case P_UTHREAD_PRIORITY_LOW:
		native_prio = THREAD_PRIORITY_BELOW_NORMAL;
		break;
	case P_UTHREAD_PRIORITY_NORMAL:
		native_prio = THREAD_PRIORITY_NORMAL;
		break;
	case P_UTHREAD_PRIORITY_HIGH:
		native_prio = THREAD_PRIORITY_ABOVE_NORMAL;
		break;
	case P_UTHREAD_PRIORITY_HIGHEST:
		native_prio = THREAD_PRIORITY_HIGHEST;
		break;
	case P_UTHREAD_PRIORITY_TIMECRITICAL:
		native_prio = THREAD_PRIORITY_TIME_CRITICAL;
		break;
	case P_UTHREAD_PRIORITY_INHERIT:
	default:
		native_prio = GetThreadPriority (GetCurrentThread ());
		break;
	}

	if (P_UNLIKELY (SetThreadPriority (thread->hdl, native_prio) == 0)) {
		P_ERROR ("PUThread: failed to call SetThreadPriority()");
		return FALSE;
	}

	thread->base.prio = prio;

	return TRUE;
}

P_LIB_API P_HANDLE
p_uthread_current_id (void)
{
	return (P_HANDLE) ((psize) GetCurrentThreadId ());
}

P_LIB_API PUThreadKey *
p_uthread_local_new (PDestroyFunc free_func)
{
	PUThreadKey *ret;

	if (P_UNLIKELY ((ret = p_malloc0 (sizeof (PUThreadKey))) == NULL)) {
		P_ERROR ("PUThread: failed to allocate memory for PUThreadKey");
		return NULL;
	}

	ret->key_idx   = TLS_OUT_OF_INDEXES;
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
	DWORD tls_idx;

	if (P_UNLIKELY (key == NULL))
		return NULL;

	tls_idx = __p_uthread_get_tls_key (key);

	return tls_idx == TLS_OUT_OF_INDEXES ? NULL : TlsGetValue (tls_idx);
}

P_LIB_API void
p_uthread_set_local (PUThreadKey	*key,
		     ppointer		value)
{
	DWORD tls_idx;

	if (P_UNLIKELY (key == NULL))
		return;

	tls_idx = __p_uthread_get_tls_key (key);

	if (P_LIKELY (tls_idx != TLS_OUT_OF_INDEXES)) {
		if (P_UNLIKELY (TlsSetValue (tls_idx, value) == 0))
			P_ERROR ("PUThread: failed to call TlsSetValue()");
	}
}

P_LIB_API void
p_uthread_replace_local	(PUThreadKey	*key,
			 ppointer	value)
{
	DWORD		tls_idx;
	ppointer	old_value;

	if (P_UNLIKELY (key == NULL))
		return;

	tls_idx = __p_uthread_get_tls_key (key);

	if (P_UNLIKELY (tls_idx == TLS_OUT_OF_INDEXES))
		return;

	old_value = TlsGetValue (tls_idx);

	if (old_value != NULL && key->free_func != NULL)
		key->free_func (old_value);

	if (P_UNLIKELY (TlsSetValue (tls_idx, value) == 0))
		P_ERROR ("PUThread: failed to call(2) TlsSetValue()");
}
