/*
 * Copyright (C) 2016 Alexander Saprykin <xelfium@gmail.com>
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
 * @file pspinlock.h
 * @brief Light-weight atomic spinlock
 * @author Alexander Saprykin
 *
 * Spinlock is an inter-thread synchronization primitive based on atomic
 * operations. It allows to guard critical section from concurrent access
 * from multiple threads at once. It is very similar to mutex in semantics,
 * but inside it provides more light-weight and fast locking mechanism
 * without thread sleeping and undesirable context switching. Thus spinlocks
 * should be used only for small code sections, otherwise long-time spinning
 * can cause extensive CPU time waste by waiting threads.
 *
 * As spinlock is based on atomic operations it would have the real meaning
 * only if underlying atomic model is lock-free (not simulated using mutex).
 * You can check if the atomic model is lock-free with p_atomic_is_lock_free().
 * Otherwise usage of spinlocks can bring more waiting time than with mutex
 * itself.
 *
 * To create a new spinlock primitive p_spinlock_new() routine should be
 * called, to delete unused spinlock primitive use p_spinlock_free().
 *
 * Use p_spinlock_lock() or p_spinlock_trylock() to synchronize access at
 * the beginning of a critical section. Only one thread is allowed to pass
 * this call, others will wait for p_spinlock_unlock() call which marks
 * end of the critical section. This way critical section code is guarded
 * against concurrent access from the multiple threads at once.
 */

#if !defined (__PLIB_H_INSIDE__) && !defined (PLIB_COMPILATION)
#  error "Header files shouldn't be included directly, consider using <plib.h> instead."
#endif

#ifndef __PSPINLOCK_H__
#define __PSPINLOCK_H__

#include <pmacros.h>
#include <ptypes.h>

P_BEGIN_DECLS

/** PSpinLock opaque data structure */
typedef struct _PSpinLock PSpinLock;

/**
 * @brief Creates new #PSpinLock object.
 * @return Pointer to newly created #PSpinLock object.
 * @since 0.0.1
 */
P_LIB_API PSpinLock *	p_spinlock_new		(void);

/**
 * @brief Locks spinlock.
 * @param spinlock #PSpinLock to lock.
 * @return TRUE in case of success, FALSE otherwise.
 * @since 0.0.1
 *
 * Thread will not sleep in this call if another thread is holding a lock,
 * instead it will try to lock @a spinlock in infinite loop.
 */
P_LIB_API pboolean	p_spinlock_lock		(PSpinLock *spinlock);

/**
 * @brief Tries to lock spinlock immediately.
 * @param spinlock #PSpinLock to lock.
 * @return TRUE in case of success, FALSE otherwise.
 * @since 0.0.1
 *
 * Tries to lock @a spinlock and returns immediately if it is not available for
 * locking. Do not lock spinlock recursively - this may lead to application
 * deadlock.
 */
P_LIB_API pboolean	p_spinlock_trylock	(PSpinLock *spinlock);

/**
 * @brief Releases locked spinlock.
 * @param spinlock #PSpinLock to release.
 * @return TRUE in case of success, FALSE otherwise.
 * @since 0.0.1
 *
 * If @a spinlock was previously locked then it becomes unlocked. Any thread
 * can unlock any spinlock. It is also safe to call this routine on unlocked
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
 */
P_LIB_API void		p_spinlock_free		(PSpinLock *spinlock);

P_END_DECLS

#endif /* __PSPINLOCK_H__ */
