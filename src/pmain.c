/* 
 * 12.01.2011
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

extern _p_atomic_thread_init		(void);
extern _p_atomic_thread_shutdown	(void);
extern _p_socket_init_once		(void);
extern _p_socket_close_once		(void);

static pboolean plib_inited = FALSE;

P_LIB_API void
p_lib_init (void)
{
	if (plib_inited)
		return;

	plib_inited = TRUE;
	
	_p_atomic_thread_init ();
	_p_socket_init_once ();
}

P_LIB_API void
p_lib_shutdown (void)
{
	if (!plib_inited)
		return;

	plib_inited = FALSE;

	_p_atomic_thread_shutdown ();
	_p_socket_close_once ();
}

