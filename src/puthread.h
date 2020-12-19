/*
 * The MIT License
 *
 * Copyright (C) 2010-2019 Alexander Saprykin <saprykin.spb@gmail.com>
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
 * @file puthread.h
 * @brief Multithreading support
 * @author Alexander Saprykin
 *
 * A thread is a system execution unit which is managed independently by the
 * scheduler of the operating system. It allows to do things in parallel or
 * concurrently.
 *
 * #PUThread provides a convinient way of multithreading support using native
 * routines to provide the best performance on the target system.
 *
 * To create the thread use the p_uthread_create() or p_uthread_create_full()
 * routines. Joinable threads allow to wait until their execution is finished
 * before proceeding further. Thus you can synchronize threads' execution within
 * the main thread.
 *
 * A reference counter mechanism is used to keep track of a #PUThread structure.
 * It means that the structure will be freed automatically when the reference
 * counter becomes zero. Use p_uthread_ref() to hold the structure and
 * p_uthread_unref() to decrement the counter back. A running thread holds a
 * reference to itself structure, so you do not require to hold a reference
 * to the thread while it is running.
 *
 * Priorities (if supported) allow to tune scheduler behavior: threads with
 * higher priority will be executed more frequently. Be careful that improper
 * priorities may lead to negative effects when some threads may receive almost
 * zero execution time.
 *
 * Thread priorities are unreliable: not all operating systems respect thread
 * priorities in favour of process ones. Priorities may be ignored for bound
 * threads (every thread bound to a kernel light-weight thread as 1:1), other
 * systems may require administrative privileges to change the thread priority
 * (i.e. Linux). Windows always respects thread priorities.
 *
 * To put the current thread (even if it was not created using the #PUThread
 * routines) in a sleep state use p_uthread_sleep().
 *
 * You can give a hint to the scheduler that the current thread do not need an
 * execution time with the p_uthread_yield() routine. This is useful when some
 * of the threads are in an idle state so you do not want to waste a CPU time.
 * This only tells to the scheduler to skip the current scheduling cycle for the
 * calling thread, though the scheduler can ingnore it.
 *
 * A thread local storage (TLS) is provided. The TLS key's value can be accessed
 * through a reference key defined as a #PUThreadKey. A TLS reference key is
 * some sort of a token which has an associated value. But every thread has its
 * own token value though using the same token object.
 *
 * After creating the TLS reference key every thread can use it to access a
 * local-specific value. Use the p_uthread_local_new() call to create the TLS
 * reference key and pass it to every thread which needs local-specific values.
 * You can also provide a destroy notification function which would be called
 * upon a TLS key removal which is usually performed on the thread exit.
 *
 * There are two calls to set a TLS key's value: p_uthread_set_local() and
 * p_uthread_replace_local(). The only difference is that the former one calls
 * the provided destroy notification function before replacing the old value.
 *
 * Thread names are used on most of operating systems for debugging purposes,
 * thereby some limitations for long name can be applied and too long names
 * will be truncated automatically.
 */

#if !defined (PLIBSYS_H_INSIDE) && !defined (PLIBSYS_COMPILATION)
#  error "Header files shouldn't be included directly, consider using <plibsys.h> instead."
#endif

#ifndef PLIBSYS_HEADER_PUTHREAD_H
#define PLIBSYS_HEADER_PUTHREAD_H

#include <pmacros.h>
#include <ptypes.h>

P_BEGIN_DECLS

/** Typedef for a #PUThread running method. */
typedef ppointer (*PUThreadFunc) (ppointer arg);

/** Thread opaque data type. */
typedef struct PUThread_ PUThread;

/** TLS key opaque data type. */
typedef struct PUThreadKey_ PUThreadKey;

/** Thread priority. */
typedef enum PUThreadPriority_ {
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
 * @param joinable Whether to create a joinable thread or not.
 * @param prio Thread priority.
 * @param stack_size Thread stack size, in bytes. Leave zero to use a default
 * value.
 * @param name Thread name, maybe NULL.
 * @return Pointer to #PUThread in case of success, NULL otherwise.
 * @since 0.0.1
 * @note Unreference the returned value after use with p_uthread_unref(). You do
 * not need to call p_uthread_ref() explicitly on the returned value.
 */
P_LIB_API PUThread *	p_uthread_create_full	(PUThreadFunc		func,
						 ppointer		data,
						 pboolean		joinable,
						 PUThreadPriority	prio,
						 psize			stack_size,
						 const pchar		*name);

/**
 * @brief Creates a #PUThread and starts it. A short version of
 * p_uthread_create_full().
 * @param func Main thread function to run.
 * @param data Pointer to pass into the thread main function, may be NULL.
 * @param joinable Whether to create a joinable thread or not.
 * @param name Thread name, maybe NULL.
 * @return Pointer to #PUThread in case of success, NULL otherwise.
 * @since 0.0.1
 * @note Unreference the returned value after use with p_uthread_unref(). You do
 * not need to call p_uthread_ref() explicitly on the returned value.
 */
P_LIB_API PUThread *	p_uthread_create	(PUThreadFunc		func,
						 ppointer		data,
						 pboolean		joinable,
						 const pchar		*name);

/**
 * @brief Exits from the currently running (caller) thread.
 * @param code Exit code.
 * @since 0.0.1
 */
P_LIB_API void		p_uthread_exit		(pint			code);

/**
 * @brief Waits for the selected thread to become finished.
 * @param thread Thread to wait for.
 * @return Thread exit code in case of success, -1 otherwise.
 * @since 0.0.1
 * @note Thread must be joinable to return the non-negative result.
 */
P_LIB_API pint		p_uthread_join		(PUThread		*thread);

/**
 * @brief Sleeps the current thread (caller) for a specified amount of time.
 * @param msec Milliseconds to sleep.
 * @return 0 in case of success, -1 otherwise.
 * @since 0.0.1
 */
P_LIB_API pint		p_uthread_sleep		(puint32		msec);

/**
 * @brief Sets a thread priority.
 * @param thread Thread to set the priority for.
 * @param prio Priority to set.
 * @return TRUE in case of success, FALSE otherwise.
 * @since 0.0.1
 */
P_LIB_API pboolean	p_uthread_set_priority	(PUThread		*thread,
						 PUThreadPriority	prio);

/**
 * @brief Tells the scheduler to skip the current (caller) thread in the current
 * planning stage.
 * @since 0.0.1
 *
 * The scheduler shouldn't give time ticks for the current thread during the
 * current period, but it may ignore this call.
 */
P_LIB_API void		p_uthread_yield		(void);

/**
 * @brief Gets an ID of the current (caller) thread.
 * @return The ID of the current thread.
 * @since 0.0.1
 *
 * This is a platform-dependent type. You shouldn't treat it as a number, it
 * only gives you the uniquer ID of the thread accross the system.
 */
P_LIB_API P_HANDLE	p_uthread_current_id	(void);

/**
 * @brief Gets a thread structure of the current (caller) thread.
 * @return The thread structure of the current thread.
 * @since 0.0.1
 * @note This call doesn't not increment the reference counter of the returned
 * structure.
 *
 * A thread structure will be returned even for the thread which was created
 * outside the library. But you should not use thread manipulation routines over
 * that structure.
 */
P_LIB_API PUThread *	p_uthread_current	(void);

/**
 * @brief Gets the ideal number of threads for the system based on the number of
 * avaialble CPUs and cores (physical and logical).
 * @return Ideal number of threads, 1 in case of failed detection.
 * @since 0.0.3
 */
P_LIB_API pint		p_uthread_ideal_count	(void);

/**
 * @brief Increments a thread reference counter
 * @param thread #PUThread to increment the reference counter.
 * @since 0.0.1
 * @note The #PUThread object will not be removed until the reference counter is
 * positive.
 */
P_LIB_API void		p_uthread_ref		(PUThread		*thread);

/**
 * @brief Decrements a thread reference counter
 * @param thread #PUThread to decrement the reference counter.
 * @since 0.0.1
 * @note When the reference counter becomes zero the #PUThread is removed from
 * the memory.
 */
P_LIB_API void		p_uthread_unref		(PUThread		*thread);

/**
 * @brief Create a new TLS reference key.
 * @param free_func TLS key destroy notification call, leave NULL if not need.
 * @return New TLS reference key in case of success, NULL otherwise.
 * @since 0.0.1
 */
P_LIB_API PUThreadKey *	p_uthread_local_new	(PDestroyFunc		free_func);

/**
 * @brief Frees a TLS reference key.
 * @param key TLS reference key to free.
 * @since 0.0.1
 *
 * It doesn't remove the TLS key itself but only removes a reference used to
 * access the TLS slot.
 */
P_LIB_API void		p_uthread_local_free	(PUThreadKey		*key);

/**
 * @brief Gets a TLS value.
 * @param key TLS reference key to get the value for.
 * @return TLS value for the given key.
 * @since 0.0.1
 * @note This call may fail only in case of abnormal use or program behavior,
 * the NULL value will be returned to tolerance the failure.
 */
P_LIB_API ppointer	p_uthread_get_local	(PUThreadKey		*key);

/**
 * @brief Sets a TLS value.
 * @param key TLS reference key to set the value for.
 * @param value TLS value to set.
 * @since 0.0.1
 * @note This call may fail only in case of abnormal use or program behavior.
 *
 * It doesn't call the destructor notification function provided with
 * p_uthread_local_new().
 */
P_LIB_API void		p_uthread_set_local	(PUThreadKey		*key,
						 ppointer		value);

/**
 * @brief Replaces a TLS value.
 * @param key TLS reference key to replace the value for.
 * @param value TLS value to set.
 * @since 0.0.1
 * @note This call may fail only in case of abnormal use or program behavior.
 *
 * This call does perform the notification function provided with
 * p_uthread_local_new() on the old TLS value. This is the only difference with
 * p_uthread_set_local().
 */
P_LIB_API void		p_uthread_replace_local	(PUThreadKey		*key,
						 ppointer		value);

P_END_DECLS

#endif /* PLIBSYS_HEADER_PUTHREAD_H */
