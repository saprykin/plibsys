/*
 * The MIT License
 *
 * Copyright (C) 2016 Alexander Saprykin <saprykin.spb@gmail.com>
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
 * @file pspinlock.h
 * @brief Light-weight atomic spinlock
 * @author Alexander Saprykin
 *
 * A spinlock is an inter-thread synchronization primitive based on atomic
 * operations. It allows to guard a critical section from concurrent access of
 * multiple threads at once. It is very similar to a mutex in semantics, but
 * inside it provides a more light-weight and fast locking mechanism without
 * thread sleeping and undesirable context switching. Thus spinlocks should be
 * used only for small code sections, otherwise long-time spinning can cause
 * extensive CPU time waste by waiting threads.
 *
 * As the spinlock is based on atomic operations it would have the real meaning
 * only if an underlying atomic model is lock-free (not simulated using the
 * mutex). You can check if the atomic model is lock-free with
 * p_atomic_is_lock_free(). Otherwise usage of spinlocks will be the same as the
 * ordinary mutex.
 *
 * To create a new spinlock primitive the p_spinlock_new() routine should be
 * called, to delete the unused spinlock primitive use p_spinlock_free().
 *
 * Use p_spinlock_lock() or p_spinlock_trylock() to synchronize access at the
 * beginning of the critical section. Only the one thread is allowed to pass
 * this call, others will wait for the p_spinlock_unlock() call which marks the
 * end of the critical section. This way the critical section code is guarded
 * against concurrent access of multiple threads at once.
 */

#if !defined (PLIBSYS_H_INSIDE) && !defined (PLIBSYS_COMPILATION)
#  error "Header files shouldn't be included directly, consider using <plibsys.h> instead."
#endif

#ifndef PLIBSYS_HEADER_PSPINLOCK_H
#define PLIBSYS_HEADER_PSPINLOCK_H

#include <pmacros.h>
#include <ptypes.h>

P_BEGIN_DECLS

/** Spinlock opaque data structure. */
typedef struct PSpinLock_ PSpinLock;

/**
 * @brief Creates a new #PSpinLock object.
 * @return Pointer to a newly created #PSpinLock object.
 * @since 0.0.1
 */
P_LIB_API PSpinLock *	p_spinlock_new		(void);

/**
 * @brief Locks a spinlock.
 * @param spinlock #PSpinLock to lock.
 * @return TRUE in case of success, FALSE otherwise.
 * @since 0.0.1
 *
 * A thread will not sleep in this call if another thread is holding the lock,
 * instead it will try to lock @a spinlock in an infinite loop.
 *
 * If the atomic model is not lock-free this call will have the same effect
 * as p_mutex_lock().
 *
 * Do not lock a spinlock recursively - this may lead to an application
 * deadlock.
 */
P_LIB_API pboolean	p_spinlock_lock		(PSpinLock *spinlock);

/**
 * @brief Tries to lock a spinlock immediately.
 * @param spinlock #PSpinLock to lock.
 * @return TRUE in case of success, FALSE otherwise.
 * @since 0.0.1
 *
 * Tries to lock @a spinlock and returns immediately if it is not available for
 * locking.
 *
 * If the atomic model is not lock-free this call will have the same effect
 * as p_mutex_trylock().
 *
 * Do not lock a spinlock recursively - this may lead to an application
 * deadlock.
 */
P_LIB_API pboolean	p_spinlock_trylock	(PSpinLock *spinlock);

/**
 * @brief Releases a locked spinlock.
 * @param spinlock #PSpinLock to release.
 * @return TRUE in case of success, FALSE otherwise.
 * @since 0.0.1
 *
 * If @a spinlock was previously locked then it becomes unlocked. Any thread
 * can unlock any spinlock. It is also safe to call this routine on an unlocked
 * spinlock.
 *
 * If the atomic model is not lock-free this call will have the same effect
 * as p_mutex_unlock(), thus it is not safe to call this routine on an unlocked
 * spinlock.
 */
P_LIB_API pboolean	p_spinlock_unlock	(PSpinLock *spinlock);

/**
 * @brief Frees #PSpinLock object.
 * @param spinlock #PSpinLock to free.
 * @since 0.0.1
 *
 * It doesn't unlock @a spinlock before freeing memory, so you should do it
 * manually.
 *
 * If the atomic model is not lock-free this call will have the same effect
 * as p_mutex_free().
 */
P_LIB_API void		p_spinlock_free		(PSpinLock *spinlock);

P_END_DECLS

#endif /* PLIBSYS_HEADER_PSPINLOCK_H */
