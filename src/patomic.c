/*
 * Copyright (C) 2011 Ryan Lortie <desrt@desrt.ca>
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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA.
 */

/*
 * This file was taken from GLib and adopted for using along with PLib.
 * See gatomic.c for other copyrights and details.
 */

#include "patomic.h"
#include "pmutex.h"

#ifdef PLIB_ATOMIC_LOCK_FREE
#  if defined (__GCC_HAVE_SYNC_COMPARE_AND_SWAP_4)
#    if !defined (__ATOMIC_SEQ_CST)
/* LCOV_EXCL_START */
pint
(p_atomic_int_get) (const volatile pint *atomic)
{
	return p_atomic_int_get (atomic);
}

void
(p_atomic_int_set) (volatile pint	*atomic,
		    pint		newval)
{
	p_atomic_int_set (atomic, newval);
}

void
(p_atomic_int_inc) (volatile pint *atomic)
{
	p_atomic_int_inc (atomic);
}

pboolean
(p_atomic_int_dec_and_test) (volatile pint *atomic)
{
	return p_atomic_int_dec_and_test (atomic);
}

pboolean
(p_atomic_int_compare_and_exchange) (volatile pint	*atomic,
				     pint		oldval,
				     pint		newval)
{
	return p_atomic_int_compare_and_exchange (atomic, oldval, newval);
}

pint
(p_atomic_int_add) (volatile pint	*atomic,
		    pint		val)
{
	return p_atomic_int_add (atomic, val);
}

puint
(p_atomic_int_and) (volatile puint	*atomic,
		    puint		val)
{
	return p_atomic_int_and (atomic, val);
}

puint
(p_atomic_int_or) (volatile puint	*atomic,
		   puint		val)
{
	return p_atomic_int_or (atomic, val);
}

puint
(p_atomic_int_xor) (volatile puint	*atomic,
		    puint		val)
{
	return p_atomic_int_xor (atomic, val);
}

ppointer
(p_atomic_pointer_get) (const volatile void *atomic)
{
	return p_atomic_pointer_get ((const volatile ppointer *) atomic);
}

void
(p_atomic_pointer_set) (volatile void	*atomic,
			ppointer	newval)
{
	p_atomic_pointer_set ((volatile ppointer *) atomic, newval);
}

pboolean
(p_atomic_pointer_compare_and_exchange) (volatile void	*atomic,
					 ppointer	oldval,
					 ppointer	newval)
{
	return p_atomic_pointer_compare_and_exchange ((volatile ppointer *) atomic,
						      oldval,
						      newval);
}

pssize
(p_atomic_pointer_add) (volatile void	*atomic,
			pssize		val)
{
	return p_atomic_pointer_add ((volatile ppointer *) atomic, val);
}

psize
(p_atomic_pointer_and) (volatile void	*atomic,
			psize		val)
{
	return p_atomic_pointer_and ((volatile ppointer *) atomic, val);
}

psize
(p_atomic_pointer_or) (volatile void	*atomic,
		       psize		val)
{
	return p_atomic_pointer_or ((volatile ppointer *) atomic, val);
}

psize
(p_atomic_pointer_xor) (volatile void	*atomic,
			psize		val)
{
	return p_atomic_pointer_xor ((volatile ppointer *) atomic, val);
}
/* LCOV_EXCL_STOP */
#    endif /* !defined (__ATOMIC_SEQ_CST) */
#  elif defined (P_OS_WIN)

#    include <windows.h>
#    if !defined (P_OS_WIN64) && !(defined (P_CC_MSVC) && _MSC_VER <= 1200)
#      define InterlockedAnd _InterlockedAnd
#      define InterlockedOr _InterlockedOr
#      define InterlockedXor _InterlockedXor
#    endif

#    if !defined (P_CC_MSVC) || _MSC_VER <= 1200

/* Inlined versions for older compiler */
static LONG
_pInterlockedAnd (volatile puint	*atomic,
		  puint			val)
{
	LONG i, j;

	j = *atomic;
	do {
		i = j;
		j = InterlockedCompareExchange (atomic, i & val, i);
	} while (i != j);

	return j;
}

#      define InterlockedAnd(a,b) _pInterlockedAnd(a,b)

static LONG
_pInterlockedOr (volatile puint	*atomic,
		 puint		val)
{
	LONG i, j;

	j = *atomic;
	do {
		i = j;
		j = InterlockedCompareExchange (atomic, i | val, i);
	} while (i != j);

	return j;
}

#      define InterlockedOr(a,b) _pInterlockedOr(a,b)

static LONG
_pInterlockedXor (volatile puint	*atomic,
		  puint			val)
{
	LONG i, j;

	j = *atomic;
	do {
		i = j;
		j = InterlockedCompareExchange (atomic, i ^ val, i);
	} while (i != j);

	return j;
}

#      define InterlockedXor(a,b) _pInterlockedXor(a,b)
#    endif

/*
 * http://msdn.microsoft.com/en-us/library/ms684122(v=vs.85).aspx
 */
pint
(p_atomic_int_get) (const volatile pint *atomic)
{
	MemoryBarrier ();
	return *atomic;
}

void
(p_atomic_int_set) (volatile pint	*atomic,
		    pint		newval)
{
	*atomic = newval;
	MemoryBarrier ();
}

void
(p_atomic_int_inc) (volatile pint *atomic)
{
	InterlockedIncrement (atomic);
}

pboolean
(p_atomic_int_dec_and_test) (volatile pint *atomic)
{
	return InterlockedDecrement (atomic) == 0;
}

pboolean
(p_atomic_int_compare_and_exchange) (volatile pint	*atomic,
				     pint		oldval,
				     pint		newval)
{
	return InterlockedCompareExchange (atomic, newval, oldval) == oldval;
}

pint
(p_atomic_int_add) (volatile pint	*atomic,
		    pint		val)
{
	return InterlockedExchangeAdd (atomic, val);
}

puint
(p_atomic_int_and) (volatile puint	*atomic,
		    puint		val)
{
	return InterlockedAnd (atomic, val);
}

puint
(p_atomic_int_or) (volatile puint	*atomic,
		   puint		val)
{
	return InterlockedOr (atomic, val);
}

puint
(p_atomic_int_xor) (volatile puint	*atomic,
		    puint		val)
{
	return InterlockedXor (atomic, val);
}

ppointer
(p_atomic_pointer_get) (const volatile void *atomic)
{
	const volatile ppointer *ptr = (const volatile ppointer *) atomic;

	MemoryBarrier ();
	return *ptr;
}

void
(p_atomic_pointer_set) (volatile void	*atomic,
			ppointer	newval)
{
	volatile ppointer *ptr = (volatile ppointer *) atomic;

	*ptr = newval;
	MemoryBarrier ();
}

pboolean
(p_atomic_pointer_compare_and_exchange) (volatile void	*atomic,
					 ppointer	oldval,
					 ppointer	newval)
{
	return InterlockedCompareExchangePointer ((volatile PVOID *) atomic, newval, oldval) == oldval;
}

pssize
(p_atomic_pointer_add) (volatile void	*atomic,
			pssize		val)
{
#    if PLIB_SIZEOF_VOID_P == 8
	return InterlockedExchangeAdd64 (atomic, val);
#    else
	return InterlockedExchangeAdd (atomic, val);
#    endif
}

psize
(p_atomic_pointer_and) (volatile void	*atomic,
			psize		val)
{
#    if PLIB_SIZEOF_VOID_P == 8
	return InterlockedAnd64 (atomic, val);
#    else
	return InterlockedAnd (atomic, val);
#    endif
}

psize
(p_atomic_pointer_or) (volatile void	*atomic,
		       psize		val)
{
#    if PLIB_SIZEOF_VOID_P == 8
	return InterlockedOr64 (atomic, val);
#    else
	return InterlockedOr (atomic, val);
#    endif
}

psize
(p_atomic_pointer_xor) (volatile void	*atomic,
			psize		val)
{
#    if PLIB_SIZEOF_VOID_P == 8
	return InterlockedXor64 (atomic, val);
#    else
	return InterlockedXor (atomic, val);
#    endif
}
#  else
#    error PLIB_ATOMIC_LOCK_FREE defined, but incapable of lock-free atomics.
#  endif /* defined (__GCC_HAVE_SYNC_COMPARE_AND_SWAP_4) */

#else /* PLIB_ATOMIC_LOCK_FREE */

/* We have to use the slow, but safe locking method. */
static PMutex *p_atomic_mutex;

pint
(p_atomic_int_get) (const volatile pint *atomic)
{
	pint value;

	p_mutex_lock (p_atomic_mutex);
	value = *atomic;
	p_mutex_unlock (p_atomic_mutex);

	return value;
}

void
(p_atomic_int_set) (volatile pint	*atomic,
		    pint		value)
{
	p_mutex_lock (p_atomic_mutex);
	*atomic = value;
	p_mutex_unlock (p_atomic_mutex);
}

void
(p_atomic_int_inc) (volatile pint *atomic)
{
	p_mutex_lock (p_atomic_mutex);
	(*atomic)++;
	p_mutex_unlock (p_atomic_mutex);
}

pboolean
(p_atomic_int_dec_and_test) (volatile pint *atomic)
{
	pboolean is_zero;

	p_mutex_lock (p_atomic_mutex);
	is_zero = --(*atomic) == 0;
	p_mutex_unlock (p_atomic_mutex);

	return is_zero;
}

pboolean
(p_atomic_int_compare_and_exchange) (volatile pint	*atomic,
				     pint		oldval,
				     pint		newval)
{
	pboolean success;

	p_mutex_lock (p_atomic_mutex);

	if ((success = (*atomic == oldval)))
		*atomic = newval;

	p_mutex_unlock (p_atomic_mutex);

	return success;
}

pint
(p_atomic_int_add) (volatile pint	*atomic,
		    pint		val)
{
	pint oldval;

	p_mutex_lock (p_atomic_mutex);
	oldval = *atomic;
	*atomic = oldval + val;
	p_mutex_unlock (p_atomic_mutex);

	return oldval;
}

puint
(p_atomic_int_and) (volatile puint	*atomic,
		    puint		val)
{
	puint oldval;

	p_mutex_lock (p_atomic_mutex);
	oldval = *atomic;
	*atomic = oldval & val;
	p_mutex_unlock (p_atomic_mutex);

	return oldval;
}

puint
(p_atomic_int_or) (volatile puint	*atomic,
		   puint		val)
{
	puint oldval;

	p_mutex_lock (p_atomic_mutex);
	oldval = *atomic;
	*atomic = oldval | val;
	p_mutex_unlock (p_atomic_mutex);

	return oldval;
}

puint
(p_atomic_int_xor) (volatile puint	*atomic,
		    puint		val)
{
	puint oldval;

	p_mutex_lock (p_atomic_mutex);
	oldval = *atomic;
	*atomic = oldval ^ val;
	p_mutex_unlock (p_atomic_mutex);

	return oldval;
}

ppointer
(p_atomic_pointer_get) (const volatile void *atomic)
{
	const volatile ppointer *ptr = atomic;
	ppointer value;

	p_mutex_lock (p_atomic_mutex);
	value = *ptr;
	p_mutex_unlock (p_atomic_mutex);

	return value;
}

void
(p_atomic_pointer_set) (volatile void	*atomic,
			ppointer	newval)
{
	volatile ppointer *ptr = atomic;

	p_mutex_lock (p_atomic_mutex);
	*ptr = newval;
	p_mutex_unlock (p_atomic_mutex);
}

pboolean
(p_atomic_pointer_compare_and_exchange) (volatile void	*atomic,
					 ppointer	oldval,
					 ppointer	newval)
{
	volatile ppointer *ptr = atomic;
	pboolean success;

	p_mutex_lock (p_atomic_mutex);

	if ((success = (*ptr == oldval)))
		*ptr = newval;

	p_mutex_unlock (p_atomic_mutex);

	return success;
}

pssize
(p_atomic_pointer_add) (volatile void	*atomic,
			pssize		val)
{
	volatile pssize *ptr = atomic;
	pssize oldval;

	p_mutex_lock (p_atomic_mutex);
	oldval = *ptr;
	*ptr = oldval + val;
	p_mutex_unlock (p_atomic_mutex);

	return oldval;
}

psize
(p_atomic_pointer_and) (volatile void	*atomic,
			psize		val)
{
	volatile psize *ptr = atomic;
	psize oldval;

	p_mutex_lock (p_atomic_mutex);
	oldval = *ptr;
	*ptr = oldval & val;
	p_mutex_unlock (p_atomic_mutex);

	return oldval;
}

psize
(p_atomic_pointer_or) (volatile void	*atomic,
		       psize		val)
{
	volatile psize *ptr = atomic;
	psize oldval;

	p_mutex_lock (p_atomic_mutex);
	oldval = *ptr;
	*ptr = oldval | val;
	p_mutex_unlock (p_atomic_mutex);

	return oldval;
}

psize
(p_atomic_pointer_xor) (volatile void	*atomic,
			psize		val)
{
	volatile psize *ptr = atomic;
	psize oldval;

	p_mutex_lock (p_atomic_mutex);
	oldval = *ptr;
	*ptr = oldval ^ val;
	p_mutex_unlock (p_atomic_mutex);

	return oldval;
}
#endif

void
__p_atomic_thread_init (void)
{
#ifndef PLIB_ATOMIC_LOCK_FREE
	p_atomic_mutex = p_mutex_new ();
#endif /* !PLIB_ATOMIC_LOCK_FREE */
}

void
__p_atomic_thread_shutdown (void)
{
#ifndef PLIB_ATOMIC_LOCK_FREE
	if (p_atomic_mutex != NULL) {
		p_mutex_free (p_atomic_mutex);
		p_atomic_mutex = NULL;
	}
#endif /* !PLIB_ATOMIC_LOCK_FREE */
}
