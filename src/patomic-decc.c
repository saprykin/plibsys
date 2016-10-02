/*
 * Copyright (C) 2016 Alexander Saprykin <xelfium@gmail.com>
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

#include "patomic.h"

#include <builtins.h>

P_LIB_API pint
p_atomic_int_get (const volatile pint *atomic)
{
	__MB ();
	return (pint) *atomic;
}

P_LIB_API void
p_atomic_int_set (volatile pint	*atomic,
		  pint		val)
{
	(void) __ATOMIC_EXCH_LONG ((volatile void *) atomic, val);
}

P_LIB_API void
p_atomic_int_inc (volatile pint *atomic)
{
	(void) __ATOMIC_INCREMENT_LONG ((volatile void *) atomic);
}

P_LIB_API pboolean
p_atomic_int_dec_and_test (volatile pint *atomic)
{
	return (__ATOMIC_DECREMENT_LONG ((volatile void *) atomic) == 1) ? TRUE : FALSE;
}

P_LIB_API pboolean
p_atomic_int_compare_and_exchange (volatile pint	*atomic,
				   pint			oldval,
				   pint			newval)
{
	return __CMP_STORE_LONG ((volatile void *) atomic,
				 oldval,
				 newval,
				 (volatile void *) atomic) == 1 ? TRUE : FALSE;
}

P_LIB_API pint
p_atomic_int_add (volatile pint	*atomic,
		  pint		val)
{
	return __ATOMIC_ADD_LONG ((volatile void *) atomic, val);
}

P_LIB_API puint
p_atomic_int_and (volatile puint	*atomic,
		  puint			val)
{
	return (puint) __ATOMIC_AND_LONG ((volatile void *) atomic, (pint) val);
}

P_LIB_API puint
p_atomic_int_or (volatile puint	*atomic,
		 puint		val)
{

	return (puint) __ATOMIC_OR_LONG ((volatile void *) atomic, (pint) val);
}

P_LIB_API puint
p_atomic_int_xor (volatile puint	*atomic,
		  puint			val)
{
	pint i;

	do {
		i = (pint) (*atomic);
	} while (__CMP_STORE_LONG ((volatile void *) atomic,
				   i,
				   i ^ ((pint) val),
				   (volatile void *) atomic) != 1);

	return i;
}

P_LIB_API ppointer
p_atomic_pointer_get (const volatile void *atomic)
{
	__MB ();
	return (ppointer) (* ((const volatile psize *) atomic));
}

P_LIB_API void
p_atomic_pointer_set (volatile void	*atomic,
		      ppointer		val)
{
#if (PLIBSYS_SIZEOF_VOID_P == 8)
	(void) __ATOMIC_EXCH_QUAD (atomic, (pint64) val);
#else
	(void) __ATOMIC_EXCH_LONG (atomic, (pint) val);
#endif
}

P_LIB_API pboolean
p_atomic_pointer_compare_and_exchange (volatile void	*atomic,
				       ppointer		oldval,
				       ppointer		newval)
{
#if (PLIBSYS_SIZEOF_VOID_P == 8)
	return __CMP_STORE_QUAD (atomic,
				 (pint64) oldval,
				 (pint64) newval,
				 atomic) == 1 ? TRUE : FALSE;
#else
	return __CMP_STORE_LONG (atomic,
				 (pint) oldval,
				 (pint) newval,
				 atomic) == 1 ? TRUE : FALSE;
#endif
}

P_LIB_API pssize
p_atomic_pointer_add (volatile void	*atomic,
		      pssize		val)
{
#if (PLIBSYS_SIZEOF_VOID_P == 8)
	return (pssize) __ATOMIC_ADD_QUAD (atomic, (pint64) val);
#else
	return (pssize) __ATOMIC_ADD_LONG (atomic, (pint) val);
#endif
}

P_LIB_API psize
p_atomic_pointer_and (volatile void	*atomic,
		      psize		val)
{
#if (PLIBSYS_SIZEOF_VOID_P == 8)
	return (psize) __ATOMIC_AND_QUAD (atomic, (pint64) val);
#else
	return (psize) __ATOMIC_AND_LONG (atomic, (pint) val);
#endif
}

P_LIB_API psize
p_atomic_pointer_or (volatile void	*atomic,
		     psize		val)
{
#if (PLIBSYS_SIZEOF_VOID_P == 8)
	return (psize) __ATOMIC_OR_QUAD (atomic, (pint64) val);
#else
	return (psize) __ATOMIC_OR_LONG (atomic, (pint) val);
#endif
}

P_LIB_API psize
p_atomic_pointer_xor (volatile void	*atomic,
		      psize		val)
{
#if (PLIBSYS_SIZEOF_VOID_P == 8)
	pint64 i;

	do {
		i = (pint64) (* ((volatile psize *) atomic));
	} while (__CMP_STORE_QUAD (atomic,
				   i,
				   i ^ ((pint64) val),
				   atomic) != 1);
#else
	pint i;

	do {
		i = (pint) (* ((volatile psize *) atomic));
	} while (__CMP_STORE_LONG (atomic,
				   i,
				   i ^ ((pint) val),
				   atomic) != 1);
#endif

	return (psize) i;
}

P_LIB_API pboolean
p_atomic_is_lock_free (void)
{
	return TRUE;
}

void
p_atomic_thread_init (void)
{
}

void
p_atomic_thread_shutdown (void)
{
}
