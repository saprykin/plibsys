/*
 * Copyright (C) 2016 Alexander Saprykin <xelfium@gmail.com>
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

#include "patomic.h"

P_LIB_API pint
p_atomic_int_get (const volatile pint *atomic)
{
	return (pint) __atomic_load_4 (atomic, __ATOMIC_SEQ_CST);
}

P_LIB_API void
p_atomic_int_set (volatile pint	*atomic,
		  pint		val)
{
	__atomic_store_4 (atomic, val, __ATOMIC_SEQ_CST);
}

P_LIB_API void
p_atomic_int_inc (volatile pint *atomic)
{
	(void) __atomic_fetch_add (atomic, 1, __ATOMIC_SEQ_CST);
}

P_LIB_API pboolean
p_atomic_int_dec_and_test (volatile pint *atomic)
{
	return (__atomic_fetch_sub (atomic, 1, __ATOMIC_SEQ_CST) == 1) ? TRUE : FALSE;
}

P_LIB_API pboolean
p_atomic_int_compare_and_exchange (volatile pint	*atomic,
				   pint			oldval,
				   pint			newval)
{
	pint tmp_int = oldval;
	
	return (pboolean) __atomic_compare_exchange_n (atomic,
						       &tmp_int,
						       newval,
						       0,
						       __ATOMIC_SEQ_CST,
						       __ATOMIC_SEQ_CST);
}

P_LIB_API pint
p_atomic_int_add (volatile pint	*atomic,
		  pint		val)
{
	return (pint) __atomic_fetch_add (atomic, val, __ATOMIC_SEQ_CST);
}

P_LIB_API puint
p_atomic_int_and (volatile puint	*atomic,
		  puint			val)
{
	return (puint) __atomic_fetch_and (atomic, val, __ATOMIC_SEQ_CST);
}

P_LIB_API puint
p_atomic_int_or (volatile puint	*atomic,
		 puint		val)
{
	return (puint) __atomic_fetch_or (atomic, val, __ATOMIC_SEQ_CST);
}

P_LIB_API puint
p_atomic_int_xor (volatile puint	*atomic,
		  puint			val)
{
	return (puint) __atomic_fetch_xor (atomic, val, __ATOMIC_SEQ_CST);
}

P_LIB_API ppointer
p_atomic_pointer_get (const volatile void *atomic)
{
#if (PLIB_SIZEOF_VOID_P == 8)
	return (ppointer) __atomic_load_8 ((const volatile psize *) atomic, __ATOMIC_SEQ_CST);
#else
	return (ppointer) __atomic_load_4 ((const volatile psize *) atomic, __ATOMIC_SEQ_CST);
#endif
}

P_LIB_API void
p_atomic_pointer_set (volatile void	*atomic,
		      ppointer		val)
{
#if (PLIB_SIZEOF_VOID_P == 8)
	__atomic_store_8 (atomic, (psize) val, __ATOMIC_SEQ_CST);
#else
	__atomic_store_4 (atomic, (psize) val, __ATOMIC_SEQ_CST);
#endif
}

P_LIB_API pboolean
p_atomic_pointer_compare_and_exchange (volatile void	*atomic,
				       ppointer		oldval,
				       ppointer		newval)
{
	ppointer tmp_pointer = oldval;

	return (pboolean) __atomic_compare_exchange_n ((volatile psize *) atomic,
						       (psize *) &tmp_pointer,
						       newval,
						       0,
						       __ATOMIC_SEQ_CST,
						       __ATOMIC_SEQ_CST);
}

P_LIB_API pssize
p_atomic_pointer_add (volatile void	*atomic,
		      pssize		val)
{
	return (pssize) __atomic_fetch_add ((volatile pssize *) atomic, val, __ATOMIC_SEQ_CST);
}

P_LIB_API psize
p_atomic_pointer_and (volatile void	*atomic,
		      psize		val)
{
	return (psize) __atomic_fetch_and ((volatile psize *) atomic, val, __ATOMIC_SEQ_CST);
}

P_LIB_API psize
p_atomic_pointer_or (volatile void	*atomic,
		     psize		val)
{
	return (psize) __atomic_fetch_or ((volatile pssize *) atomic, val, __ATOMIC_SEQ_CST);
}

P_LIB_API psize
p_atomic_pointer_xor (volatile void	*atomic,
		      psize		val)
{
	return (psize) __atomic_fetch_xor ((volatile pssize *) atomic, val, __ATOMIC_SEQ_CST);
}

P_LIB_API pboolean
p_atomic_is_lock_free (void)
{
	return TRUE;
}

void
__p_atomic_thread_init (void)
{
}

void
__p_atomic_thread_shutdown (void)
{
}
