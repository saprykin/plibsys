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

#ifdef P_CC_SUN
#  define PATOMIC_INT_CAST(x) (pint *) (x)
#  define PATOMIC_SIZE_CAST(x) (psize *) (x)
#else
#  define PATOMIC_INT_CAST(x) x
#  define PATOMIC_SIZE_CAST(x) x
#endif

P_LIB_API pint
p_atomic_int_get (const volatile pint *atomic)
{
	return (pint) __atomic_load_4 (PATOMIC_INT_CAST (atomic), __ATOMIC_SEQ_CST);
}

P_LIB_API void
p_atomic_int_set (volatile pint	*atomic,
		  pint		val)
{
	__atomic_store_4 (PATOMIC_INT_CAST (atomic), val, __ATOMIC_SEQ_CST);
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

	return (pboolean) __atomic_compare_exchange_n (PATOMIC_INT_CAST (atomic),
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
#if (PLIBSYS_SIZEOF_VOID_P == 8)
	return (ppointer) __atomic_load_8 (PATOMIC_SIZE_CAST ((const volatile psize *) atomic), __ATOMIC_SEQ_CST);
#else
	return (ppointer) __atomic_load_4 (PATOMIC_SIZE_CAST ((const volatile psize *) atomic), __ATOMIC_SEQ_CST);
#endif
}

P_LIB_API void
p_atomic_pointer_set (volatile void	*atomic,
		      ppointer		val)
{
#if (PLIBSYS_SIZEOF_VOID_P == 8)
	__atomic_store_8 (PATOMIC_SIZE_CAST ((volatile psize *) atomic), (psize) val, __ATOMIC_SEQ_CST);
#else
	__atomic_store_4 (PATOMIC_SIZE_CAST ((volatile psize *) atomic), (psize) val, __ATOMIC_SEQ_CST);
#endif
}

P_LIB_API pboolean
p_atomic_pointer_compare_and_exchange (volatile void	*atomic,
				       ppointer		oldval,
				       ppointer		newval)
{
	ppointer tmp_pointer = oldval;

	return (pboolean) __atomic_compare_exchange_n (PATOMIC_SIZE_CAST ((volatile psize *) atomic),
						       (psize *) &tmp_pointer,
						       PPOINTER_TO_PSIZE (newval),
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
p_atomic_thread_init (void)
{
}

void
p_atomic_thread_shutdown (void)
{
}
