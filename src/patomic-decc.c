/*
 * The MIT License
 *
 * Copyright (C) 2016 Alexander Saprykin <saprykin.spb@gmail.com>
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

#include "patomic.h"

#ifdef P_OS_VMS
#  include <builtins.h>
#else
#  include <machine/builtins.h>
#endif

#ifdef __ia64
#  define PATOMIC_DECC_CAS_LONG(atomic_src, oldval, newval, atomic_dst)	\
	  __CMP_SWAP_LONG ((volatile void *) (atomic_src),		\
			   (pint) (oldval),				\
			   (pint) (newval))
#  define PATOMIC_DECC_CAS_QUAD(atomic_src, oldval, newval, atomic_dst)	\
	  __CMP_SWAP_QUAD ((volatile void *) (atomic_src),		\
			   (pint64) (oldval),				\
			   (pint64) (newval))	  
#else
#  define PATOMIC_DECC_CAS_LONG(atomic_src, oldval, newval, atomic_dst)	\
	  __CMP_STORE_LONG ((volatile void *) (atomic_src),		\
			    (pint) (oldval),				\
			    (pint) (newval),				\
			    (volatile void *) (atomic_dst))
#  define PATOMIC_DECC_CAS_QUAD(atomic_src, oldval, newval, atomic_dst)	\
	  __CMP_STORE_QUAD ((volatile void *) (atomic_src),		\
			    (pint64) (oldval),				\
			    (pint64) (newval),				\
			    (volatile void *) (atomic_dst))
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
	(void) __ATOMIC_EXCH_LONG ((volatile void *) atomic, val);
	__MB ();
}

P_LIB_API void
p_atomic_int_inc (volatile pint *atomic)
{
	__MB ();
	(void) __ATOMIC_INCREMENT_LONG ((volatile void *) atomic);
	__MB ();
}

P_LIB_API pboolean
p_atomic_int_dec_and_test (volatile pint *atomic)
{
	pboolean result;

	__MB ();
	result = __ATOMIC_DECREMENT_LONG ((volatile void *) atomic) == 1 ? TRUE : FALSE;
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
	result = __ATOMIC_ADD_LONG ((volatile void *) atomic, val);
	__MB ();

	return result;
}

P_LIB_API puint
p_atomic_int_and (volatile puint	*atomic,
		  puint			val)
{
	puint result;

	__MB ();
	result = (puint) __ATOMIC_AND_LONG ((volatile void *) atomic, (pint) val);
	__MB ();

	return result;
}

P_LIB_API puint
p_atomic_int_or (volatile puint	*atomic,
		 puint		val)
{
	puint result;

	__MB ();
	result = (puint) __ATOMIC_OR_LONG ((volatile void *) atomic, (pint) val);
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
	__MB ();
}

P_LIB_API pboolean
p_atomic_pointer_compare_and_exchange (volatile void	*atomic,
				       ppointer		oldval,
				       ppointer		newval)
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
p_atomic_pointer_add (volatile void	*atomic,
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
p_atomic_pointer_and (volatile void	*atomic,
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
p_atomic_pointer_or (volatile void	*atomic,
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
p_atomic_pointer_xor (volatile void	*atomic,
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
