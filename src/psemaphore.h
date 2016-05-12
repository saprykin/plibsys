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
 * @file psemaphore.h
 * @brief Semaphore routines
 * @author Alexander Saprykin
 *
 * Semaphore is a synchronization primitive which controls access to shared data
 * from the concurrently running threads. Unlike a mutex (which is a particular
 * case of binary semaphore) it allows concurrent access not to only one thread.
 *
 * Semaphore has a counter which means a number of available resources (units).
 * Before entering a critical section thread must perform the so called P
 * (acquire) operation: if counter is positive it decrements the counter by 1
 * and continues execution; otherwise thread is suspended until the counter
 * becomes positive. Before leaving the critical section thread must perform the
 * so called V (release) operation: increments counter by 1 and wakes up a
 * waiting thread from the queue (if any).
 *
 * You can think about semaphore as a resource controller: P operation takes one
 * unit, while V operation gives one unit back. Thread could not continue
 * execution without taking a resource unit. By setting initial semaphore
 * counter value you can control how much concurrent threads can work with a
 * shared resource.
 *
 * This semaphore implementation is process-wide so you can synchronize not only
 * the threads. But it makes this IPC primitive (actually like any other IPC
 * primitive, as well) relatively heavy. Consider using mutex or spinlock
 * instead if you do not need to cross process boundary.
 *
 * Process-wide semaphore is identified by its name across the system, thus it
 * is also called named semaphore. Use p_semaphore_new() to open a named
 * semaphore and p_semaphore_free() to close it.
 *
 * Please note the following platform specific differences:
 *
 * - Windows doesn't own IPC objects (processes own them), which means that a
 * semaphore will be removed from the system after the last process or thread
 * closes it (or after terminating all the processes and threads holding opened
 * semaphore).
 *
 * - UNIX systems own IPC objects. Because of that UNIX IPC objects can survive
 * application crash: already used semaphore can be opened in a locked state and
 * application can fail into a deadlock or an inconsistent state. This could
 * happen if you have not closed all the opened semaphores explicitly before
 * terminating the application.
 *
 * - IRIX allows to open several instances of a semaphore within the single
 * process, but it will close the object after the first close call from any of
 * the threads within the process.
 *
 * Use third argument as #P_SEM_ACCESS_CREATE in p_semaphore_new() to reset a
 * semaphore value while openning it. This argument is ignored on Windows. You
 * can also take ownership of a semaphore with p_semaphore_take_ownership() to
 * explicitly remove it from the system after closing.
 */

#if !defined (__PLIBSYS_H_INSIDE__) && !defined (PLIBSYS_COMPILATION)
#  error "Header files shouldn't be included directly, consider using <plibsys.h> instead."
#endif

#ifndef __PSEMAPHORE_H__
#define __PSEMAPHORE_H__

#include <pmacros.h>
#include <ptypes.h>
#include <perror.h>

P_BEGIN_DECLS

/** Enum with semaphore creation modes. */
typedef enum _PSemaphoreAccessMode {
	P_SEM_ACCESS_OPEN	= 0,	/**< Open existing semaphore or create one with a given value.	*/
	P_SEM_ACCESS_CREATE	= 1	/**< Create semaphore, reset to a given value if exists.	*/
} PSemaphoreAccessMode;

/** Semaphore opaque data structure. */
typedef struct _PSemaphore PSemaphore;

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
 * @a init_val used only in one of following cases: semaphore with the such name
 * doesn't exist, or semaphore with the such name exists but @a mode specified
 * as #P_SEM_ACCESS_CREATE (non-Windows platforms only). In other cases
 * @a init_val is ignored. @a name is system-wide, so any other process can open
 * that semaphore passing the same name.
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
 * systems where semaphore can survive an application crash. On Windows platform
 * this call has no effect.
 *
 * The common usage of this call is upon application startup to ensure that
 * semaphore from the previous crash will be unlinked from the system. To do
 * that, call p_semaphore_new(), take ownership of the semaphore object and
 * remove it with p_semaphore_free() call. After that, create it again.
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
 * It doesn't release acquired semaphore, be careful to not to make a deadlock
 * while removing acquired semaphore.
 */
P_LIB_API void		p_semaphore_free		(PSemaphore		*sem);

P_END_DECLS

#endif /* __PSEMAPHORE_H__ */
