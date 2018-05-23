/*
 * The MIT License
 *
 * Copyright (C) 2010-2016 Alexander Saprykin <saprykin.spb@gmail.com>
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

/**
 * @file patomic.h
 * @brief Atomic operations
 * @author Alexander Saprykin
 *
 * Atomic operations can be used to avoid heavy thread synchronization
 * primitives such as mutexes, semaphores and so on. These operations are
 * performed atomically and can't be preempted by another thread.
 *
 * Lock-free atomic operations require software and hardware support. Usually
 * lock-free atomic operations are implemented with low-level using assembly
 * inlines. Some of the compilers provide built-in routines to perform atomic
 * operations. You can use the p_atomic_is_lock_free() call to check whether
 * such a support is provided or not.
 *
 * If there is no hardware or software support for lock-free atomic operations
 * then they can be simulated (though in rather slower manner) using a thread
 * global synchronization primitive (i.e. mutex), but it could block threads
 * while performing atomic operations on distinct variables from distinct
 * threads.
 *
 * The Windows platform provides all the required lock-free operations in most
 * cases, so it always has lock-free support.
 */

#if !defined (PLIBSYS_H_INSIDE) && !defined (PLIBSYS_COMPILATION)
#  error "Header files shouldn't be included directly, consider using <plibsys.h> instead."
#endif

#ifndef PLIBSYS_HEADER_PATOMIC_H
#define PLIBSYS_HEADER_PATOMIC_H

#include <ptypes.h>
#include <pmacros.h>

P_BEGIN_DECLS

/**
 * @brief Gets #pint value from @a atomic.
 * @param atomic Pointer to #pint to get the value from.
 * @return Integer value.
 * @since 0.0.1
 *
 * This call acts as a full compiler and hardware memory barrier (before the
 * get).
 */
P_LIB_API pint		p_atomic_int_get			(const volatile pint	*atomic);

/**
 * @brief Sets #pint value to @a atomic.
 * @param[out] atomic Pointer to #pint to set the value for.
 * @param val New #pint value.
 * @since 0.0.1
 *
 * This call acts as a full compiler and hardware memory barrier (after the
 * set).
 */
P_LIB_API void		p_atomic_int_set			(volatile pint		*atomic,
								 pint			val);

/**
 * @brief Increments #pint value from @a atomic by 1.
 * @param[in,out] atomic Pointer to #pint to increment the value.
 * @since 0.0.1
 *
 * Think of this operation as an atomic version of `{ *atomic += 1; }`.
 *
 * This call acts as a full compiler and hardware memory barrier.
 */
P_LIB_API void		p_atomic_int_inc			(volatile pint		*atomic);

/**
 * @brief Decrements #pint value from @a atomic by 1 and tests the result for
 * zero.
 * @param[in,out] atomic Pointer to #pint to decrement the value.
 * @return TRUE if the new value is equal to zero, FALSE otherwise.
 * @since 0.0.1
 *
 * Think of this operation as an atomic version of
 * `{ *atomic -= 1; return (*atomic == 0); }`.
 *
 * This call acts as a full compiler and hardware memory barrier.
 */
P_LIB_API pboolean	p_atomic_int_dec_and_test		(volatile pint		*atomic);

/**
 * @brief Compares @a oldval with the value pointed to by @a atomic and if
 * they are equal, atomically exchanges the value of @a atomic with @a newval.
 * @param[in,out] atomic Pointer to #pint.
 * @param oldval Old #pint value.
 * @param newval New #pint value.
 * @return TRUE if @a atomic value was equal @a oldval, FALSE otherwise.
 * @since 0.0.1
 *
 * This compare and exchange is done atomically.
 *
 * Think of this operation as an atomic version of
 * `{ if (*atomic == oldval) { *atomic = newval; return TRUE; } else return FALSE; }`.
 *
 * This call acts as a full compiler and hardware memory barrier.
 */
P_LIB_API pboolean	p_atomic_int_compare_and_exchange	(volatile pint		*atomic,
								 pint			oldval,
								 pint			newval);

/**
 * @brief Atomically adds #pint value to @a atomic value.
 * @param[in,out] atomic Pointer to #pint.
 * @param val Integer to add to @a atomic value.
 * @return Old value before the addition.
 * @since 0.0.1
 *
 * Think of this operation as an atomic version of
 * `{ tmp = *atomic; *atomic += val; return tmp; }`.
 *
 * This call acts as a full compiler and hardware memory barrier.
 */
P_LIB_API pint		p_atomic_int_add			(volatile pint		*atomic,
								 pint			val);

/**
 * @brief Atomically performs the bitwise 'and' operation of @a atomic value
 * and @a val storing the result back in @a atomic.
 * @param[in,out] atomic Pointer to #puint.
 * @param val #puint to perform bitwise 'and' with @a atomic value.
 * @return Old @a atomic value before the operation.
 * @since 0.0.1
 *
 * This call acts as a full compiler and hardware memory barrier.
 *
 * Think of this operation as an atomic version of
 * `{ tmp = *atomic; *atomic &= val; return tmp; }`.
 */
P_LIB_API puint		p_atomic_int_and			(volatile puint		*atomic,
								 puint			val);

/**
 * @brief Atomically performs the bitwise 'or' operation of @a atomic value
 * and @a val storing the result back in @a atomic.
 * @param[in,out] atomic Pointer to #puint.
 * @param val #puint to perform bitwise 'or' with @a atomic value.
 * @return Old @a atomic value before the operation.
 * @since 0.0.1
 *
 * Think of this operation as an atomic version of
 * `{ tmp = *atomic; *atomic |= val; return tmp; }`.
 *
 * This call acts as a full compiler and hardware memory barrier.
 */
P_LIB_API puint		p_atomic_int_or				(volatile puint		*atomic,
								 puint			val);

/**
 * @brief Atomically performs the bitwise 'xor' operation of @a atomic value
 * and @a val storing the result back in @a atomic.
 * @param[in,out] atomic Pointer to #puint.
 * @param val #puint to perform bitwise 'xor' with @a atomic value.
 * @return Old @a atomic value before the operation.
 * @since 0.0.1
 *
 * Think of this operation as an atomic version of
 * `{ tmp = *atomic; *atomic ^= val; return tmp; }`.
 *
 * This call acts as a full compiler and hardware memory barrier.
 */
P_LIB_API puint		p_atomic_int_xor			(volatile puint		*atomic,
								 puint			val);

/**
 * @brief Gets #ppointer-sized value from @a atomic.
 * @param atomic Pointer to get the value from.
 * @return Value from the pointer.
 * @since 0.0.1
 *
 * This call acts as a full compiler and hardware memory barrier (before the get).
 */
P_LIB_API ppointer	p_atomic_pointer_get			(const volatile void	*atomic);

/**
 * @brief Sets @a val to #ppointer-sized @a atomic.
 * @param[out] atomic Pointer to set the value for.
 * @param val New value for @a atomic.
 * @since 0.0.1
 *
 * This call acts as a full compiler and hardware memory barrier (after the set).
 */
P_LIB_API void		p_atomic_pointer_set			(volatile void		*atomic,
								 ppointer		val);

/**
 * @brief Compares @a oldval with the value pointed to by @a atomic and if
 * they are equal, atomically exchanges the value of @a atomic with @a newval.
 * @param[in,out] atomic Pointer to #ppointer-sized value.
 * @param oldval Old #ppointer-sized value.
 * @param newval New #ppointer-sized value.
 * @return TRUE if @a atomic value was equal @a oldval, FALSE otherwise.
 * @since 0.0.1
 *
 * This compare and exchange is done atomically.
 *
 * Think of this operation as an atomic version of
 * `{ if (*atomic == oldval) { *atomic = newval; return TRUE; } else return FALSE; }`.
 *
 * This call acts as a full compiler and hardware memory barrier.
 */
P_LIB_API pboolean	p_atomic_pointer_compare_and_exchange	(volatile void		*atomic,
								 ppointer 		oldval,
								 ppointer		newval);

/**
 * @brief Atomically adds #ppointer-sized value to @a atomic value.
 * @param[in,out] atomic Pointer to #ppointer-sized value.
 * @param val Value to add to @a atomic value.
 * @return Old value before the addition.
 * @since 0.0.1
 *
 * Think of this operation as an atomic version of
 * `{ tmp = *atomic; *atomic += val; return tmp; }`.
 *
 * This call acts as a full compiler and hardware memory barrier.
 */
P_LIB_API pssize	p_atomic_pointer_add			(volatile void		*atomic,
								 pssize			val);

/**
 * @brief Atomically performs the bitwise 'and' operation of #ppointer-sized
 * @a atomic value and @a val storing the result back in @a atomic.
 * @param[in,out] atomic Pointer to #ppointer-size value.
 * @param val #psize to perform bitwise 'and' with @a atomic value.
 * @return Old @a atomic value before the operation.
 * @since 0.0.1
 *
 * Think of this operation as an atomic version of
 * `{ tmp = *atomic; *atomic &= val; return tmp; }`.
 *
 * This call acts as a full compiler and hardware memory barrier.
 */
P_LIB_API psize		p_atomic_pointer_and			(volatile void		*atomic,
								 psize			val);

/**
 * @brief Atomically performs the bitwise 'or' operation of #ppointer-sized
 * @a atomic value and @a val storing the result back in @a atomic.
 * @param[in,out] atomic Pointer to #ppointer-size value.
 * @param val #psize to perform bitwise 'or' with @a atomic value.
 * @return Old @a atomic value before the operation.
 * @since 0.0.1
 *
 * Think of this operation as an atomic version of
 * `{ tmp = *atomic; *atomic |= val; return tmp; }`.
 *
 * This call acts as a full compiler and hardware memory barrier.
 */
P_LIB_API psize		p_atomic_pointer_or			(volatile void		*atomic,
								 psize			val);

/**
 * @brief Atomically performs the bitwise 'xor' operation of #ppointer-sized
 * @a atomic value and @a val storing the result back in @a atomic.
 * @param[in,out] atomic Pointer to #ppointer-size value.
 * @param val #psize to perform bitwise 'xor' with @a atomic value.
 * @return Old @a atomic value before the operation.
 * @since 0.0.1
 *
 * Think of this operation as an atomic version of
 * `{ tmp = *atomic; *atomic ^= val; return tmp; }`.
 *
 * This call acts as a full compiler and hardware memory barrier.
 */
P_LIB_API psize		p_atomic_pointer_xor			(volatile void		*atomic,
								 psize			val);

/**
 * @brief Checks whether atomic operations are lock-free.
 * @return TRUE in case of success, FALSE otherwise.
 * @since 0.0.1
 *
 * Some underlying atomic model implementations may not support lock-free
 * operations depending on hardware or software.
 */
P_LIB_API pboolean	p_atomic_is_lock_free			(void);

P_END_DECLS

#endif /* PLIBSYS_HEADER_PATOMIC_H */
