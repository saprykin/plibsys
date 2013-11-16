/* 
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

/* Hash table organized like this: table[hash key]->[list with values]
 * Note: this implementation is not intended to use on huge loads */

#include "pmem.h"
#include "pmain.h"

extern void __p_atomic_thread_init	(void);
extern void __p_atomic_thread_shutdown	(void);
extern void __p_socket_init_once	(void);
extern void __p_socket_close_once	(void);
extern void __p_uthread_init		(void);
extern void __p_uthread_shutdown	(void);

#ifdef P_OS_WIN
extern void _p_mutex_win_init		(void);
extern void _p_mutex_win_shutdown	(void);
#endif

static pboolean plib_inited = FALSE;

P_LIB_API void
p_lib_init (void)
{
	if (plib_inited)
		return;

	plib_inited = TRUE;
	
	__p_atomic_thread_init ();
	__p_socket_init_once ();
	__p_uthread_init ();
	
#ifdef P_OS_WIN
	_p_mutex_win_init ();
#endif
}

P_LIB_API void
p_lib_shutdown (void)
{
	if (!plib_inited)
		return;

	plib_inited = FALSE;

	__p_atomic_thread_shutdown ();
	__p_socket_close_once ();
	__p_uthread_shutdown ();
	
#ifdef P_OS_WIN
	_p_mutex_win_shutdown ();
#endif
}
