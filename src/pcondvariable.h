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

/**
 * @file pcondvariable.h
 * @brief Conditional variable
 * @author Alexander Saprykin
 *
 * Conditional variable is an inter-thread synchronization primitive, often
 * used in classical 'producers-consumers' concurrent data access models.
 *
 * The main idea is to notify waiting thread(s) for some events before they
 * can enter critical section. Hence the name of the primitive: thread enters
 * critical section upon accomplished condition. Compare it with mutex where
 * thread enters critical section as soon as no one holds a lock.
 *
 * Several threads can be notified at once, but only one of them can enter
 * critical section. The order of threads in that case is implementation
 * dependent.
 *
 * As thread enters critical section upon condition it still requires a mutex
 * to guard its code against concurrent access of other threads. Mutex provided
 * in pair with conditional variable will be automatically locked on condition,
 * thread should unlock it explicitly after leaving the critical section. That
 * mutex is unlocked while waiting for a condition and should be locked prior
 * calling a condition waiting routine.
 *
 * The waiting thread behavior: create a new conditional variable with
 * p_cond_variable_new(), create and lock mutex before the critical section and
 * wait for a signal from another thread on this conditional variable
 * using p_cond_variable_wait().
 *
 * The signaling thread behavior: upon reaching event time emit a signal with
 * p_cond_variable_signal() to wake up a single waiting thread or
 * p_cond_variable_broadcast() to wake up all the waiting threads.
 *
 * After emitting the signal only the one thread will get locked mutex back to
 * continue executing critical section.
 *
 * It is implementation dependent whether a thread will receive a missed signal
 * (when notification from one thread was emitted prior another thread has been
 * called for waiting), so do not rely on this behavior.
 */

#if !defined (__PLIB_H_INSIDE__) && !defined (PLIB_COMPILATION)
#  error "Header files shouldn't be included directly, consider using <plib.h> instead."
#endif

#ifndef __PCONDVARIABLE_H__
#define __PCONDVARIABLE_H__

#include <pmacros.h>
#include <ptypes.h>
#include <pmutex.h>

P_BEGIN_DECLS

/** PCondVariable opaque data structure */
typedef struct _PCondVariable PCondVariable;

/**
 * @brief Creates new #PCondVariable.
 * @return Pointer to a newly created #PCondVariable structure, or NULL if
 * failed.
 * @since 0.0.1
 */
P_LIB_API PCondVariable *	p_cond_variable_new		(void);

/**
 * @brief Frees #PCondVariable structure.
 * @param cond Condtional variable to free.
 * @since 0.0.1
 */
P_LIB_API void			p_cond_variable_free		(PCondVariable	*cond);

/**
 * @brief Waits for a signal on a given conditional variable.
 * @param cond Conditional variable to wait on.
 * @param mutex Locked mutex which will remain locked after waiting.
 * @return TRUE on success, FALSE otherwise.
 * @since 0.0.1
 *
 * Calling thread will sleep until signal on @a cond arrived.
 */
P_LIB_API pboolean		p_cond_variable_wait		(PCondVariable	*cond,
								 PMutex		*mutex);

/**
 * @brief Emitts a signal on the given conditional variable for the one waiting
 * thread.
 * @param cond Conditional variable to emit signal on.
 * @return TRUE on success, FALSE otherwise.
 * @since 0.0.1
 *
 * After emitting a signal only the one thread waiting for it will be waken up.
 * Do not rely on queue concept for waiting threads. Though implementation is
 * intended to be much close to queue, it's not fairly enough. Due that any
 * thread can be waken up, even if it just called p_cond_variable_wait() while
 * there were other waiting threads.
 */
P_LIB_API pboolean		p_cond_variable_signal		(PCondVariable	*cond);

/**
 * @brief Emitts a signal on the given conditional variable for all the waiting
 * threads.
 * @param cond Conditional variable to emit signal on.
 * @return TRUE on success, FALSE otherwise.
 * @since 0.0.1
 *
 * After emitting a signal all the threads waiting for it will be waken up.
 */
P_LIB_API pboolean		p_cond_variable_broadcast	(PCondVariable	*cond);

P_END_DECLS

#endif /* __PCONDVARIABLE_H__ */
