/*
 * The MIT License
 *
 * Copyright (C) 2010-2017 Alexander Saprykin <saprykin.spb@gmail.com>
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
 * @file psemaphore.h
 * @brief Semaphore routines
 * @author Alexander Saprykin
 *
 * A semaphore is a synchronization primitive which controls access to shared
 * data from the concurrently running threads. Unlike a mutex (which is a
 * particular case of a binary semaphore) it allows concurrent access not to
 * only the one thread.
 *
 * The semaphore has a counter which means the number of available resources
 * (units). Before entering a critical section a thread must perform the so
 * called P (acquire) operation: if the counter is positive it decrements the
 * counter by 1 and continues execution; otherwise the thread is suspended until
 * the counter becomes positive. Before leaving the critical section the thread
 * must perform the so called V (release) operation: increments the counter by 1
 * and wakes up a waiting thread from the queue (if any).
 *
 * You can think about the semaphore as a resource controller: the P operation
 * takes one unit, while the V operation gives one unit back. The thread could
 * not continue execution without taking a resource unit. By setting the initial
 * semaphore counter value you can control how much concurrent threads can work
 * with a shared resource.
 *
 * This semaphore implementation is process-wide so you can synchronize not only
 * the threads. But it makes this IPC primitive (actually like any other IPC
 * primitive, as well) relatively heavy. Consider using a mutex or a spinlock
 * instead if you do not need to cross a process boundary.
 *
 * A process-wide semaphore is identified by its name across the system, thus it
 * is also called a named semaphore. Use p_semaphore_new() to open the named
 * semaphore and p_semaphore_free() to close it.
 *
 * Please note the following platform specific differences:
 *
 * - Windows doesn't own IPC objects (processes own them), which means that a
 * semaphore will be removed from the system after the last process or thread
 * closes it (or after terminating all the processes and threads holding open
 * semaphore).
 *
 * - UNIX systems own IPC objects. Because of that UNIX IPC objects can survive
 * an application crash: an already used semaphore can be opened in a locked
 * state and an application can fail into a deadlock or an inconsistent state.
 * This could happen if you have not closed all the open semaphores explicitly
 * before terminating the application.
 *
 * - IRIX allows to open several instances of a semaphore within the single
 * process, but it will close the object after the first close call from any of
 * the threads within the process.
 *
 * - AmigaOS has process-wide semaphores without actual tracking of counter,
 * which means that semaphore behaves the same way as recursive mutex.
 *
 * - OpenVMS (as of 8.4 release) has declared prototypes for process-wide named
 * semaphores but the actual implementation is broken.
 *
 * - OS/2 lacks support for process-wide named semaphores.
 *
 * - Syllable lacks support for process-wide named semaphores.
 *
 * - BeOS lacks support for process-wide named semaphores.
 *
 * Use the third argument as #P_SEM_ACCESS_CREATE in p_semaphore_new() to reset
 * a semaphore value while opening it. This argument is ignored on Windows. You
 * can also take ownership of the semaphore with p_semaphore_take_ownership() to
 * explicitly remove it from the system after closing.
 */

#if !defined (PLIBSYS_H_INSIDE) && !defined (PLIBSYS_COMPILATION)
#  error "Header files shouldn't be included directly, consider using <plibsys.h> instead."
#endif

#ifndef PLIBSYS_HEADER_PSEMAPHORE_H
#define PLIBSYS_HEADER_PSEMAPHORE_H

#include <pmacros.h>
#include <ptypes.h>
#include <perror.h>

P_BEGIN_DECLS

/** Enum with semaphore creation modes. */
typedef enum PSemaphoreAccessMode_ {
	P_SEM_ACCESS_OPEN	= 0,	/**< Opens an existing semaphore or creates one with a given value.	*/
	P_SEM_ACCESS_CREATE	= 1	/**< Creates semaphore, resets to a given value if exists.		*/
} PSemaphoreAccessMode;

/** Semaphore opaque data structure. */
typedef struct PSemaphore_ PSemaphore;

/**
 * @brief Creates a new #PSemaphore object.
 * @param name Semaphore name.
 * @param init_val Initial semaphore value.
 * @param mode Creation mode.
 * @param[out] error Error report object, NULL to ignore.
 * @return Pointer to a newly created #PSemaphore object in case of success,
 * NULL otherwise.
 * @since 0.0.1
 *
 * The @a init_val is used only in one of following cases: a semaphore with the
 * such name doesn't exist, or the semaphore with the such name exists but
 * @a mode specified as #P_SEM_ACCESS_CREATE (non-Windows platforms only). In
 * other cases @a init_val is ignored. The @a name is system-wide, so any other
 * process can open that semaphore passing the same name.
 */
P_LIB_API PSemaphore *	p_semaphore_new			(const pchar		*name,
							 pint			init_val,
							 PSemaphoreAccessMode	mode,
							 PError			**error);

/**
 * @brief Takes ownership of a semaphore.
 * @param sem Semaphore to take ownership.
 * @since 0.0.1
 *
 * If you take ownership of a semaphore object, p_semaphore_free() will try to
 * completely unlink it and remove from the system. This is useful on UNIX
 * systems where the semaphore can survive an application crash. On the Windows
 * platform this call has no effect.
 *
 * The common usage of this call is upon application startup to ensure that the
 * semaphore from the previous crash will be unlinked from the system. To do
 * that, call p_semaphore_new(), take ownership of the semaphore object and
 * remove it with the p_semaphore_free() call. After that, create it again.
 *
 * You can also do the same thing upon semaphore creation passing
 * #P_SEM_ACCESS_CREATE to p_semaphore_new(). The only difference is that you
 * should already know whether this semaphore object is from the previous crash
 * or not.
 */
P_LIB_API void		p_semaphore_take_ownership	(PSemaphore		*sem);

/**
 * @brief Acquires (P operation) a semaphore.
 * @param sem #PSemaphore to acquire.
 * @param[out] error Error report object, NULL to ignore.
 * @return TRUE in case of success, FALSE otherwise.
 * @since 0.0.1
 */
P_LIB_API pboolean	p_semaphore_acquire		(PSemaphore		*sem,
							 PError			**error);

/**
 * @brief Releases (V operation) a semaphore.
 * @param sem #PSemaphore to release.
 * @param[out] error Error report object, NULL to ignore.
 * @return TRUE in case of success, FALSE otherwise.
 * @since 0.0.1
 */
P_LIB_API pboolean	p_semaphore_release		(PSemaphore		*sem,
							 PError			**error);

/**
 * @brief Frees #PSemaphore object.
 * @param sem #PSemaphore to free.
 * @since 0.0.1
 *
 * It doesn't release an acquired semaphore, be careful to not to make a
 * deadlock while removing the acquired semaphore.
 */
P_LIB_API void		p_semaphore_free		(PSemaphore		*sem);

P_END_DECLS

#endif /* PLIBSYS_HEADER_PSEMAPHORE_H */
