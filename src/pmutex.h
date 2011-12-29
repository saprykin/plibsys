/* 
 * 16.09.2010
 * Copyright (C) 2010 Alexander Saprykin <xelfium@gmail.com>
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
 * @file pmutex.h
 * @brief Mutex routines
 * @author Alexander Saprykin
 *
 * Mutex is a thread synchronization primitive. It's a binary semaphore in other
 * words. PLib supports different mutex implementations: System V, POSIX, Sun Solaris
 * and Win32. PLib is compiled using one of them (depend which of implementations
 * are available on target system). Because of that some non-usual actions (double-lock,
 * unlock non-locked mutex) can lead to unpredictable behaviour. Do not rely on such
 * features when writing cross-platform applications.
 */

#if !defined (__PLIB_H_INSIDE__) && !defined (PLIB_COMPILATION)
#  error "Header files shouldn't be included directly, consider using <plib.h> instead."
#endif

#ifndef __PMUTEX_H__
#define __PMUTEX_H__

#include <pmacros.h>
#include <ptypes.h>

P_BEGIN_DECLS

/** PMutex opaque data structure */
typedef struct _PMutex PMutex;

/**
 * @brief Creates new #PMutex object.
 * @return Pointer to newly created #PMutex object.
 * @since 0.0.1
 */
P_LIB_API PMutex *	p_mutex_new	(void);

/**
 * @brief Locks mutex.
 * @param mutex #PMutex to lock.
 * @return TRUE in case of success, FALSE otherwise.
 * @since 0.0.1
 *
 * Force calling thread to sleep until @a mutex becomes available for locking.
 */
P_LIB_API pboolean	p_mutex_lock	(PMutex *mutex);

/**
 * @brief Tries to lock mutex immediately.
 * @param mutex #PMutex to lock.
 * @return TRUE in case of success, FALSE otherwise.
 * @since 0.0.1
 *
 * Tries to lock @a mutex and returns immediately if it is not available for
 * locking. Do not recursively lock mutex - this may lead to application
 * deadlock (implementation dependent).
 */
P_LIB_API pboolean	p_mutex_trylock	(PMutex *mutex);

/**
 * @brief Releases locked mutex.
 * @param mutex #PMutex to release.
 * @return TRUE in case of success, FALSE otherwise.
 * @since 0.0.1
 *
 * If @a mutex was previousely lock then it becomes unlocked. Do not use
 * this function on non-locked mutexes - behaviour may be unpredictable.
 * It's implementation dependent whether only the same thread can lock and
 * unlock mutex.
 */
P_LIB_API pboolean	p_mutex_unlock	(PMutex *mutex);

/**
 * @brief Frees #PMutex object.
 * @param mutex #PMutex to free.
 * @since 0.0.1
 *
 * It doesn't unlock @a mutex before freeing memory, so you should do it
 * manually.
 */
P_LIB_API void		p_mutex_free	(PMutex *mutex);

/** Locks static mutex. */
#define p_static_mutex_lock(mutex)	p_mutex_lock (p_static_mutex_get_mutex (mutex))

/** Immediatly tries to lock static mutex. */
#define p_static_mutex_trylock(mutex)	p_mutex_trylock (p_static_mutex_get_mutex (mutex))

/** Unlocks static mutex. */
#define p_static_mutex_unlock(mutex)	p_mutex_unlock (p_static_mutex_get_mutex (mutex))

/**
 * @brief Initializes #PStaticMutex object.
 * @param mutex #PStaticMutex to initialize. * @since 0.0.1
 */
P_LIB_API void		p_static_mutex_init (PStaticMutex *mutex);

/**
 * @brief Frees #PStaticMutex object.
 * @param mutex #PStaticMutex to free. * @since 0.0.1
 *
 * It doesn't unlock @a mutex before freeing memory, so you should do it
 * manually.
 */
P_LIB_API void		p_static_mutex_free (PStaticMutex *mutex);

P_END_DECLS

#endif /* __PMUTEX_H__ */

