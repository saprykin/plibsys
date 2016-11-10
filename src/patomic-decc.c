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

#ifdef P_OS_VMS
#  include <builtins.h>
#else
#  include <machine/builtins.h>
#endif

#ifdef __ia64
#  define PATOMIC_DECC_CAS_LONG(atomic_src, oldval, newval, atomic_dst)	\
	  __CMP_SWAP_LONG ((volatile ppointer) (atomic_src),		\
			   (pint) (oldval),				\
			   (pint) (newval))
#  define PATOMIC_DECC_CAS_QUAD(atomic_src, oldval, newval, atomic_dst)	\
	  __CMP_SWAP_QUAD ((volatile ppointer) (atomic_src),		\
			   (pint64) (oldval),				\
			   (pint64) (newval))	  
#else
#  define PATOMIC_DECC_CAS_LONG(atomic_src, oldval, newval, atomic_dst)	\
	  __CMP_STORE_LONG ((volatile ppointer) (atomic_src),		\
			    (pint) (oldval),				\
			    (pint) (newval),				\
			    (volatile ppointer) (atomic_dst))
#  define PATOMIC_DECC_CAS_QUAD(atomic_src, oldval, newval, atomic_dst)	\
	  __CMP_STORE_QUAD ((volatile ppointer) (atomic_src),		\
			    (pint64) (oldval),				\
			    (pint64) (newval),				\
			    (volatile ppointer) (atomic_dst))
#endif

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
	(void) __ATOMIC_EXCH_LONG ((volatile ppointer) atomic, val);
	__MB ();
}

P_LIB_API void
p_atomic_int_inc (volatile pint *atomic)
{
	__MB ();
	(void) __ATOMIC_INCREMENT_LONG ((volatile ppointer) atomic);
	__MB ();
}

P_LIB_API pboolean
p_atomic_int_dec_and_test (volatile pint *atomic)
{
	pboolean result;

	__MB ();
	result = __ATOMIC_DECREMENT_LONG ((volatile ppointer) atomic) == 1 ? TRUE : FALSE;
	__MB ();

	return result;
}

P_LIB_API pboolean
p_atomic_int_compare_and_exchange (volatile pint	*atomic,
				   pint			oldval,
				   pint			newval)
{
	pboolean result;

	__MB ();
	result = PATOMIC_DECC_CAS_LONG (atomic, oldval, newval, atomic) == 1 ? TRUE : FALSE;
	__MB ();

	return result;
}

P_LIB_API pint
p_atomic_int_add (volatile pint	*atomic,
		  pint		val)
{
	pint result;

	__MB ();
	result = __ATOMIC_ADD_LONG ((volatile ppointer) atomic, val);
	__MB ();

	return result;
}

P_LIB_API puint
p_atomic_int_and (volatile puint	*atomic,
		  puint			val)
{
	puint result;

	__MB ();
	result = (puint) __ATOMIC_AND_LONG ((volatile ppointer) atomic, (pint) val);
	__MB ();

	return result;
}

P_LIB_API puint
p_atomic_int_or (volatile puint	*atomic,
		 puint		val)
{
	puint result;

	__MB ();
	result = (puint) __ATOMIC_OR_LONG ((volatile ppointer) atomic, (pint) val);
	__MB ();

	return result;
}

P_LIB_API puint
p_atomic_int_xor (volatile puint	*atomic,
		  puint			val)
{
	pint i;

	do {
		__MB ();
		i = (pint) (*atomic);
	} while (PATOMIC_DECC_CAS_LONG (atomic, i, i ^ ((pint) val), atomic) != 1);

	__MB ();

	return i;
}

P_LIB_API ppointer
p_atomic_pointer_get (volatile pconstpointer atomic)
{
	__MB ();
	return (ppointer) (* ((const volatile psize *) atomic));
}

P_LIB_API void
p_atomic_pointer_set (volatile ppointer	atomic,
		      ppointer		val)
{
#if (PLIBSYS_SIZEOF_VOID_P == 8)
	(void) __ATOMIC_EXCH_QUAD (atomic, (pint64) val);
#else
	(void) __ATOMIC_EXCH_LONG (atomic, (pint) val);
#endif
	__MB ();
}

P_LIB_API pboolean
p_atomic_pointer_compare_and_exchange (volatile ppointer	atomic,
				       ppointer			oldval,
				       ppointer			newval)
{
	pboolean result;

	__MB ();
#if (PLIBSYS_SIZEOF_VOID_P == 8)
	result = PATOMIC_DECC_CAS_QUAD (atomic, oldval, newval, atomic) == 1 ? TRUE : FALSE;
#else
	result = PATOMIC_DECC_CAS_LONG (atomic, oldval, newval, atomic) == 1 ? TRUE : FALSE;
#endif
	__MB ();

	return result;
}

P_LIB_API pssize
p_atomic_pointer_add (volatile ppointer	atomic,
		      pssize		val)
{
	pssize result;

	__MB ();
#if (PLIBSYS_SIZEOF_VOID_P == 8)
	result = (pssize) __ATOMIC_ADD_QUAD (atomic, (pint64) val);
#else
	result = (pssize) __ATOMIC_ADD_LONG (atomic, (pint) val);
#endif
	__MB ();

	return result;
}

P_LIB_API psize
p_atomic_pointer_and (volatile ppointer	atomic,
		      psize		val)
{
	psize result;

	__MB ();
#if (PLIBSYS_SIZEOF_VOID_P == 8)
	result = (psize) __ATOMIC_AND_QUAD (atomic, (pint64) val);
#else
	result = (psize) __ATOMIC_AND_LONG (atomic, (pint) val);
#endif
	__MB ();

	return result;
}

P_LIB_API psize
p_atomic_pointer_or (volatile ppointer	atomic,
		     psize		val)
{
	psize result;

	__MB ();
#if (PLIBSYS_SIZEOF_VOID_P == 8)
	result = (psize) __ATOMIC_OR_QUAD (atomic, (pint64) val);
#else
	result = (psize) __ATOMIC_OR_LONG (atomic, (pint) val);
#endif
	__MB ();

	return result;
}

P_LIB_API psize
p_atomic_pointer_xor (volatile ppointer	atomic,
		      psize		val)
{
#if (PLIBSYS_SIZEOF_VOID_P == 8)
	pint64 i;

	do {
		__MB ();
		i = (pint64) (* ((volatile psize *) atomic));
	} while (PATOMIC_DECC_CAS_QUAD (atomic, i, i ^ ((pint64) val), atomic) != 1);
#else
	pint i;

	do {
		__MB ();
		i = (pint) (* ((volatile psize *) atomic));
	} while (PATOMIC_DECC_CAS_LONG (atomic, i, i ^ ((pint) val), atomic) != 1);
#endif
	__MB ();

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
