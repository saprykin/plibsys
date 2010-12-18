/* 
 * 05.10.2010
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

#include "pprocess.h"

#ifdef P_OS_WIN
#include <winsock2.h>
#include <windows.h>
#else
#include <sys/types.h>
#include <signal.h>
#include <unistd.h>
#endif

P_LIB_API puint32
p_process_get_current_pid (void)
{
#ifdef P_OS_WIN
	return GetCurrentProcessId ();
#else
	return getpid ();
#endif
}

P_LIB_API pboolean
p_process_is_running (puint32 pid)
{
#ifdef P_OS_WIN
	HANDLE proc;
	DWORD ret;

	if ((proc = OpenProcess (SYNCHRONIZE, FALSE, pid)) == NULL)
		return FALSE;
	
	ret = WaitForSingleObject (proc, 0);
	CloseHandle (proc);

	return ret == WAIT_TIMEOUT;
#else
	return kill (pid, 0) == 0;
#endif
}

