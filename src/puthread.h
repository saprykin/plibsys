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
 * @file puthread.h
 * @brief Multithreading support
 * @author Alexander Saprykin
 *
 * Thread is a system execution unit which is managed independently by a
 * scheduler of the operating system. It allows to do things in parallel or
 * concurrently.
 *
 * #PUThread provides a convinient way of multithreading support using native
 * routines to provide best performance on a target system.
 *
 * To create a thread use p_uthread_create() or p_uthread_create_full() routines.
 * Joinable threads allow to wait until their execution is finished before
 * proceeding further. Thus you can synchronize threads execution within the
 * main thread.
 *
 * Priorities (if supported) allow to tune scheduler behavior: threads with
 * higher priority will be executed more frequenlty. Be careful that improper
 * priorities may lead to negative effects when some threads may receive almost
 * zero execution time. Not all operating systems respect thread priorities in
 * favour of process ones.
 *
 * To put a current thread (even if it was not created using #PUThread routines)
 * in sleep state use p_uthread_sleep().
 *
 * You can give a hint to a scheduler that a current thread do not need an
 * execution time with p_uthread_yield() routine. This is useful when some of
 * the threads are in the idle state so you do not want to waste a CPU time.
 * This only tells to a scheduler to skip current scheduling cycle for the
 * calling thread, though the scheduler can ingnore it.
 */

#if !defined (__PLIBSYS_H_INSIDE__) && !defined (PLIBSYS_COMPILATION)
#  error "Header files shouldn't be included directly, consider using <plibsys.h> instead."
#endif

#ifndef __PUTHREAD_H__
#define __PUTHREAD_H__

#include <pmacros.h>
#include <ptypes.h>

P_BEGIN_DECLS

/** Typedef for #PUThread running method */
typedef ppointer (*PUThreadFunc) (ppointer arg);

/** #PUThread opaque data type */
typedef struct _PUThread PUThread;

/** Thread priority */
typedef enum _PUThreadPriority {
	P_UTHREAD_PRIORITY_LOWEST	= 0,	/**< Lowest possible priority.					*/
	P_UTHREAD_PRIORITY_LOW		= 1,	/**< Low priority (min_prio * 6 + normal_prio * 4) / 10.	*/
	P_UTHREAD_PRIORITY_NORMAL	= 2,	/**< Normal priority (scheduler's priority).			*/
	P_UTHREAD_PRIORITY_HIGH		= 3,	/**< High priority ((normal_prio + max_prio * 2) / 3).		*/
	P_UTHREAD_PRIORITY_HIGHEST	= 4	/**< Highest possible priority.					*/
} PUThreadPriority;

/**
 * @brief Creates a new #PUThread and starts it.
 * @param func Main thread function to run.
 * @param data Pointer to pass into the thread main function, may be NULL.
 * @param joinable Whether to create joinable thread or not.
 * @param prio Thread priority.
 * @return Pointer to #PUThread in case of success, NULL otherwise.
 * @since 0.0.1
 */
P_LIB_API PUThread *		p_uthread_create_full	(PUThreadFunc		func,
							 ppointer		data,
							 pboolean		joinable,
							 PUThreadPriority	prio);

/**
 * @brief Creates a #PUThread and starts it. Short version of p_uthread_create_full().
 * @param func Main thread function to run.
 * @param data Pointer to pass into the thread main function, may be NULL.
 * @param joinable Whether to create joinable thread or not.
 * @return Pointer to #PUThread in case of success, NULL otherwise.
 * @since 0.0.1
 */
P_LIB_API PUThread *		p_uthread_create	(PUThreadFunc		func,
							 ppointer		data,
							 pboolean		joinable);

/**
 * @brief Exits from the currently running (caller) thread.
 * @param code Exit code.
 * @since 0.0.1
 */
P_LIB_API P_NO_RETURN void	p_uthread_exit		(pint			code);

/**
 * @brief Waits for selected thread to become finished. Thread must be joinable
 * to return non-negative result.
 * @param thread Thread to wait for.
 * @return Thread exit code in case of success, -1 otherwise.
 * @since 0.0.1
 */
P_LIB_API pint			p_uthread_join		(PUThread		*thread);

/**
 * @brief Frees #PUThread and it's resources.
 * @param thread #PUThread to free.
 * @note Freeing the running thread can lead to unpredictable behavior.
 * @since 0.0.1
 */
P_LIB_API void			p_uthread_free		(PUThread		*thread);

/**
 * @brief Sleeps the current thread (caller) for specified amount of time.
 * @param msec Milliseconds to sleep.
 * @return 0 in case of success, -1 otherwise.
 * @since 0.0.1
 */
P_LIB_API pint			p_uthread_sleep		(puint32		msec);

/**
 * @brief Sets thread priority.
 * @param thread Thread to set priority for.
 * @param prio Priority to set.
 * @return 0 in case of success, -1 otherwise.
 * @since 0.0.1
 */
P_LIB_API pint			p_uthread_set_priority	(PUThread		*thread,
							 PUThreadPriority	prio);

/**
 * @brief Tells scheduler to skip the current (caller) thread in the current
 * planning stage. In other words, scheduler shouldn't give time ticks for the
 * current thread during the current period.
 * @since 0.0.1
 */
P_LIB_API void			p_uthread_yield		(void);

P_END_DECLS

#endif /* __PUTHREAD_H__ */
