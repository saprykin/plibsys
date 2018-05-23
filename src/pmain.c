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
#include "pmain.h"

extern void p_mem_init			(void);
extern void p_mem_shutdown		(void);
extern void p_atomic_thread_init	(void);
extern void p_atomic_thread_shutdown	(void);
extern void p_socket_init_once		(void);
extern void p_socket_close_once		(void);
extern void p_uthread_init		(void);
extern void p_uthread_shutdown		(void);
extern void p_cond_variable_init	(void);
extern void p_cond_variable_shutdown	(void);
extern void p_rwlock_init		(void);
extern void p_rwlock_shutdown		(void);
extern void p_time_profiler_init	(void);
extern void p_time_profiler_shutdown	(void);
extern void p_library_loader_init	(void);
extern void p_library_loader_shutdown	(void);

static pboolean pp_plibsys_inited = FALSE;
static pchar pp_plibsys_version[] = PLIBSYS_VERSION_STR;

P_LIB_API void
p_libsys_init (void)
{
	if (P_UNLIKELY (pp_plibsys_inited == TRUE))
		return;

	pp_plibsys_inited = TRUE;

	p_mem_init ();
	p_atomic_thread_init ();
	p_socket_init_once ();
	p_uthread_init ();
	p_cond_variable_init ();
	p_rwlock_init ();
	p_time_profiler_init ();
	p_library_loader_init ();
}

P_LIB_API void
p_libsys_init_full (const PMemVTable *vtable)
{
	if (p_mem_set_vtable (vtable) == FALSE)
		P_ERROR ("MAIN::p_libsys_init_full: failed to initialize memory table");

	p_libsys_init ();
}

P_LIB_API void
p_libsys_shutdown (void)
{
	if (P_UNLIKELY (pp_plibsys_inited == FALSE))
		return;

	pp_plibsys_inited = FALSE;

	p_library_loader_init ();
	p_time_profiler_shutdown ();
	p_rwlock_shutdown ();
	p_cond_variable_shutdown ();
	p_uthread_shutdown ();
	p_socket_close_once ();
	p_atomic_thread_shutdown ();
	p_mem_shutdown ();
}

P_LIB_API const pchar *
p_libsys_version (void)
{
	return (const pchar *) pp_plibsys_version;
}

#ifdef P_OS_WIN
extern void p_uthread_win32_thread_detach (void);

BOOL WINAPI DllMain (HINSTANCE	hinstDLL,
		     DWORD	fdwReason,
		     LPVOID	lpvReserved);

BOOL WINAPI
DllMain (HINSTANCE	hinstDLL,
	 DWORD		fdwReason,
	 LPVOID		lpvReserved)
{
	P_UNUSED (hinstDLL);
	P_UNUSED (lpvReserved);

	switch (fdwReason) {
	case DLL_PROCESS_ATTACH:
		break;

	case DLL_THREAD_DETACH:
		p_uthread_win32_thread_detach ();
		break;

	case DLL_PROCESS_DETACH:
		break;

	default:
		;
	}

	return TRUE;
}
#endif
