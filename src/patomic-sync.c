/*
 * Copyright (C) 2010-2017 Alexander Saprykin <xelfium@gmail.com>
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

#ifdef P_CC_CRAY
#  include <intrinsics.h>
#endif

P_LIB_API pint
p_atomic_int_get (const volatile pint *atomic)
{
#ifdef P_CC_CRAY
	__builtin_ia32_mfence ();
#else
	__sync_synchronize ();
#endif
	return *atomic;
}

P_LIB_API void
p_atomic_int_set (volatile pint	*atomic,
		  pint		val)
{
	*atomic = val;
#ifdef P_CC_CRAY
	__builtin_ia32_mfence ();
#else
	__sync_synchronize ();
#endif
}

P_LIB_API void
p_atomic_int_inc (volatile pint *atomic)
{
	(void) __sync_fetch_and_add (atomic, 1);
}

P_LIB_API pboolean
p_atomic_int_dec_and_test (volatile pint *atomic)
{
	return __sync_fetch_and_sub (atomic, 1) == 1 ? TRUE : FALSE;
}

P_LIB_API pboolean
p_atomic_int_compare_and_exchange (volatile pint	*atomic,
				   pint			oldval,
				   pint			newval)
{
#ifdef P_CC_CRAY
	return __sync_val_compare_and_swap (atomic, oldval, newval) == oldval ? TRUE : FALSE;
#else
	return (pboolean) __sync_bool_compare_and_swap (atomic, oldval, newval);
#endif
}

P_LIB_API pint
p_atomic_int_add (volatile pint	*atomic,
		  pint		val)
{
	return (pint) __sync_fetch_and_add (atomic, val);
}

P_LIB_API puint
p_atomic_int_and (volatile puint	*atomic,
		  puint			val)
{
	return (puint) __sync_fetch_and_and (atomic, val);
}

P_LIB_API puint
p_atomic_int_or (volatile puint	*atomic,
		 puint		val)
{
	return (puint) __sync_fetch_and_or (atomic, val);
}

P_LIB_API puint
p_atomic_int_xor (volatile puint	*atomic,
		  puint			val)
{
	return (puint) __sync_fetch_and_xor (atomic, val);
}

P_LIB_API ppointer
p_atomic_pointer_get (const volatile void *atomic)
{
#ifdef P_CC_CRAY
	__builtin_ia32_mfence ();
#else
	__sync_synchronize ();
#endif
	return (ppointer) *((const volatile psize *) atomic);
}

P_LIB_API void
p_atomic_pointer_set (volatile void	*atomic,
		      ppointer		val)
{
	volatile psize *cur_val = (volatile psize *) atomic;

	*cur_val = (psize) val;
#ifdef P_CC_CRAY
	__builtin_ia32_mfence ();
#else
	__sync_synchronize ();
#endif
}

P_LIB_API pboolean
p_atomic_pointer_compare_and_exchange (volatile void	*atomic,
				       ppointer		oldval,
				       ppointer		newval)
{
#ifdef P_CC_CRAY
	return __sync_val_compare_and_swap ((volatile psize *) atomic,
					    (psize) oldval,
					    (psize) newval) == ((psize) oldval) ? TRUE : FALSE;
#else
	return (pboolean) __sync_bool_compare_and_swap ((volatile psize *) atomic,
							(psize) oldval,
							(psize) newval);
#endif
}

P_LIB_API pssize
p_atomic_pointer_add (volatile void	*atomic,
		      pssize		val)
{
	return (pssize) __sync_fetch_and_add ((volatile pssize *) atomic, val);
}

P_LIB_API psize
p_atomic_pointer_and (volatile void	*atomic,
		      psize		val)
{
	return (psize) __sync_fetch_and_and ((volatile psize *) atomic, val);
}

P_LIB_API psize
p_atomic_pointer_or (volatile void	*atomic,
		     psize		val)
{
	return (psize) __sync_fetch_and_or ((volatile psize *) atomic, val);
}

P_LIB_API psize
p_atomic_pointer_xor (volatile void	*atomic,
		      psize		val)
{
	return (psize) __sync_fetch_and_xor ((volatile psize *) atomic, val);
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
