/*
 * Copyright (C) 2010-2016 Alexander Saprykin <xelfium@gmail.com>
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
