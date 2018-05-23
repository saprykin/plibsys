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

#include "pprocess.h"

#ifndef P_OS_WIN
#  include <sys/types.h>
#  include <signal.h>
#  include <unistd.h>
#endif

P_LIB_API puint32
p_process_get_current_pid (void)
{
#ifdef P_OS_WIN
	return (puint32) GetCurrentProcessId ();
#else
	return (puint32) getpid ();
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
	return kill ((pid_t) pid, 0) == 0;
#endif
}
