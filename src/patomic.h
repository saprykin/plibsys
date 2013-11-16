/* 
 * Copyright (C) 2010 Alexander Saprykin <xelfium@gmail.com>
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
 * mutexes, semaphores and so on. These operations performs atomically and can't be
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
 * @brief Atomically adds @a val to @a *atomic and returns old value just before
 * the addition. Acts as a memory barrier.
 * @param atomic Pointer to integer.
 * @param val Integer to add to @a atomic integer.
 * @return Value before addtition.
 * @since 0.0.1
 */
pint		p_atomic_int_exchange_and_add		(volatile pint		*atomic,
							 pint			val);

/**
 * @brief Atomically adds value.
 * @param atomic Pointer to integer.
 * @param val Integer to add to @a atomic integer.
 * @since 0.0.1
 */
void		p_atomic_int_add			(volatile pint		*atomic,
							 pint			val);

/**
 * @brief Compares @a oldval with value pointed to by @a atomic and if they are equal,
 * atomically exchanges @a *atomic with @a newval. Acts as a memory barrier.
 * @param atomic Pointer to integer.
 * @param oldval Old value.
 * @param newval New value.
 * @return TRUE if @a *atomic was equiled @a oldval, FALSE otherwise.
 * @since 0.0.1
 */
pboolean	p_atomic_int_compare_and_exchange	(volatile pint		*atomic,
							 pint			oldval,
							 pint			newval);

/**
 * @brief Compares @a oldval pointer with value pointed to by @a atomic pointer
 * and if they are equal,atomically exchanges @a *atomic with @a newval. Acts as a memory barrier.
 * @param atomic Pointer to pointer.
 * @param oldval Old value pointer.
 * @param newval New value pointer.
 * @return TRUE if @a *atomic was equiled @a oldval, FALSE otherwise.
 * @since 0.0.1
 */
pboolean	p_atomic_pointer_compare_and_exchange	(volatile ppointer	*atomic, 
							 ppointer 		oldval, 
							 ppointer		newval);

/**
 * @brief Gets integer value from @a *atomic atomically.
 * @param atomic Pointer to integer to get value from.
 * @return Integer value.
 * @since 0.0.1
 */
pint		p_atomic_int_get			(volatile pint		*atomic);

/**
 * @brief Sets integer value to @a *atomic atomically.
 * @param atomic Pointer to integer to set value for.
 * @param newval New value.
 * @since 0.0.1
 */
void		p_atomic_int_set			(volatile pint		*atomic,
							 pint			newval);

/**
 * @brief Gets pointer value from @a *atomic atomically.
 * @param atomic Pointer to pointer to get value from.
 * @return Pointer value.
 * @since 0.0.1
 */
ppointer	p_atomic_pointer_get			(volatile ppointer	*atomic);

/**
 * @brief Sets pointer value to @a *atomic atomically.
 * @param atomic Pointer to pointer to set value for.
 * @param newval New pointer value.
 * @since 0.0.1
 */
void		p_atomic_pointer_set			(volatile ppointer	*atomic,
							 ppointer		newval);

/**
 * @brief Memory barrier function.
 * @since 0.0.1
 */
void		p_atomic_memory_barrier			(void);

#ifndef P_ATOMIC_OP_MEMORY_BARRIER_NEEDED
/**
 * @brief Gets integer value from @a *atomic atomically.
 * @param atomic Pointer to integer to get value from.
 * @return Integer value.
 * @since 0.0.1
 */
#  define p_atomic_int_get(atomic) 		((pint)*(atomic))

/**
 * @brief Sets integer value to @a *atomic atomically.
 * @param atomic Pointer to integer to set value for.
 * @param newval New value.
 * @since 0.0.1
 */
#  define p_atomic_int_set(atomic, newval) 	((void) (*(atomic) = (newval)))

/**
 * @brief Gets pointer value from @a *atomic atomically.
 * @param atomic Pointer to pointer to get value from.
 * @return Pointer value.
 * @since 0.0.1
 */
#  define p_atomic_pointer_get(atomic) 		((ppointer)*(atomic))

/**
 * @brief Sets pointer value to @a *atomic atomically.
 * @param atomic Pointer to pointer to set value for.
 * @param newval New pointer value.
 * @since 0.0.1
 */
#  define p_atomic_pointer_set(atomic, newval)	((void) (*(atomic) = (newval)))
#else
#  define p_atomic_int_get(atomic) \
   ((void) sizeof (pchar [sizeof (*(atomic)) == sizeof (pint) ? 1 : -1]), \
   (p_atomic_int_get) ((volatile pint *) (void *) (atomic)))
#  define p_atomic_int_set(atomic, newval) \
   ((void) sizeof (pchar [sizeof (*(atomic)) == sizeof (pint) ? 1 : -1]), \
   (p_atomic_int_set) ((volatile pint *) (void *) (atomic), (newval)))
#  define p_atomic_pointer_get(atomic) \
   ((void) sizeof (pchar [sizeof (*(atomic)) == sizeof (ppointer) ? 1 : -1]), \
   (p_atomic_pointer_get) ((volatile ppointer *) (void *) (atomic)))
#  define p_atomic_pointer_set(atomic, newval) \
   ((void) sizeof (pchar [sizeof (*(atomic)) == sizeof (ppointer) ? 1 : -1]), \
   (p_atomic_pointer_set) ((volatile ppointer *) (void *) (atomic), (newval)))
#endif /* P_ATOMIC_OP_MEMORY_BARRIER_NEEDED */

/**
 * @brief Increments @a *atomic atomically.
 * @param atomic Pointer to integer to increment.
 * @since 0.0.1
 */
#define p_atomic_int_inc(atomic) (p_atomic_int_add ((atomic), 1))

/**
 * @brief Decrements @a *atomic atomically.
 * @param atomic Pointer to integer to decrement.
 * @return TRUE if integer pointed by @a atomic is 0 after decrementing, FALSE otherwise.
 * @since 0.0.1
 */
#define p_atomic_int_dec_and_test(atomic)				\
  (p_atomic_int_exchange_and_add ((atomic), -1) == 1)

P_END_DECLS

#endif /* __P_ATOMIC_H__ */
