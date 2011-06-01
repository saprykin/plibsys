/* 
 * 07.04.2011
 * Copyright (C) 2010-2011 Alexander Saprykin <xelfium@gmail.com>
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
 * @brief Conditional variables
 * @author Alexander Saprykin
 *
 * Conditional variable are some sort of synchronization primitives used for threads.
 * The basic usage follows: create a new conditional variable with p_cond_variable_new(),
 * then in thread you can wait for signal from another thread on this conditional variable
 * using p_cond_variable_wait() and passing mutex there. Mutex should be locked, or
 * behaviour can be unpredictable. After that thread will be slept until signal on this
 * conditional variabl will not be emitted by another thread using p_cond_variable_signal()
 * or p_cond_variable_broadcast(). As soon as waked up, thread already holds mutex locked
 * and can work with shared resources. For additional information see other sources about
 * conditional variables concept.
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
 * @return Pointer to newly created #PCondVariable structure, or NULL if failed.
 * @since 0.0.1
 */
P_LIB_API PCondVariable *	p_cond_variable_new		(void);

/**
 * @brief Frees #PCondVariable structure.
 * @param cond Condtional variable to free.
 * @since 0.0.1
 */
P_LIB_API void			p_cond_variable_free		(PCondVariable *cond);

/**
 * @brief Waits for signal on given conditional variable.
 * @param cond Conditional variable to wait on.
 * @param mutex Locked mutex which will remain locked after waiting.
 * @return 0 on success, -1 otherwise.
 * @since 0.0.1
 *
 * Calling thread will sleep until signal on @a cond arrived.
 */
P_LIB_API pint			p_cond_variable_wait		(PCondVariable *cond, PMutex *mutex);

/**
 * @brief Emits signal on given conditional variable for one waiting thread.
 * @param cond Conditional variable to emit signal on.
 * @return 0 on success, -1 otherwise. 
 * @since 0.0.1
 *
 * After emitting signal only one thread waiting for it will be waken up. Do not
 * rely on queue concept for waiting threads. Though implementation is intended to
 * be much close to queue, it's not fairly enough. Due that any thread can be waken
 * up, even if it just called p_cond_variable_wait() while there were another waiting
 * threads.
 */
P_LIB_API pint			p_cond_variable_signal		(PCondVariable *cond);

/**
 * @brief Emits signal on given conditional variable for all waiting threads.
 * @param cond Conditional variable to emit signal on.
 * @return 0 on success, -1 otherwise. 
 * @since 0.0.1
 *
 * After emitting signal all threads waiting for it will be waken up.
 */
P_LIB_API pint			p_cond_variable_broadcast	(PCondVariable *cond);

P_END_DECLS

#endif /* __PCONDVARIABLE_H__ */

