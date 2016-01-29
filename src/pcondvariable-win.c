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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA.
 */

/* Taken from "Strategies for Implementing POSIX Condition Variables on Win32"
 * by Douglas C. Schmidt and Irfan Pyarali.
 * See: http://www.cse.wustl.edu/~schmidt/win32-cv-1.html
 * See: https://github.com/python/cpython/blob/master/Python/condvar.h
 */

#include "pmem.h"
#include "pcondvariable.h"

#include <stdlib.h>

#include <windows.h>

struct _PCondVariable {
#ifdef PLIB_HAS_VISTA_CV
	CONDITION_VARIABLE	cv;
#else
	HANDLE			waiters_sema;
	pint			waiters_count;
#endif
};

P_LIB_API PCondVariable *
p_cond_variable_new (void)
{
	PCondVariable *ret;

	if ((ret = p_malloc0 (sizeof (PCondVariable))) == NULL) {
		P_ERROR ("PCondVariable: failed to allocate memory");
		return NULL;
	}

#ifdef PLIB_HAS_VISTA_CV
	InitializeConditionVariable (&ret->cv);
#else
	ret->waiters_count	= 0;
	ret->waiters_sema	= CreateSemaphore (NULL, 0, MAXLONG, NULL);

	if (ret->waiters_sema == NULL) {
		P_ERROR ("PCondVariable: failed to initialize semaphore");
		p_free (ret);
		return NULL;
	}
#endif

	return ret;
}

P_LIB_API void
p_cond_variable_free (PCondVariable *cond)
{
	if (cond == NULL)
		return;

#ifndef PLIB_HAS_VISTA_CV
	CloseHandle (cond->waiters_sema);
#endif

	p_free (cond);
}

P_LIB_API pboolean
p_cond_variable_wait (PCondVariable	*cond,
		      PMutex		*mutex)
{
#ifndef PLIB_HAS_VISTA_CV
	DWORD wait;
#endif

	if (cond == NULL || mutex == NULL)
		return FALSE;

#ifdef PLIB_HAS_VISTA_CV
	return SleepConditionVariableCS (&cond->cv, (PCRITICAL_SECTION) mutex, INFINITE) != 0 ? TRUE : FALSE;
#else
	cond->waiters_count++;

	p_mutex_unlock (mutex);
	wait = WaitForSingleObjectEx (cond->waiters_sema, INFINITE, FALSE);
	p_mutex_lock (mutex);

	if (wait != WAIT_OBJECT_0)
		--cond->waiters_count;

	if (wait == WAIT_FAILED)
		return FALSE;

	return wait == WAIT_OBJECT_0 ? TRUE : FALSE;
#endif
}

P_LIB_API pboolean
p_cond_variable_signal (PCondVariable *cond)
{
	if (cond == NULL)
		return FALSE;

#ifdef PLIB_HAS_VISTA_CV
	WakeConditionVariable (&cond->cv);
#else
	if (cond->waiters_count > 0) {
		cond->waiters_count--;
		return ReleaseSemaphore (cond->waiters_sema, 1, 0) != 0 ? TRUE : FALSE;
	}
#endif

	return TRUE;
}

P_LIB_API pboolean
p_cond_variable_broadcast (PCondVariable *cond)
{
#ifndef PLIB_HAS_VISTA_CV
	pint waiters;
#endif

	if (cond == NULL)
		return FALSE;

#ifdef PLIB_HAS_VISTA_CV
	WakeAllConditionVariable (&cond->cv);
#else
	waiters = cond->waiters_count;

	if (waiters > 0) {
		cond->waiters_count = 0;
		return ReleaseSemaphore (cond->waiters_sema, waiters, 0) != 0 ? TRUE : FALSE;
	}
#endif

	return TRUE;
}
