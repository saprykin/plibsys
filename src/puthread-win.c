/*
 * The MIT License
 *
 * Copyright (C) 2010-2016 Alexander Saprykin <saprykin.spb@gmail.com>
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
#include "pmutex.h"
#include "patomic.h"
#include "puthread.h"
#include "puthread-private.h"

#include <stdlib.h>
#include <time.h>
#include <string.h>

#include <process.h>

typedef HANDLE puthread_hdl;

struct PUThread_ {
	PUThreadBase		base;
	puthread_hdl		hdl;
	PUThreadFunc		proxy;
};

struct PUThreadKey_ {
	DWORD			key_idx;
	PDestroyFunc		free_func;
};

typedef struct PUThreadDestructor_ PUThreadDestructor;

struct PUThreadDestructor_ {
	DWORD			key_idx;
	PDestroyFunc		free_func;
	PUThreadDestructor	*next;
};

static PUThreadDestructor * volatile pp_uthread_tls_destructors = NULL;
static PMutex *pp_uthread_tls_mutex = NULL;

static DWORD pp_uthread_get_tls_key (PUThreadKey *key);
static puint __stdcall pp_uthread_win32_proxy (ppointer data);

static DWORD
pp_uthread_get_tls_key (PUThreadKey *key)
{
	DWORD tls_key = key->key_idx;

	if (P_LIKELY (tls_key != TLS_OUT_OF_INDEXES))
		return tls_key;

	p_mutex_lock (pp_uthread_tls_mutex);

	tls_key = key->key_idx;

	if (P_LIKELY (tls_key == TLS_OUT_OF_INDEXES)) {
		PUThreadDestructor *destr = NULL;

		tls_key = TlsAlloc ();

		if (P_UNLIKELY (tls_key == TLS_OUT_OF_INDEXES)) {
			P_ERROR ("PUThread::pp_uthread_get_tls_key: TlsAlloc() failed");
			p_mutex_unlock (pp_uthread_tls_mutex);
			return TLS_OUT_OF_INDEXES;
		}

		if (key->free_func != NULL) {
			if (P_UNLIKELY ((destr = p_malloc0 (sizeof (PUThreadDestructor))) == NULL)) {
				P_ERROR ("PUThread::pp_uthread_get_tls_key: failed to allocate memory");

				if (P_UNLIKELY (TlsFree (tls_key) == 0))
					P_ERROR ("PUThread::pp_uthread_get_tls_key: TlsFree() failed(1)");

				p_mutex_unlock (pp_uthread_tls_mutex);
				return TLS_OUT_OF_INDEXES;
			}

			destr->key_idx   = tls_key;
			destr->free_func = key->free_func;
			destr->next      = pp_uthread_tls_destructors;

			/* At the same time thread exit could be performed at there is no
			 * lock for the global destructor list */
			if (P_UNLIKELY (p_atomic_pointer_compare_and_exchange ((PVOID volatile *) &pp_uthread_tls_destructors,
									       (PVOID) destr->next,
									       (PVOID) destr) == FALSE)) {
				P_ERROR ("PUThread::pp_uthread_get_tls_key: p_atomic_pointer_compare_and_exchange() failed");

				if (P_UNLIKELY (TlsFree (tls_key) == 0))
					P_ERROR ("PUThread::pp_uthread_get_tls_key: TlsFree() failed(2)");

				p_free (destr);

				p_mutex_unlock (pp_uthread_tls_mutex);
				return TLS_OUT_OF_INDEXES;
			}
		}

		key->key_idx = tls_key;
	}

	p_mutex_unlock (pp_uthread_tls_mutex);

	return tls_key;
}

static puint __stdcall
pp_uthread_win32_proxy (ppointer data)
{
	PUThread *thread = data;

	thread->proxy (thread);

	_endthreadex (0);

	return 0;
}

void
p_uthread_win32_thread_detach (void)
{
	pboolean was_called;

	do {
		PUThreadDestructor *destr;

		was_called = FALSE;

		destr = (PUThreadDestructor *) p_atomic_pointer_get ((const PVOID volatile *) &pp_uthread_tls_destructors);

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
p_uthread_init_internal (void)
{
	if (P_LIKELY (pp_uthread_tls_mutex == NULL))
		pp_uthread_tls_mutex = p_mutex_new ();
}

void
p_uthread_shutdown_internal (void)
{
	PUThreadDestructor *destr;

	p_uthread_win32_thread_detach ();

	destr = pp_uthread_tls_destructors;

	while (destr != NULL) {
		PUThreadDestructor *next_destr = destr->next;

		TlsFree (destr->key_idx);
		p_free (destr);

		destr = next_destr;
	}

	pp_uthread_tls_destructors = NULL;

	if (P_LIKELY (pp_uthread_tls_mutex != NULL)) {
		p_mutex_free (pp_uthread_tls_mutex);
		pp_uthread_tls_mutex = NULL;
	}
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

	ret->proxy = func;

	if (P_UNLIKELY ((ret->hdl = (HANDLE) _beginthreadex (NULL,
							     (puint) stack_size,
							     pp_uthread_win32_proxy,
							     ret,
							     CREATE_SUSPENDED,
							     NULL)) == NULL)) {
		P_ERROR ("PUThread::p_uthread_create_internal: _beginthreadex() failed");
		p_free (ret);
		return NULL;
	}

	ret->base.joinable = joinable;

	p_uthread_set_priority (ret, prio);

	if (P_UNLIKELY (ResumeThread (ret->hdl) == (DWORD) -1)) {
		P_ERROR ("PUThread::p_uthread_create_internal: ResumeThread() failed");
		CloseHandle (ret->hdl);
		p_free (ret);
	}

	return ret;
}

void
p_uthread_exit_internal (void)
{
	_endthreadex (0);
}

void
p_uthread_wait_internal (PUThread *thread)
{
	if (P_UNLIKELY ((WaitForSingleObject (thread->hdl, INFINITE)) != WAIT_OBJECT_0))
		P_ERROR ("PUThread::p_uthread_wait_internal: WaitForSingleObject() failed");
}

void
p_uthread_free_internal (PUThread *thread)
{
	CloseHandle (thread->hdl);
	p_free (thread);
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
		P_ERROR ("PUThread::p_uthread_set_priority: SetThreadPriority() failed");
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
		P_ERROR ("PUThread::p_uthread_local_new: failed to allocate memory");
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

	tls_idx = pp_uthread_get_tls_key (key);

	return tls_idx == TLS_OUT_OF_INDEXES ? NULL : TlsGetValue (tls_idx);
}

P_LIB_API void
p_uthread_set_local (PUThreadKey	*key,
		     ppointer		value)
{
	DWORD tls_idx;

	if (P_UNLIKELY (key == NULL))
		return;

	tls_idx = pp_uthread_get_tls_key (key);

	if (P_LIKELY (tls_idx != TLS_OUT_OF_INDEXES)) {
		if (P_UNLIKELY (TlsSetValue (tls_idx, value) == 0))
			P_ERROR ("PUThread::p_uthread_set_local: TlsSetValue() failed");
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

	tls_idx = pp_uthread_get_tls_key (key);

	if (P_UNLIKELY (tls_idx == TLS_OUT_OF_INDEXES))
		return;

	old_value = TlsGetValue (tls_idx);

	if (old_value != NULL && key->free_func != NULL)
		key->free_func (old_value);

	if (P_UNLIKELY (TlsSetValue (tls_idx, value) == 0))
		P_ERROR ("PUThread::p_uthread_replace_local: TlsSetValue() failed");
}
