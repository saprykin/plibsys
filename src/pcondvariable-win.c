/* 
 * Copyright (C) 2010-2013 Alexander Saprykin <xelfium@gmail.com>
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
 */

#include "pmem.h"
#include "pcondvariable.h"

#include <stdlib.h>
#include <time.h>
#include <string.h>

#include <winsock2.h>
#include <windows.h>

struct _PCondVariable {
	CRITICAL_SECTION	waiters_count_lock;
	HANDLE			waiters_sema;
	HANDLE			waiters_done;
	pboolean		was_broadcast;
	pint			waiters_count;
};

P_LIB_API PCondVariable *
p_cond_variable_new (void)
{
	PCondVariable *ret;

	if ((ret = p_malloc0 (sizeof (PCondVariable))) == NULL) {
		P_ERROR ("PCondVariable: failed to allocate memory");
		return NULL;
	}

	ret->waiters_count	= 0;
	ret->was_broadcast	= FALSE;
	ret->waiters_sema	= CreateSemaphore (NULL, 0, MAXLONG, NULL);
	
	InitializeCriticalSection (&ret->waiters_count_lock);

	ret->waiters_done = CreateEvent (NULL, FALSE, FALSE, NULL);

	return ret;
}

P_LIB_API void
p_cond_variable_free (PCondVariable *cond)
{
	if (cond == NULL)
		return;

	DeleteCriticalSection (&cond->waiters_count_lock);
	CloseHandle (cond->waiters_sema);
	CloseHandle (cond->waiters_done);

	p_free (cond);
}

P_LIB_API pint
p_cond_variable_wait (PCondVariable	*cond,
		      PMutex		*mutex)
{
	pboolean	last_waiter;

	if (cond == NULL || mutex == NULL)
		return -1;
	
	EnterCriticalSection (&cond->waiters_count_lock);
	cond->waiters_count++;
	LeaveCriticalSection (&cond->waiters_count_lock);

	/* We can always cast PMutex to HANDLE since structure has only one HANDLE field */
	SignalObjectAndWait (*((HANDLE *) mutex), cond->waiters_sema, INFINITE, FALSE);
	
	EnterCriticalSection (&cond->waiters_count_lock);

	cond->waiters_count--;

	last_waiter = (cond->was_broadcast == TRUE && cond->waiters_count == 0) ? TRUE : FALSE;

	LeaveCriticalSection (&cond->waiters_count_lock);

	if (last_waiter)
		SignalObjectAndWait (cond->waiters_done, *((HANDLE *) mutex), INFINITE, FALSE);
	else
		WaitForSingleObject (*((HANDLE *) mutex), INFINITE);

	return 0;
}

P_LIB_API pint
p_cond_variable_signal (PCondVariable *cond)
{
	pboolean have_waiters;

	if (cond == NULL)
		return -1;

	EnterCriticalSection (&cond->waiters_count_lock);
  	have_waiters = (cond->waiters_count > 0) ? TRUE : FALSE;
	LeaveCriticalSection (&cond->waiters_count_lock);

	if (have_waiters)
		ReleaseSemaphore (cond->waiters_sema, 1, 0);

	return 0;
}

P_LIB_API pint
p_cond_variable_broadcast (PCondVariable *cond)
{
	pboolean have_waiters;

	if (cond == NULL)
		return -1;

	EnterCriticalSection (&cond->waiters_count_lock);
	have_waiters = FALSE;

	if (cond->waiters_count > 0) {
		cond->was_broadcast = TRUE;
		have_waiters = TRUE;
	}

	if (have_waiters) {
		ReleaseSemaphore (cond->waiters_sema, cond->waiters_count, 0);
		LeaveCriticalSection (&cond->waiters_count_lock);
		WaitForSingleObject (cond->waiters_done, INFINITE);
		cond->was_broadcast = FALSE;
	} else
		LeaveCriticalSection (&cond->waiters_count_lock);

	return 0;
}
