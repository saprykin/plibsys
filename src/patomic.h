/*
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
 * See gatomic.h for other copyrights and details.
 */

/**
 * @file patomic.h
 * @brief Atomic operations
 * @author Alexander Saprykin
 *
 * Atomic operations can be used to avoid thread synchronization primitives such as
 * mutexes, semaphores and so on. These operations are performed atomically and can't be
 * preempted by another thread.
 */

#if !defined (__PLIB_H_INSIDE__) && !defined (PLIB_COMPILATION)
#  error "Header files shouldn't be included directly, consider using <plib.h> instead."
#endif

#ifndef __P_ATOMIC_H__
#define __P_ATOMIC_H__

#include <ptypes.h>
#include <pmacros.h>

P_BEGIN_DECLS

/**
 * @brief Gets integer value from @a *atomic atomically.
 * @param atomic Pointer to integer to get value from.
 * @return Integer value.
 * @since 0.0.1
 */
P_LIB_API pint		p_atomic_int_get			(const volatile pint	*atomic);

/**
 * @brief Sets integer value to @a *atomic atomically.
 * @param[out] atomic Pointer to integer to set value for.
 * @param newval New value.
 * @since 0.0.1
 */
P_LIB_API void		p_atomic_int_set			(volatile pint		*atomic,
								 pint			newval);

/**
 * @brief Increments integer value from @a *atomic atomically.
 * @param[in, out] atomic Pointer to integer to increment the value.
 * @since 0.0.1
 */
P_LIB_API void		p_atomic_int_inc			(volatile pint		*atomic);

/**
 * @brief Decrements integer value from @a *atomic atomically and tests the result.
 * @param[in, out] atomic Pointer to integer to decrement the value.
 * @return TRUE if new value is equal to zero, FALSE otherwise.
 * @since 0.0.1
 */
P_LIB_API pboolean	p_atomic_int_dec_and_test		(volatile pint		*atomic);

/**
 * @brief Compares @a oldval with value pointed to by @a atomic and if they are equal,
 * atomically exchanges @a *atomic with @a newval. Acts as a memory barrier.
 * @param[in, out] atomic Pointer to integer.
 * @param oldval Old value.
 * @param newval New value.
 * @return TRUE if @a *atomic was equiled @a oldval, FALSE otherwise.
 * @since 0.0.1
 */
P_LIB_API pboolean	p_atomic_int_compare_and_exchange	(volatile pint		*atomic,
								 pint			oldval,
								 pint			newval);

/**
 * @brief Atomically adds integer value.
 * @param[in, out] atomic Pointer to integer.
 * @param val Integer to add to @a atomic integer.
 * @return Old value before the addition.
 * @since 0.0.1
 */
P_LIB_API pint		p_atomic_int_add			(volatile pint		*atomic,
								 pint			val);

/**
 * @brief Atomically performs bitwise 'and' operation of the unsigned integer
 * @a *atomic and unsigned integer @a val storing result value back into the @a *atomic.
 * @param[in, out] atomic Pointer to unsigned integer.
 * @param val Unsigned integer to perform bitwise 'and' with @a *atomic integer.
 * @return Old value before the operation.
 * @since 0.0.1
 */
P_LIB_API puint		p_atomic_int_and			(volatile puint		*atomic,
								 puint			val);


/**
 * @brief Atomically performs bitwise 'or' operation of the unsigned integer
 * @a *atomic and unsigned integer @a val storing result value back into the @a *atomic.
 * @param[in, out] atomic Pointer to unsigned integer.
 * @param val Unsigned integer to perform bitwise 'or' with @a *atomic integer.
 * @return Old value before the operation.
 * @since 0.0.1
 */
P_LIB_API puint		p_atomic_int_or				(volatile puint		*atomic,
								 puint			val);

/**
 * @brief Atomically performs bitwise 'xor' operation of the unsigned integer
 * @a *atomic and unsigned integer @a val storing result value back into the @a *atomic.
 * @param[in, out] atomic Pointer to unsigned integer.
 * @param val Unsigned integer to perform bitwise 'xor' with @a *atomic integer.
 * @return Old value before the operation.
 * @since 0.0.1
 */
P_LIB_API puint		p_atomic_int_xor			(volatile puint		*atomic,
								 puint			val);

/**
 * @brief Gets #ppointer-sized value from @a atomic atomically.
 * @param atomic Pointer to get value from.
 * @return Value from the pointer.
 * @since 0.0.1
 */
P_LIB_API ppointer	p_atomic_pointer_get			(const volatile void	*atomic);

/**
 * @brief Sets value to #ppointer-sized variable @a atomic atomically.
 * @param[out] atomic Pointer to set value for.
 * @param newval New pointer value.
 * @since 0.0.1
 */
P_LIB_API void		p_atomic_pointer_set			(volatile void		*atomic,
								 ppointer		newval);

/**
 * @brief Compares value from the @a oldval pointer with value pointed to by
 * the @a atomic pointer and if they are equal, atomically exchanges @a *atomic
 * with @a newval. Acts as a memory barrier.
 * @param[in, out] atomic Pointer to compare value from.
 * @param oldval Old value pointer.
 * @param newval New value pointer.
 * @return TRUE if @a *atomic was equiled @a oldval, FALSE otherwise.
 * @since 0.0.1
 */
P_LIB_API pboolean	p_atomic_pointer_compare_and_exchange	(volatile void		*atomic,
								 ppointer 		oldval,
								 ppointer		newval);

/**
 * @brief Atomically adds #ppointer-sized value.
 * @param[in, out] atomic Pointer to #ppointer-sized value.
 * @param val Value to add to @a atomic value.
 * @return Old value before the addition.
 * @since 0.0.1
 */
P_LIB_API pssize	p_atomic_pointer_add			(volatile void		*atomic,
								 pssize			val);

/**
 * @brief Atomically performs bitwise 'and' operation of the #ppointer-sized
 * @a *atomic and #ppointer-sized @a val storing result value back into the @a *atomic.
 * @param[in, out] atomic Pointer to #ppointer-sized value.
 * @param val Value to perform bitwise 'and' with @a *atomic value.
 * @return Old value before the operation.
 * @since 0.0.1
 */
P_LIB_API psize		p_atomic_pointer_and			(volatile void		*atomic,
								 psize			val);


/**
 * @brief Atomically performs bitwise 'or' operation of the #ppointer-sized
 * @a *atomic and #ppointer-sized @a val storing result value back into the @a *atomic.
 * @param[in, out] atomic Pointer to #ppointer-sized value.
 * @param val Value to perform bitwise 'or' with @a *atomic value.
 * @return Old value before the operation.
 * @since 0.0.1
 */
P_LIB_API psize		p_atomic_pointer_or			(volatile void		*atomic,
								 psize			val);

/**
 * @brief Atomically performs bitwise 'xor' operation of the #ppointer-sized
 * @a *atomic and #ppointer-sized @a val storing result value back into the @a *atomic.
 * @param[in, out] atomic Pointer to #ppointer-sized value.
 * @param val Value to perform bitwise 'xor' with @a *atomic value.
 * @return Old value before the operation.
 * @since 0.0.1
 */
P_LIB_API psize		p_atomic_pointer_xor			(volatile void		*atomic,
								 psize			val);

#if defined (PLIB_ATOMIC_LOCK_FREE) && defined (__GCC_HAVE_SYNC_COMPARE_AND_SWAP_4)
   /* We prefer the new C11-style atomic extension of GCC if available,
    * see: https://bugzilla.gnome.org/show_bug.cgi?id=730807 */
#  if defined (__ATOMIC_SEQ_CST)

#    define p_atomic_int_get(atomic)						\
({										\
	(pint) __atomic_load_4 ((atomic), __ATOMIC_SEQ_CST);			\
})

#    define p_atomic_int_set(atomic, newval)					\
({										\
	__atomic_store_4 ((atomic), (newval), __ATOMIC_SEQ_CST);		\
})

#    if (PLIB_SIZEOF_VOID_P == 8)

#      define p_atomic_pointer_get(atomic)					\
({										\
	(ppointer) __atomic_load_8 ((atomic), __ATOMIC_SEQ_CST);		\
})

#      define p_atomic_pointer_set(atomic, newval)				\
({										\
	__atomic_store_8 ((atomic), (psize) (newval), __ATOMIC_SEQ_CST);	\
})

#    else /* PLIB_SIZEOF_VOID_P == 8 */

#      define p_atomic_pointer_get(atomic)					\
({										\
	(ppointer) __atomic_load_4 ((atomic), __ATOMIC_SEQ_CST);		\
})

#      define p_atomic_pointer_set(atomic, newval)				\
({										\
	__atomic_store_4 ((atomic), (psize) (newval), __ATOMIC_SEQ_CST);	\
})

#    endif /* PLIB_SIZEOF_VOID_P == 8 */

#    else /* defined(__ATOMIC_SEQ_CST) */

#      define p_atomic_int_get(atomic)						\
({										\
	__sync_synchronize ();							\
	(pint) *(atomic);							\
})

#      define p_atomic_int_set(atomic, newval)					\
({										\
	*(atomic) = (newval);							\
	__sync_synchronize ();							\
})

#      define p_atomic_pointer_get(atomic)					\
({										\
	__sync_synchronize ();							\
	(ppointer) *(atomic);							\
})

#      define p_atomic_pointer_set(atomic, newval)				\
({										\
	*(atomic) = (__typeof__ (*(atomic))) (psize) (newval);			\
	__sync_synchronize ();							\
})

#    endif /* !defined(__ATOMIC_SEQ_CST) */

#    define p_atomic_int_inc(atomic)						\
({										\
	(void) __sync_fetch_and_add ((atomic), 1);				\
})

#    define p_atomic_int_dec_and_test(atomic)					\
({										\
	__sync_fetch_and_sub ((atomic), 1) == 1;				\
})

#    define p_atomic_int_compare_and_exchange(atomic, oldval, newval)		\
({										\
	(pboolean) __sync_bool_compare_and_swap ((atomic), (oldval), (newval));	\
})

#    define p_atomic_int_add(atomic, val)					\
({										\
	(pint) __sync_fetch_and_add ((atomic), (val));				\
})

#    define p_atomic_int_and(atomic, val)					\
({										\
	(puint) __sync_fetch_and_and ((atomic), (val));				\
})

#    define p_atomic_int_or(atomic, val)					\
({										\
	(puint) __sync_fetch_and_or ((atomic), (val));				\
})

#    define p_atomic_int_xor(atomic, val)					\
({										\
	(puint) __sync_fetch_and_xor ((atomic), (val));				\
})

#    define p_atomic_pointer_compare_and_exchange(atomic, oldval, newval)	\
({										\
	(pboolean) __sync_bool_compare_and_swap ((atomic), (oldval), (newval));	\
})

#    define p_atomic_pointer_add(atomic, val)					\
({										\
	(pssize) __sync_fetch_and_add ((atomic), (ppointer) (val));		\
})

#    define p_atomic_pointer_and(atomic, val)					\
({										\
	(psize) __sync_fetch_and_and ((atomic), (ppointer) (val));		\
})

#    define p_atomic_pointer_or(atomic, val)					\
({										\
	(psize) __sync_fetch_and_or ((atomic), (ppointer) (val));		\
})

#    define p_atomic_pointer_xor(atomic, val)					\
({										\
	(psize) __sync_fetch_and_xor ((atomic), (ppointer) (val));		\
})

#else /* defined(PLIB_ATOMIC_LOCK_FREE) && defined(__GCC_HAVE_SYNC_COMPARE_AND_SWAP_4) */

#  define p_atomic_int_get(atomic)					\
	(p_atomic_int_get ((pint *) (atomic)))
#  define p_atomic_int_set(atomic, newval)				\
	(p_atomic_int_set ((pint *) (atomic), (pint) (newval)))
#  define p_atomic_int_compare_and_exchange(atomic, oldval, newval)	\
	(p_atomic_int_compare_and_exchange ((pint *) (atomic), (oldval), (newval)))
#  define p_atomic_int_add(atomic, val)					\
	(p_atomic_int_add ((pint *) (atomic), (val)))
#  define p_atomic_int_and(atomic, val)					\
	(p_atomic_int_and ((puint *) (atomic), (val)))
#  define p_atomic_int_or(atomic, val)					\
	(p_atomic_int_or ((puint *) (atomic), (val)))
#  define p_atomic_int_xor(atomic, val)					\
	(p_atomic_int_xor ((puint *) (atomic), (val)))
#  define p_atomic_int_inc(atomic)					\
	(p_atomic_int_inc ((pint *) (atomic)))
#  define p_atomic_int_dec_and_test(atomic)				\
	(p_atomic_int_dec_and_test ((pint *) (atomic)))

#  define p_atomic_pointer_get(atomic)					\
	(p_atomic_pointer_get (atomic))
#  define p_atomic_pointer_set(atomic, newval)				\
	(p_atomic_pointer_set ((atomic), (ppointer) (newval)))
#  define p_atomic_pointer_compare_and_exchange(atomic, oldval, newval)	\
	(p_atomic_pointer_compare_and_exchange ((atomic), (ppointer) (oldval), (ppointer) (newval)))
#  define p_atomic_pointer_add(atomic, val)				\
	(p_atomic_pointer_add ((atomic), (pssize) (val)))
#  define p_atomic_pointer_and(atomic, val)				\
	(p_atomic_pointer_and ((atomic), (psize) (val)))
#  define p_atomic_pointer_or(atomic, val)				\
	(p_atomic_pointer_or ((atomic), (psize) (val)))
#  define p_atomic_pointer_xor(atomic, val)				\
	(p_atomic_pointer_xor ((atomic), (psize) (val)))

#endif /* PLIB_ATOMIC_LOCK_FREE && __GCC_HAVE_SYNC_COMPARE_AND_SWAP_4 */

P_END_DECLS

#endif /* __P_ATOMIC_H__ */
