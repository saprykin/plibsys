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
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
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
 * higher priority will be executed more frequently. Be careful that improper
 * priorities may lead to negative effects when some threads may receive almost
 * zero execution time.
 *
 * Thread priorities are unreliable: not all operating systems respect thread
 * priorities in favour of process ones. Priorities may be ignored for bound
 * threads (every thread bound to a kernel light-weight thread as 1:1), other
 * systems may require administrative privileges to change a thread priority
 * (i.e. Linux). Windows always respects thread priorities.
 *
 * To put a current thread (even if it was not created using #PUThread routines)
 * in sleep state use p_uthread_sleep().
 *
 * You can give a hint to a scheduler that a current thread do not need an
 * execution time with p_uthread_yield() routine. This is useful when some of
 * the threads are in the idle state so you do not want to waste a CPU time.
 * This only tells to a scheduler to skip current scheduling cycle for the
 * calling thread, though the scheduler can ingnore it.
 *
 * Thread local storage (TLS) is provided. A TLS key's value can be accessed
 * through a reference key defined as #PUThreadKey. TLS reference key is a some
 * sort of a token which has an associated value. But every thread has its own
 * token value though using the same token object.
 *
 * After creating a TLS reference key every thread can use it to access a
 * local-specific value. Use p_uthread_local_new() call to create a TLS
 * reference key and pass it to every thread which needs local-specific values.
 * You can also provide a destroy notification function which would be called
 * upon a TLS key removal which is usually performed on a thread exit.
 *
 * There are two calls to set a TLS key's value: p_uthread_set_local() and
 * p_uthread_replace_local(). The only difference is that former one calls
 * provided destroy notification function before replacing the old value.
 */

#if !defined (__PLIBSYS_H_INSIDE__) && !defined (PLIBSYS_COMPILATION)
#  error "Header files shouldn't be included directly, consider using <plibsys.h> instead."
#endif

#ifndef __PUTHREAD_H__
#define __PUTHREAD_H__

#include <pmacros.h>
#include <ptypes.h>

P_BEGIN_DECLS

/** Typedef for #PUThread running method. */
typedef ppointer (*PUThreadFunc) (ppointer arg);

/** Thread opaque data type. */
typedef struct _PUThread PUThread;

/** TLS key opaque data type. */
typedef struct _PUThreadKey PUThreadKey;

/** Thread priority. */
typedef enum _PUThreadPriority {
	P_UTHREAD_PRIORITY_INHERIT	= 0,	/**< Inherits the caller thread priority. Default priority.	*/
	P_UTHREAD_PRIORITY_IDLE		= 1,	/**< Scheduled only when no other threads are running.		*/
	P_UTHREAD_PRIORITY_LOWEST	= 2,	/**< Scheduled less often than #P_UTHREAD_PRIORITY_LOW.		*/
	P_UTHREAD_PRIORITY_LOW		= 3,	/**< Scheduled less often than #P_UTHREAD_PRIORITY_NORMAL.	*/
	P_UTHREAD_PRIORITY_NORMAL	= 4,	/**< Operating system's default priority.			*/
	P_UTHREAD_PRIORITY_HIGH		= 5,	/**< Scheduled more often than #P_UTHREAD_PRIORITY_NORMAL.	*/
	P_UTHREAD_PRIORITY_HIGHEST	= 6,	/**< Scheduled more often than #P_UTHREAD_PRIORITY_HIGH.	*/
	P_UTHREAD_PRIORITY_TIMECRITICAL	= 7	/**< Scheduled as often as possible.				*/
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
 * @brief Creates a #PUThread and starts it. Short version of
 * p_uthread_create_full().
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
 * @return TRUE in case of success, FALSE otherwise.
 * @since 0.0.1
 */
P_LIB_API pboolean		p_uthread_set_priority	(PUThread		*thread,
							 PUThreadPriority	prio);

/**
 * @brief Tells scheduler to skip the current (caller) thread in the current
 * planning stage.
 * @since 0.0.1
 *
 * Scheduler shouldn't give time ticks for the current thread during the current
 * period, but it may ignore this call.
 */
P_LIB_API void			p_uthread_yield		(void);

/**
 * @brief Gets an ID of the current (caller) thread.
 * @since 0.0.1
 *
 * This is a platform-dependent type. You shouldn't treat it as a number, it
 * only gives you a uniquer ID of the thread accross the system.
 */
P_LIB_API P_HANDLE		p_uthread_current_id	(void);

/**
 * @brief Create a new TLS reference key.
 * @param free_func TLS key destroy notification call, leave NULL if not need.
 * @return New TLS reference key in case of success, NULL otherwise.
 * @since 0.0.1
 */
P_LIB_API PUThreadKey *		p_uthread_local_new	(PDestroyFunc		free_func);

/**
 * @brief Frees TLS reference key.
 * @param key TLS reference key to free.
 * @since 0.0.1
 *
 * It doesn't remove a TLS key itself but only removes a reference used to
 * access a TLS slot.
 */
P_LIB_API void			p_uthread_local_free	(PUThreadKey		*key);

/**
 * @brief Gets a TLS value.
 * @param key TLS reference key to get the value for.
 * @return TLS value for the given key.
 * @since 0.0.1
 * @note This call may fail only in case of abnormal usage or program behavior,
 * NULL value will be returned to tolerance the failure.
 */
P_LIB_API ppointer		p_uthread_get_local	(PUThreadKey		*key);

/**
 * @brief Sets a TLS value.
 * @param key TLS reference key to set the value for.
 * @param value TLS value to set.
 * @since 0.0.1
 * @note This call may fail only in case of abnormal usage or program behavior.
 *
 * It doesn't call a destructor notification function provided with
 * p_uthread_local_new().
 */
P_LIB_API void			p_uthread_set_local	(PUThreadKey		*key,
							 ppointer		value);

/**
 * @brief Replaces a TLS value.
 * @param key TLS reference key to replace the value for.
 * @param value TLS value to set.
 * @since 0.0.1
 * @note This call may fail only in case of abnormal usage or program behavior.
 *
 * This call does perform a notification function provided with
 * p_uthread_local_new() on an old TLS value. This is the only difference with
 * p_uthread_set_local().
 */
P_LIB_API void			p_uthread_replace_local	(PUThreadKey		*key,
							 ppointer		value);

P_END_DECLS

#endif /* __PUTHREAD_H__ */
