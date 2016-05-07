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
 * @file pmutex.h
 * @brief Mutex routines
 * @author Alexander Saprykin
 *
 * Mutex is a mutual exclusive (hence mutex) synchronization primitive which
 * allows access to critical section only to one of the concurrently running
 * threads. It is used to protected shared data structures from concurrent
 * modifications which could lead to unpredictable behavior.
 *
 * When entering a critical section thread must call p_mutex_lock() to get a
 * lock. If another thread is already holding the lock all other threads will
 * be suspended until the lock is released with p_mutex_unlock(). After
 * releasing the lock one of the waiting threads is resumed to continue
 * execution. On most systems it is not specified whether a mutex waiting queue
 * is fair (FIFO) on not.
 *
 * The typical mutex usage:
 * @code
 * p_mutex_lock (mutex);
 *
 * ... code in critical section ...
 *
 * p_mutex_unlock (mutex);
 * @endcode
 * You can also think of a mutex as a binary semaphore.
 *
 * It is implementation dependent whether recursive locking or non-locked mutex
 * unlocking is allowed, but such actions can lead to unpredictable behavior.
 * Do not rely on such behavior in cross-platform applications.
 */

#if !defined (__PLIBSYS_H_INSIDE__) && !defined (PLIBSYS_COMPILATION)
#  error "Header files shouldn't be included directly, consider using <plibsys.h> instead."
#endif

#ifndef __PMUTEX_H__
#define __PMUTEX_H__

#include <pmacros.h>
#include <ptypes.h>

P_BEGIN_DECLS

/** Mutex opaque data structure */
typedef struct _PMutex PMutex;

/**
 * @brief Creates a new #PMutex object.
 * @return Pointer to a newly created #PMutex object.
 * @since 0.0.1
 */
P_LIB_API PMutex *	p_mutex_new	(void);

/**
 * @brief Locks a mutex.
 * @param mutex #PMutex to lock.
 * @return TRUE in case of success, FALSE otherwise.
 * @since 0.0.1
 * @warning Do not lock the mutex recursively - it may lead to application
 * deadlock (implementation dependent).
 *
 * Forces calling thread to sleep until @a mutex becomes available for locking.
 */
P_LIB_API pboolean	p_mutex_lock	(PMutex *mutex);

/**
 * @brief Tries to lock a mutex immediately.
 * @param mutex #PMutex to lock.
 * @return TRUE in case of success, FALSE otherwise.
 * @since 0.0.1
 * @warning Do not lock the mutex recursively - it may lead to application
 * deadlock (implementation dependent).
 *
 * Tries to lock @a mutex and returns immediately if it is not available for
 * locking.
 */
P_LIB_API pboolean	p_mutex_trylock	(PMutex *mutex);

/**
 * @brief Releases a locked mutex.
 * @param mutex #PMutex to release.
 * @return TRUE in case of success, FALSE otherwise.
 * @since 0.0.1
 * @warning Do not use this function on non-locked mutexes - behavior may be
 * unpredictable.
 *
 * If @a mutex was previously locked then it becomes unlocked.
 *
 * It's implementation dependent whether only the same thread can lock and
 * unlock the same mutex.
 */
P_LIB_API pboolean	p_mutex_unlock	(PMutex *mutex);

/**
 * @brief Frees #PMutex object.
 * @param mutex #PMutex to free.
 * @since 0.0.1
 * @warning It doesn't unlock @a mutex before freeing memory, so you should do
 * it manually.
 */
P_LIB_API void		p_mutex_free	(PMutex *mutex);

P_END_DECLS

#endif /* __PMUTEX_H__ */
