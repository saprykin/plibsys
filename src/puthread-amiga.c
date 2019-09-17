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

#include "pmem.h"
#include "patomic.h"
#include "pcondvariable.h"
#include "plist.h"
#include "pmutex.h"
#include "puthread.h"
#include "puthread-private.h"

#include <stdlib.h>
#include <string.h>
#include <setjmp.h>

#include <proto/exec.h>
#include <proto/dos.h>

#define PUTHREAD_AMIGA_MAX_TLS_KEYS 128
#define PUTHREAD_AMIGA_MIN_STACK    524288
#define PUTHREAD_AMIGA_MAX_CLEANS   4

typedef struct {
	pboolean 	in_use;
	PDestroyFunc	free_func;
} PUThreadTLSKey;

typedef struct {
	pint		id;
	struct Task	*task;
	jmp_buf		jmpbuf;
	ppointer	tls_values[PUTHREAD_AMIGA_MAX_TLS_KEYS];
} PUThreadInfo;

struct PUThread_ {
	PUThreadBase	base;
	PUThreadFunc	proxy;
	PCondVariable	*join_cond;
	struct Task	*task;
};

typedef pint puthread_key_t;

struct PUThreadKey_ {
	puthread_key_t	key;
	PDestroyFunc	free_func;
};

static PMutex *pp_uthread_glob_mutex = NULL;
static PList  *pp_uthread_list       = NULL;
static pint    pp_uthread_last_id    = 0;

static PUThreadTLSKey pp_uthread_tls_keys[PUTHREAD_AMIGA_MAX_TLS_KEYS];

static pint pp_uthread_get_amiga_priority (PUThreadPriority prio);
static puthread_key_t pp_uthread_get_tls_key (PUThreadKey *key);
static pint pp_uthread_find_next_id (void);
static PUThreadInfo * pp_uthread_find_thread_info (struct Task *task);
static PUThreadInfo * pp_uthread_find_or_create_thread_info (struct Task *task);
static pint pp_uthread_amiga_proxy (void);

static pint
pp_uthread_get_amiga_priority (PUThreadPriority prio)
{
	/* Priority limit is [-128, 127] */

	switch (prio) {
		case P_UTHREAD_PRIORITY_INHERIT:
			return 0;
		case P_UTHREAD_PRIORITY_IDLE:
			return -128;
		case P_UTHREAD_PRIORITY_LOWEST:
			return -50;
		case P_UTHREAD_PRIORITY_LOW:
			return -25;
		case P_UTHREAD_PRIORITY_NORMAL:
			return 0;
		case P_UTHREAD_PRIORITY_HIGH:
			return 25;
		case P_UTHREAD_PRIORITY_HIGHEST:
			return 50;
		case P_UTHREAD_PRIORITY_TIMECRITICAL:
			return 127;
		default:
			return 0;
	}
}

static puthread_key_t
pp_uthread_get_tls_key (PUThreadKey *key)
{
	puthread_key_t	thread_key;
	pint 		key_idx;

	thread_key = (puthread_key_t) p_atomic_int_get (&key->key);

	if (P_LIKELY (thread_key >= 0))
		return thread_key;

	p_mutex_lock (pp_uthread_glob_mutex);

	if (key->key >= 0) {
		p_mutex_unlock (pp_uthread_glob_mutex);
		return key->key;
	}

	/* Find free TLS key index */

	for (key_idx = 0; key_idx < PUTHREAD_AMIGA_MAX_TLS_KEYS; ++key_idx) {
		if (P_LIKELY (pp_uthread_tls_keys[key_idx].in_use == FALSE)) {
			pp_uthread_tls_keys[key_idx].in_use    = TRUE;
			pp_uthread_tls_keys[key_idx].free_func = key->free_func;

			break;
		}
	}

	if (key_idx == PUTHREAD_AMIGA_MAX_TLS_KEYS) {
		p_mutex_unlock (pp_uthread_glob_mutex);
		P_ERROR ("PUThread::pp_uthread_get_tls_key: all slots for TLS keys are used");
		return -1;
	}

	key->key = key_idx;

	p_mutex_unlock (pp_uthread_glob_mutex);

	return key_idx;
}

/* Must be used only inside a protected critical region */

static pint
pp_uthread_find_next_id (void)
{
	PList		*cur_list;
	PUThreadInfo	*thread_info;
	pboolean	have_dup;
	pboolean	was_found  = FALSE;
	pint		cur_id     = pp_uthread_last_id;
	pint		of_counter = 0;

	while (was_found == FALSE && of_counter < 2) {
		have_dup = FALSE;
		cur_id   = (cur_id == P_MAXINT32) ? 0 : cur_id + 1;

		if (cur_id == 0)
			++of_counter;

		for (cur_list = pp_uthread_list; cur_list != NULL; cur_list = cur_list->next) {
			thread_info = (PUThreadInfo *) cur_list->data;

			if (thread_info->id == cur_id) {
				have_dup = TRUE;
				break;
			}
		}

		if (have_dup == FALSE)
			was_found = TRUE;
	}

	if (P_UNLIKELY (of_counter == 2))
		return -1;

	pp_uthread_last_id = cur_id;

	return cur_id;
}

/* Must be used only inside a protected critical region */

static PUThreadInfo *
pp_uthread_find_thread_info (struct Task *task)
{
	PList		*cur_list;
	PUThreadInfo	*thread_info;

	for (cur_list = pp_uthread_list; cur_list != NULL; cur_list = cur_list->next) {
		thread_info = (PUThreadInfo *) cur_list->data;

		if (thread_info->task == task)
			return thread_info;
	}

	return NULL;
}

/* Must be used only inside a protected critical region */

static PUThreadInfo *
pp_uthread_find_or_create_thread_info (struct Task *task)
{
	PUThreadInfo	*thread_info;
	pint		task_id;

	thread_info  = pp_uthread_find_thread_info (task);

	if (thread_info == NULL) {
		/* Call is from a forein thread */

		task_id = pp_uthread_find_next_id ();

		if (P_UNLIKELY (task_id == -1)) {
			/* Beyond the limit of the number of threads */
			P_ERROR ("PUThread::pp_uthread_find_or_create_thread_info: no free thread slots left");
			return NULL;
		}

		if (P_UNLIKELY ((thread_info = p_malloc0 (sizeof (PUThreadInfo))) == NULL)) {
			P_ERROR ("PUThread::pp_uthread_find_or_create_thread_info: failed to allocate memory");
			return NULL;
		}

		thread_info->id   = task_id;
		thread_info->task = task;

		pp_uthread_list = p_list_append (pp_uthread_list, thread_info);
	}

	return thread_info;
}

static pint
pp_uthread_amiga_proxy (void)
{
	PUThread	*thread;
	PUThreadInfo	*thread_info;
	struct Task	*task;
	PDestroyFunc	dest_func;
	ppointer	dest_data;
	pboolean	need_pass;
	pint		i;
	pint		clean_counter;

	/* Wait for outer routine to finish data initialization */

	p_mutex_lock (pp_uthread_glob_mutex);

	task        = IExec->FindTask (NULL);
	thread      = (PUThread *) (task->tc_UserData);
	thread_info = pp_uthread_find_thread_info (task);

	p_mutex_unlock (pp_uthread_glob_mutex);

	IExec->SetTaskPri (task, pp_uthread_get_amiga_priority (thread->base.prio));

	if (!setjmp (thread_info->jmpbuf))
		thread->proxy (thread);

	/* Clean up TLS values */

	p_mutex_lock (pp_uthread_glob_mutex);

	need_pass     = TRUE;
	clean_counter = 0;

	while (need_pass && clean_counter < PUTHREAD_AMIGA_MAX_CLEANS) {
		need_pass = FALSE;

		for (i = 0; i < PUTHREAD_AMIGA_MAX_TLS_KEYS; ++i) {
			if (pp_uthread_tls_keys[i].in_use == TRUE) {
				dest_func = pp_uthread_tls_keys[i].free_func;
				dest_data = thread_info->tls_values[i];

				if (dest_func != NULL && dest_data != NULL) {
					/* Destructor may do some trick with TLS as well */
					thread_info->tls_values[i] = NULL;

					p_mutex_unlock (pp_uthread_glob_mutex);
					(dest_func) (dest_data);
					p_mutex_lock (pp_uthread_glob_mutex);

					need_pass = TRUE;
				}
			}
		}

		++clean_counter;
	}

	pp_uthread_list = p_list_remove (pp_uthread_list, thread_info);

	p_free (thread_info);

	p_mutex_unlock (pp_uthread_glob_mutex);

	/* Signal to possible waiter */

	p_cond_variable_broadcast (thread->join_cond);
}

void
p_uthread_init_internal (void)
{
	if (P_LIKELY (pp_uthread_glob_mutex == NULL)) {
		pp_uthread_glob_mutex = p_mutex_new ();
		pp_uthread_list       = NULL;
		pp_uthread_last_id    = 0;

		memset (pp_uthread_tls_keys, 0, sizeof (PUThreadTLSKey) * PUTHREAD_AMIGA_MAX_TLS_KEYS);
	}
}

void
p_uthread_shutdown_internal (void)
{
	PList		*cur_list;
	PUThreadInfo	*thread_info;
	PDestroyFunc	dest_func;
	ppointer	dest_data;
	pboolean	need_pass;
	pint		i;
	pint		clean_counter;

	/* Perform destructors */

	p_mutex_lock (pp_uthread_glob_mutex);

	need_pass     = TRUE;
	clean_counter = 0;

	while (need_pass && clean_counter < PUTHREAD_AMIGA_MAX_CLEANS) {
		need_pass = FALSE;

		for (i = 0; i < PUTHREAD_AMIGA_MAX_TLS_KEYS; ++i) {
			if (pp_uthread_tls_keys[i].in_use == FALSE)
				continue;

			dest_func = pp_uthread_tls_keys[i].free_func;

			if (dest_func == NULL)
				continue;

			for (cur_list = pp_uthread_list; cur_list != NULL; cur_list = cur_list->next) {
				thread_info = (PUThreadInfo *) cur_list->data;
				dest_data   = thread_info->tls_values[i];

				if (dest_data != NULL) {
					/* Destructor may do some trick with TLS as well */

					thread_info->tls_values[i] = NULL;

					p_mutex_unlock (pp_uthread_glob_mutex);
					(dest_func) (dest_data);
					p_mutex_lock (pp_uthread_glob_mutex);

					need_pass = TRUE;
				}
			}
		}
	}

	/* Clean the list */

	p_list_foreach (pp_uthread_list, (PFunc) p_free, NULL);
	p_list_free (pp_uthread_list);

	pp_uthread_list = NULL;

	p_mutex_unlock (pp_uthread_glob_mutex);

	if (P_LIKELY (pp_uthread_glob_mutex != NULL)) {
		p_mutex_free (pp_uthread_glob_mutex);
		pp_uthread_glob_mutex = NULL;
	}
}

void
p_uthread_win32_thread_detach (void)
{
}

void
p_uthread_free_internal (PUThread *thread)
{
	if (thread->join_cond != NULL)
		p_cond_variable_free (thread->join_cond);

	p_free (thread);
}

PUThread *
p_uthread_create_internal (PUThreadFunc		func,
			   pboolean		joinable,
			   PUThreadPriority	prio,
			   psize		stack_size)
{
	PUThread	*ret;
	PUThreadInfo	*thread_info;
	struct Task	*task;
	pint		task_id;

	if (P_UNLIKELY ((ret = p_malloc0 (sizeof (PUThread))) == NULL)) {
		P_ERROR ("PUThread::p_uthread_create_internal: failed to allocate memory");
		return NULL;
	}

	if (P_UNLIKELY ((ret->join_cond = p_cond_variable_new ()) == NULL)) {
		P_ERROR ("PUThread::p_uthread_create_internal: failed to allocate condvar");
		p_uthread_free_internal (ret);
		return NULL;
	}

	if (P_UNLIKELY ((thread_info = p_malloc0 (sizeof (PUThreadInfo))) == NULL)) {
		P_ERROR ("PUThread::p_uthread_create_internal: failed to allocate memory (2)");
		p_uthread_free_internal (ret);
		return NULL;
	}

	p_mutex_lock (pp_uthread_glob_mutex);

	task_id = pp_uthread_find_next_id ();

	if (P_UNLIKELY (task_id == -1)) {
		p_mutex_unlock (pp_uthread_glob_mutex);
		P_ERROR ("PUThread::p_uthread_create_internal: no free thread slots left");
		p_uthread_free_internal (ret);
		p_free (thread_info);
		return NULL;
	}

	ret->proxy         = func;
	ret->base.prio     = prio;
	ret->base.joinable = joinable;

	if (stack_size < PUTHREAD_AMIGA_MIN_STACK)
		stack_size = PUTHREAD_AMIGA_MIN_STACK;

	task = (struct Task *) IDOS->CreateNewProcTags (NP_Entry,     pp_uthread_amiga_proxy,
							NP_StackSize, stack_size,
							NP_UserData,  ret,
							NP_Child,     TRUE,
							TAG_END);

	if (P_UNLIKELY (task == NULL)) {
		p_mutex_unlock (pp_uthread_glob_mutex);
		P_ERROR ("PUThread::p_uthread_create_internal: CreateTaskTags() failed");
		p_uthread_free_internal (ret);
		p_free (thread_info);
		return NULL;
	}

	thread_info->task = task;
	thread_info->id   = task_id;

	pp_uthread_list = p_list_append (pp_uthread_list, thread_info);

	ret->task = task;

	p_mutex_unlock (pp_uthread_glob_mutex);

	return ret;
}

void
p_uthread_exit_internal (void)
{
	PUThreadInfo *thread_info;

	p_mutex_lock (pp_uthread_glob_mutex);

	thread_info = pp_uthread_find_thread_info (IExec->FindTask (NULL));

	p_mutex_unlock (pp_uthread_glob_mutex);

	if (P_UNLIKELY (thread_info == NULL)) {
		P_WARNING ("PUThread::p_uthread_exit_internal: trying to exit from foreign thread");
		return;
	}

	longjmp (thread_info->jmpbuf, 1);
}

void
p_uthread_wait_internal (PUThread *thread)
{
	PUThreadInfo *thread_info;

	p_mutex_lock (pp_uthread_glob_mutex);

	thread_info = pp_uthread_find_thread_info (thread->task);

	if (thread_info == NULL) {
		p_mutex_unlock (pp_uthread_glob_mutex);
		return;
	}

	p_cond_variable_wait (thread->join_cond, pp_uthread_glob_mutex);
	p_mutex_unlock (pp_uthread_glob_mutex);
}

void
p_uthread_set_name_internal (PUThread *thread)
{
	struct Task *task = thread->task;

	task->tc_Node.ln_Name = thread->base.name;
}

P_LIB_API void
p_uthread_yield (void)
{
	BYTE		old_prio;
	struct Task	*task;

	task = IExec->FindTask (NULL);

	old_prio = IExec->SetTaskPri (task, -10);
	IExec->SetTaskPri (task, old_prio);
}

P_LIB_API pboolean
p_uthread_set_priority (PUThread		*thread,
			PUThreadPriority	prio)
{
	if (P_UNLIKELY (thread == NULL))
		return FALSE;

	IExec->SetTaskPri (thread->task, pp_uthread_get_amiga_priority (prio));

	thread->base.prio = prio;
	return TRUE;
}

P_LIB_API P_HANDLE
p_uthread_current_id (void)
{
	PUThreadInfo *thread_info;
	
	p_mutex_lock (pp_uthread_glob_mutex);

	thread_info  = pp_uthread_find_or_create_thread_info (IExec->FindTask (NULL));

	p_mutex_unlock (pp_uthread_glob_mutex);

	if (P_UNLIKELY (thread_info == NULL))
		P_WARNING ("PUThread::p_uthread_current_id: failed to integrate foreign thread");

	return (thread_info == NULL) ? NULL : (P_HANDLE) ((psize) thread_info->id);
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
	PUThreadInfo	*thread_info;
	puthread_key_t	tls_key;
	ppointer	value = NULL;

	if (P_UNLIKELY (key == NULL))
		return NULL;

	if (P_UNLIKELY ((tls_key = pp_uthread_get_tls_key (key)) == -1))
		return NULL;

	p_mutex_lock (pp_uthread_glob_mutex);

	thread_info = pp_uthread_find_thread_info (IExec->FindTask (NULL));
	
	if (P_LIKELY (thread_info != NULL))
		value = thread_info->tls_values[tls_key];

	p_mutex_unlock (pp_uthread_glob_mutex);

	return value;
}

P_LIB_API void
p_uthread_set_local (PUThreadKey	*key,
		     ppointer		value)
{
	PUThreadInfo	*thread_info;
	puthread_key_t	tls_key;

	if (P_UNLIKELY (key == NULL))
		return;

	tls_key = pp_uthread_get_tls_key (key);

	if (P_LIKELY (tls_key != -1)) {
		p_mutex_lock (pp_uthread_glob_mutex);

		thread_info = pp_uthread_find_or_create_thread_info (IExec->FindTask (NULL));

		if (P_LIKELY (thread_info != NULL)) {
			if (P_LIKELY (pp_uthread_tls_keys[tls_key].in_use == TRUE))
				thread_info->tls_values[tls_key] = value;
		}

		p_mutex_unlock (pp_uthread_glob_mutex);
	}
}

P_LIB_API void
p_uthread_replace_local	(PUThreadKey	*key,
			 ppointer	value)
{
	PUThreadInfo	*thread_info;
	puthread_key_t	tls_key;
	ppointer	old_value;

	if (P_UNLIKELY (key == NULL))
		return;

	tls_key = pp_uthread_get_tls_key (key);

	if (P_UNLIKELY (tls_key == -1))
		return;

	p_mutex_lock (pp_uthread_glob_mutex);

	if (P_LIKELY (pp_uthread_tls_keys[tls_key].in_use == TRUE)) {
		thread_info = pp_uthread_find_or_create_thread_info (IExec->FindTask (NULL));

		if (P_LIKELY (thread_info != NULL)) {
			old_value = thread_info->tls_values[tls_key];

			if (old_value != NULL && key->free_func != NULL)
				key->free_func (old_value);

			thread_info->tls_values[tls_key] = value;
		}
	}

	p_mutex_unlock (pp_uthread_glob_mutex);
}
