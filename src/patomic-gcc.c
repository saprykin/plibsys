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

/*
 * This file was taken from GLib and adopted for using along with PLib.
 * See gatomic-gcc.c for other copyrights and details.
 */

#include "plib.h"

pint
p_atomic_int_exchange_and_add (volatile pint	*atomic,
			       pint		val)
{
	return __sync_fetch_and_add (atomic, val);
}

void
p_atomic_int_add (volatile pint	*atomic,
		  pint		val)
{
	__sync_fetch_and_add (atomic, val);
}

pboolean
p_atomic_int_compare_and_exchange (volatile pint	*atomic,
				   pint			oldval,
				   pint			newval)
{
	return __sync_bool_compare_and_swap (atomic, oldval, newval);
}

pboolean
p_atomic_pointer_compare_and_exchange (volatile ppointer	*atomic,
				       ppointer			oldval,
				       ppointer			newval)
{
	return __sync_bool_compare_and_swap (atomic, oldval, newval);
}

void
_p_atomic_thread_init (void)
{
}

void
_p_atomic_thread_shutdown (void)
{
}

pint
(p_atomic_int_get) (volatile pint *atomic)
{
	__sync_synchronize ();
	return *atomic;
}

void
(p_atomic_int_set) (volatile pint	*atomic,
		    pint		newval)
{
	*atomic = newval;
	__sync_synchronize ();
}

ppointer
(p_atomic_pointer_get) (volatile ppointer *atomic)
{
	__sync_synchronize ();
	return *atomic;
}

void
(p_atomic_pointer_set) (volatile ppointer	*atomic,
			ppointer		newval)
{
	*atomic = newval;
	__sync_synchronize ();
}

void
p_atomic_memory_barrier (void)
{
	__sync_synchronize ();
}

