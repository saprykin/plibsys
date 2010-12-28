/* 
 * 21.10.2010
 * Copyright (C) 2010 Alexander Saprykin <xelfium@gmail.com>
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

/* TODO: conditional variables */
/* TODO: priorities */
/* TODO: once routines */
/* TODO: barriers */
/* TODO: _full version of create func */

#include "pmem.h"
#include "puthread.h"

#include <stdlib.h>
#include <time.h>
#include <string.h>

#include <winsock2.h>
#include <windows.h>

typedef HANDLE puthread_hdl;

struct _PUThread {
	puthread_hdl hdl;
	pboolean joinable;
};

P_LIB_API PUThread *
p_uthread_create (PUThreadFunc func, ppointer data, pboolean joinable)
{
	PUThread *ret;

	if (!func)
		return NULL;

	if ((ret = p_malloc0 (sizeof (PUThread))) == NULL) {
		P_ERROR ("PUThread: failed to allocate memory");
		return NULL;
	}

	if ((ret->hdl = CreateThread (NULL, 0, (LPTHREAD_START_ROUTINE) func, data, 0, NULL)) == NULL) {
		P_ERROR ("PUThread: failed to create thread");
		p_free (ret);
		return NULL;
	}

	ret->joinable = joinable;
	return ret;
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

	if (!thread || !thread->joinable)
		return -1;

	if ((WaitForSingleObject (thread->hdl, INFINITE)) != WAIT_OBJECT_0) {
		P_ERROR ("PUThread: failed to join thread");
		return -1;
	}

	if (!GetExitCodeThread (thread->hdl, &exit_code)) {
		P_ERROR ("PUThread: failed to get exit code");
		return -1;
	}

	return exit_code;
}

P_LIB_API void
p_uthread_free (PUThread *thread)
{
	if (!thread)
		return;

	CloseHandle (thread->hdl);

	p_free (thread);
}

P_LIB_API void
p_uthread_yield (void)
{
	Sleep (0);
}

