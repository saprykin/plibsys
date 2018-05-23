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
 * @file pmutex.h
 * @brief Mutex routines
 * @author Alexander Saprykin
 *
 * A mutex is a mutual exclusive (hence mutex) synchronization primitive which
 * allows access to a critical section only to one of the concurrently running
 * threads. It is used to protected shared data structures from concurrent
 * modifications which could lead to unpredictable behavior.
 *
 * When entering a critical section a thread must call p_mutex_lock() to get a
 * lock. If another thread is already holding the lock all other threads will
 * be suspended until the lock is released with p_mutex_unlock(). After
 * releasing the lock one of the waiting threads is resumed to continue
 * execution. On most systems it is not specified whether a mutex waiting queue
 * is fair (FIFO) or not.
 *
 * The typical mutex usage:
 * @code
 * p_mutex_lock (mutex);
 *
 * ... code in critical section ...
 *
 * p_mutex_unlock (mutex);
 * @endcode
 * You can also think of the mutex as a binary semaphore.
 *
 * It is implementation dependent whether recursive locking or non-locked mutex
 * unlocking is allowed, but such actions can lead to unpredictable behavior.
 * Do not rely on such behavior in cross-platform applications.
 *
 * This is the thread scoped mutex implementation. You could not share this
 * mutex outside the process adress space, but you can share it between the
 * threads of the same process.
 */

#if !defined (PLIBSYS_H_INSIDE) && !defined (PLIBSYS_COMPILATION)
#  error "Header files shouldn't be included directly, consider using <plibsys.h> instead."
#endif

#ifndef PLIBSYS_HEADER_PMUTEX_H
#define PLIBSYS_HEADER_PMUTEX_H

#include <pmacros.h>
#include <ptypes.h>

P_BEGIN_DECLS

/** Mutex opaque data structure. */
typedef struct PMutex_ PMutex;

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
 * @warning Do not lock the mutex recursively - it may lead to an application
 * deadlock (implementation dependent).
 *
 * Forces the calling thread to sleep until @a mutex becomes available for
 * locking.
 */
P_LIB_API pboolean	p_mutex_lock	(PMutex *mutex);

/**
 * @brief Tries to lock a mutex immediately.
 * @param mutex #PMutex to lock.
 * @return TRUE in case of success, FALSE otherwise.
 * @since 0.0.1
 * @warning Do not lock the mutex recursively - it may lead to an application
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

#endif /* PLIBSYS_HEADER_PMUTEX_H */
