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

#include "pcondvariable.h"
#include "pspinlock.h"
#include "patomic.h"
#include "pmem.h"

#include <stdlib.h>
#include <string.h>

#include <kernel/OS.h>

typedef struct _PCondThread {
	thread_id		thread;
	struct _PCondThread	*next;
} PCondThread;

struct PCondVariable_ {
	PSpinLock 	*lock;
	PCondThread	*wait_head;
	pint		wait_count;
};

P_LIB_API PCondVariable *
p_cond_variable_new (void)
{
	PCondVariable *ret;

	if (P_UNLIKELY ((ret = p_malloc0 (sizeof (PCondVariable))) == NULL)) {
		P_ERROR ("PCondVariable::p_cond_variable_new: failed to allocate memory");
		return NULL;
	}

	if ((ret->lock = p_spinlock_new ()) == NULL) {
		P_ERROR ("PCondVariable::p_cond_variable_new: failed to initialize");
		p_free (ret);
		return NULL;
	}

	return ret;
}

P_LIB_API void
p_cond_variable_free (PCondVariable *cond)
{
	if (P_UNLIKELY (cond == NULL))
		return;

	if ((cond->wait_count > 0) || (cond->wait_head != NULL))
		P_WARNING ("PCondVariable::p_cond_variable_free: destroying while threads are waiting");

	p_spinlock_free (cond->lock);
	p_free (cond);
}

P_LIB_API pboolean
p_cond_variable_wait (PCondVariable	*cond,
		      PMutex		*mutex)
{
	PCondThread *wait_thread;

	if (P_UNLIKELY (cond == NULL || mutex == NULL))
		return FALSE;

	if ((wait_thread = p_malloc0 (sizeof (PCondThread))) == NULL) {
		P_ERROR ("PCondVariable::p_cond_variable_wait: failed to allocate memory");
		return FALSE;
	}

	wait_thread->thread = find_thread (NULL);
	wait_thread->next   = NULL;
	
	if (p_spinlock_lock (cond->lock) != TRUE) {
		P_ERROR ("PCondVariable::p_cond_variable_wait: failed to lock internal spinlock");
		return FALSE;
	}
	
	if (cond->wait_head != NULL)
		cond->wait_head->next = wait_thread;
	else
		cond->wait_head = wait_thread;

	p_atomic_int_inc ((volatile pint *) &cond->wait_count);
	
	if (p_spinlock_unlock (cond->lock) != TRUE) {
		P_ERROR ("PCondVariable::p_cond_variable_wait: failed to unlock internal spinlock");
		return FALSE;
	}

	if (p_mutex_unlock (mutex) != TRUE) {
		P_ERROR ("PCondVariable::p_cond_variable_wait: failed to unlock mutex");
		return FALSE;
	}

	suspend_thread (wait_thread->thread);

	if (p_mutex_lock (mutex) != TRUE) {
		P_ERROR ("PCondVariable::p_cond_variable_wait: failed to lock mutex");
		return FALSE;
	}

	return TRUE;
}

P_LIB_API pboolean
p_cond_variable_signal (PCondVariable *cond)
{
	PCondThread	*wait_thread;
	thread_info	thr_info;

	if (P_UNLIKELY (cond == NULL))
		return FALSE;

	if (p_spinlock_lock (cond->lock) != TRUE) {
		P_ERROR ("PCondVariable::p_cond_variable_signal: failed to lock internal spinlock");
		return FALSE;
	}

	if (cond->wait_head == NULL) {
		if (p_spinlock_unlock (cond->lock) != TRUE) {
			P_ERROR ("PCondVariable::p_cond_variable_signal(1): failed to unlock internal spinlock");
			return FALSE;
		} else
			return TRUE;
	}

	wait_thread = cond->wait_head;
	cond->wait_head = wait_thread->next;

	p_atomic_int_add ((volatile pint *) &cond->wait_count, -1);

	if (p_spinlock_unlock (cond->lock) != TRUE) {
		P_ERROR ("PCondVariable::p_cond_variable_signal(2): failed to unlock internal spinlock");
		return FALSE;
	}

	memset (&thr_info, 0, sizeof (thr_info));
	
	while (get_thread_info (wait_thread->thread, &thr_info) == B_OK) {
		if (thr_info.state != B_THREAD_READY)
			break;
	}

	resume_thread (wait_thread->thread);

	p_free (wait_thread);
	return TRUE;
}

P_LIB_API pboolean
p_cond_variable_broadcast (PCondVariable *cond)
{
	PCondThread	*cur_thread;
	PCondThread	*next_thread;
	thread_info	thr_info;
	
	if (P_UNLIKELY (cond == NULL))
		return FALSE;

	if (p_spinlock_lock (cond->lock) != TRUE) {
		P_ERROR ("PCondVariable::p_cond_variable_broadcast: failed to lock internal spinlock");
		return FALSE;
	}

	if (cond->wait_head == NULL) {
		if (p_spinlock_unlock (cond->lock) != TRUE) {
			P_ERROR ("PCondVariable::p_cond_variable_broadcast(1): failed to unlock internal spinlock");
			return FALSE;
		} else
			return TRUE;
	}

	cur_thread = cond->wait_head;

	do {
		memset (&thr_info, 0, sizeof (thr_info)); 

		while (get_thread_info (cur_thread->thread, &thr_info) == B_OK) {
			if (thr_info.state != B_THREAD_READY)
				break;
		}

		resume_thread (cur_thread->thread);

		next_thread = cur_thread->next;
		p_free (cur_thread);

		cur_thread = next_thread;
	} while (cur_thread != NULL);

	cond->wait_head  = NULL;
	cond->wait_count = 0;

	if (p_spinlock_unlock (cond->lock) != TRUE) {
		P_ERROR ("PCondVariable::p_cond_variable_broadcast(2): failed to unlock internal spinlock");
		return FALSE;
	}

	return TRUE;
}

void
p_cond_variable_init (void)
{
}

void
p_cond_variable_shutdown (void)
{
}
