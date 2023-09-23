/*
 * The MIT License
 *
 * Copyright (C) 2017-2019 Alexander Saprykin <saprykin.spb@gmail.com>
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

#define INCL_DOSPROCESS
#define INCL_DOSERRORS
#include <os2.h>
#include <process.h>

#ifdef P_DEBUG
#  undef P_DEBUG
#endif

#include "pmem.h"
#include "patomic.h"
#include "pmutex.h"
#include "puthread.h"
#include "puthread-private.h"

#include <stdlib.h>

typedef TID puthread_hdl;

struct PUThread_ {
	PUThreadBase	base;
	puthread_hdl	hdl;
	PUThreadFunc	proxy;
};

struct PUThreadKey_ {
	PULONG		key;
	PDestroyFunc	free_func;
};

typedef struct PUThreadDestructor_ PUThreadDestructor;

struct PUThreadDestructor_ {
	PULONG			key;
	PDestroyFunc		free_func;
	PUThreadDestructor	*next;
};

static PUThreadDestructor * volatile pp_uthread_tls_destructors = NULL;
static PMutex *pp_uthread_tls_mutex = NULL;

static void pp_uthread_get_os2_priority (PUThreadPriority prio, PULONG thr_class, PLONG thr_level);
static PULONG pp_uthread_get_tls_key (PUThreadKey *key);
static void pp_uthread_clean_destructors (void);
static void pp_uthread_os2_proxy (ppointer data);

static void
pp_uthread_get_os2_priority (PUThreadPriority prio, PULONG thr_class, PLONG thr_level)
{
	switch (prio) {
		case P_UTHREAD_PRIORITY_INHERIT:
		{
			APIRET	ulrc;
			PTIB	ptib = NULL;

			if (P_UNLIKELY (DosGetInfoBlocks (&ptib, NULL) != NO_ERROR)) {
				P_WARNING ("PUThread::pp_uthread_get_os2_priority: DosGetInfoBlocks() failed");
				*thr_class = PRTYC_REGULAR;
				*thr_level = 0;
			} else {
				*thr_class = ((ptib->tib_ptib2->tib2_ulpri) >> 8) & 0x00FF;
				*thr_level = (ptib->tib_ptib2->tib2_ulpri) & 0x001F;
			}

			return;
		}
		case P_UTHREAD_PRIORITY_IDLE:
		{
			*thr_class = PRTYC_IDLETIME;
			*thr_level = 0;

			return;
		}
		case P_UTHREAD_PRIORITY_LOWEST:
		{
			*thr_class = PRTYC_REGULAR;
			*thr_level = PRTYD_MINIMUM;

			return;
		}
		case P_UTHREAD_PRIORITY_LOW:
		{
			*thr_class = PRTYC_REGULAR;
			*thr_level = PRTYD_MINIMUM / 2;

			return;
		}
		case P_UTHREAD_PRIORITY_NORMAL:
		{
			*thr_class = PRTYC_REGULAR;
			*thr_level = 0;

			return;
		}
		case P_UTHREAD_PRIORITY_HIGH:
		{
			*thr_class = PRTYC_REGULAR;
			*thr_level = PRTYD_MAXIMUM / 2;

			return;
		}
		case P_UTHREAD_PRIORITY_HIGHEST:
		{
			*thr_class = PRTYC_REGULAR;
			*thr_level = PRTYD_MAXIMUM;

			return;
		}
		case P_UTHREAD_PRIORITY_TIMECRITICAL:
		{
			*thr_class = PRTYC_TIMECRITICAL;
			*thr_level = 0;

			return;
		}
	}
}

static PULONG
pp_uthread_get_tls_key (PUThreadKey *key)
{
	PULONG thread_key;

	thread_key = (PULONG) p_atomic_pointer_get ((ppointer) &key->key);

	if (P_LIKELY (thread_key != NULL))
		return thread_key;

	p_mutex_lock (pp_uthread_tls_mutex);

	thread_key = key->key;

	if (P_LIKELY (thread_key == NULL)) {
		PUThreadDestructor *destr = NULL;

		if (key->free_func != NULL) {
			if (P_UNLIKELY ((destr = p_malloc0 (sizeof (PUThreadDestructor))) == NULL)) {
				P_ERROR ("PUThread::pp_uthread_get_tls_key: failed to allocate memory");
				p_mutex_unlock (pp_uthread_tls_mutex);
				return NULL;
			}
		}

		if (P_UNLIKELY (DosAllocThreadLocalMemory (1, &thread_key) != NO_ERROR)) {
			P_ERROR ("PUThread::pp_uthread_get_tls_key: DosAllocThreadLocalMemory() failed");
			p_free (destr);
			p_mutex_unlock (pp_uthread_tls_mutex);
			return NULL;
		}

		if (destr != NULL) {
			destr->key       = thread_key;
			destr->free_func = key->free_func;
			destr->next      = pp_uthread_tls_destructors;

			/* At the same time thread exit could be performed at there is no
			 * lock for the global destructor list */
			if (P_UNLIKELY (p_atomic_pointer_compare_and_exchange ((void * volatile *) &pp_uthread_tls_destructors,
									       (void *) destr->next,
									       (void *) destr) == FALSE)) {
				P_ERROR ("PUThread::pp_uthread_get_tls_key: p_atomic_pointer_compare_and_exchange() failed");

				if (P_UNLIKELY (DosFreeThreadLocalMemory (thread_key) != NO_ERROR))
					P_ERROR ("PUThread::pp_uthread_get_tls_key: DosFreeThreadLocalMemory() failed");

				p_free (destr);
				p_mutex_unlock (pp_uthread_tls_mutex);
				return NULL;
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
			PULONG value;

			value = destr->key;

			if (value != NULL && ((ppointer) *value) != NULL && destr->free_func != NULL) {
				*destr->key = (ULONG) NULL;
				destr->free_func ((ppointer) *value);
				was_called = TRUE;
			}

			destr = destr->next;
		}
	} while (was_called);
}

static void
pp_uthread_os2_proxy (ppointer data)
{
	PUThread *thread = data;

	thread->proxy (thread);

	pp_uthread_clean_destructors ();
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

	if (P_UNLIKELY ((ret = p_malloc0 (sizeof (PUThread))) == NULL)) {
		P_ERROR ("PUThread::p_uthread_create_internal: failed to allocate memory");
		return NULL;
	}

	ret->base.joinable = joinable;
	ret->proxy         = func;

	if (P_UNLIKELY ((ret->hdl = _beginthread ((void (*) (void *)) pp_uthread_os2_proxy,
						  NULL,
						  (puint) stack_size,
						  ret)) <= 0)) {
		P_ERROR ("PUThread::p_uthread_create_internal: _beginthread() failed");
		p_free (ret);
		return NULL;
	}

	ret->base.prio = P_UTHREAD_PRIORITY_INHERIT;
	p_uthread_set_priority (ret, prio);

	return ret;
}

void
p_uthread_exit_internal (void)
{
	pp_uthread_clean_destructors ();
	_endthread ();
}

void
p_uthread_wait_internal (PUThread *thread)
{
	APIRET ulrc;

	while ((ulrc = DosWaitThread (&thread->hdl, DCWW_WAIT)) == ERROR_INTERRUPT)
		;

	if (P_UNLIKELY (ulrc != NO_ERROR && ulrc != ERROR_INVALID_THREADID))
		P_ERROR ("PUThread::p_uthread_wait_internal: DosWaitThread() failed");
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
	DosSleep (0);
}

P_LIB_API pboolean
p_uthread_set_priority (PUThread		*thread,
			PUThreadPriority	prio)
{
	APIRET	ulrc;
	PTIB	ptib = NULL;
	LONG	cur_level;
	LONG	new_level;
	ULONG	new_class;

	if (P_UNLIKELY (thread == NULL))
		return FALSE;

	if (P_UNLIKELY (DosGetInfoBlocks (&ptib, NULL) != NO_ERROR)) {
		P_WARNING ("PUThread::p_uthread_set_priority: DosGetInfoBlocks() failed");
		return FALSE;
	}

	cur_level = (ptib->tib_ptib2->tib2_ulpri) & 0x001F;

	pp_uthread_get_os2_priority (prio, &new_class, &new_level);

	if (P_UNLIKELY (DosSetPriority (PRTYS_THREAD, new_class, new_level - cur_level, 0) != NO_ERROR)) {
		P_WARNING ("PUThread::p_uthread_set_priority: DosSetPriority() failed");
		return FALSE;
	}

	thread->base.prio = prio;
	return TRUE;
}

P_LIB_API P_HANDLE
p_uthread_current_id (void)
{
	return (P_HANDLE) (_gettid ());
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
	PULONG tls_key;

	if (P_UNLIKELY (key == NULL))
		return NULL;

	if (P_UNLIKELY ((tls_key = pp_uthread_get_tls_key (key)) == NULL))
		return NULL;

	return (ppointer) *tls_key;
}

P_LIB_API void
p_uthread_set_local (PUThreadKey	*key,
		     ppointer		value)
{
	PULONG tls_key;

	if (P_UNLIKELY (key == NULL))
		return;

	tls_key = pp_uthread_get_tls_key (key);

	if (P_LIKELY (tls_key != NULL))
		*tls_key = (ULONG) value;
}

P_LIB_API void
p_uthread_replace_local	(PUThreadKey	*key,
			 ppointer	value)
{
	PULONG		tls_key;
	ppointer	old_value;

	if (P_UNLIKELY (key == NULL))
		return;

	tls_key = pp_uthread_get_tls_key (key);

	if (P_UNLIKELY (tls_key == NULL))
		return;

	old_value = (ppointer) *tls_key;

	if (old_value != NULL && key->free_func != NULL)
		key->free_func (old_value);

	*tls_key = (ULONG) value;
}
