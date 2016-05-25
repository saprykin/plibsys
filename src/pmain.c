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
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

#include "pmem.h"
#include "pmain.h"

extern void __p_mem_init		(void);
extern void __p_mem_shutdown		(void);
extern void __p_atomic_thread_init	(void);
extern void __p_atomic_thread_shutdown	(void);
extern void __p_socket_init_once	(void);
extern void __p_socket_close_once	(void);
extern void __p_uthread_init		(void);
extern void __p_uthread_shutdown	(void);

static pboolean plibsys_inited = FALSE;

P_LIB_API void
p_libsys_init (void)
{
	if (P_UNLIKELY (plibsys_inited == TRUE))
		return;

	plibsys_inited = TRUE;

	__p_mem_init ();
	__p_atomic_thread_init ();
	__p_socket_init_once ();
	__p_uthread_init ();
}

P_LIB_API void
p_libsys_init_full (const PMemVTable *vtable)
{
	p_mem_set_vtable (vtable);
	p_libsys_init ();
}

P_LIB_API void
p_libsys_shutdown (void)
{
	if (P_UNLIKELY (plibsys_inited == FALSE))
		return;

	plibsys_inited = FALSE;

	__p_uthread_shutdown ();
	__p_socket_close_once ();
	__p_atomic_thread_shutdown ();
	__p_mem_shutdown ();
}

#ifdef P_OS_WIN
extern void __p_uthread_win32_thread_detach (void);

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
		__p_uthread_win32_thread_detach ();
		break;

	case DLL_PROCESS_DETACH:
		break;

	default:
		;
	}

	return TRUE;
}
#endif
