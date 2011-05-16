/* 
 * 22.08.2010
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
 * @file psemaphore.h
 * @brief Semaphore routines
 * @author Alexander Saprykin
 *
 * Semaphore is a system-wide synchronization primitive. It can be used to synchronize different
 * processes within the system. PLib supports different semaphore implementations: System V,
 * POSIX and Win32. PLib is compiled using one of them (depend which of implementations
 * are available on target system). Windows IPC system different from Unix one: Windows doesn't
 * own IPC objects (processes own them), while Unix systems do. Because of that Unix IPC
 * objects can survive application crash: semaphore can be opened in locked state if it was
 * locked during application crash. Use third argument as P_SEM_ACCESS_CREATE in p_semaphore_new()
 * to reset semaphore value while openning it. This argument is ignored on Windows. System V
 * semaphores are more resistant to crashes (leaving in locked state) than POSIX ones.
 * #PSemaphore is quite heavy structure, so consider using #PMutex for thread synchronization
 * instead (if possible). 
 */

#if !defined (__PLIB_H_INSIDE__) && !defined (PLIB_COMPILATION)
#  error "Header files shouldn't be included directly, consider using <plib.h> instead."
#endif

#ifndef __PSEMAPHORE_H__
#define __PSEMAPHORE_H__

#include <pmacros.h>
#include <ptypes.h>

P_BEGIN_DECLS

/** Enum with #PSemaphore errors */
typedef enum _PSemaphoreError {
	P_SEM_ERROR_NONE	= 0,	/**< No error */
	P_SEM_ERROR_ACCESS	= 1,	/**< Not enough rights to access semphore or its key */
	P_SEM_ERROR_RESOURSE	= 2,	/**< Not enough system resources */
	P_SEM_ERROR_EXISTS	= 3,	/**< Semaphore doesn't exist or was removed before */
	P_SEM_ERROR_UNKNOWN	= 4	/**< Unknown error */
} PSemaphoreError;

/** Enum with #PSemaphore creation modes */
typedef enum _PSemaphoreAccessMode {
	P_SEM_ACCESS_OPEN	= 0,	/**< Open existing semaphore or create one with given value */
	P_SEM_ACCESS_CREATE	= 1	/**< Create semaphore, reset to given value if exists */
} PSemaphoreAccessMode;

/** #PSemaphore opaque data structure */
typedef struct _PSemaphore PSemaphore;

/**
 * @brief Creates new #PSemaphore object.
 * @param name Semaphore name.
 * @param init_val Initial semaphore value.
 * @param mode Creation mode.
 * @return Pointer to newly created #PSemaphore object in case of success, NULL otherwise.
 * @since 0.0.1
 *
 * @a init_val used only in one of following cases: semaphore with such name doesn't
 * exist, semaphore with such name exists but @a mode specified as P_SEM_ACCESS_CREATE
 * (non-Windows platforms only). In other cases @a init_val ignored. @a name is
 * system-wide, so any process can open semaphore with the same name.
 */
P_LIB_API PSemaphore *	p_semaphore_new 	(const pchar		*name,
						 pint			init_val,
						 PSemaphoreAccessMode	mode);

/**
 * @brief Acquires semaphore.
 * @param sem #PSemaphore to acquire.
 * @return TRUE in case of success, FALSE otherwise.
 * @since 0.0.1
 */
P_LIB_API pboolean	p_semaphore_acquire 	(PSemaphore		*sem);

/**
 * @brief Releases semaphore.
 * @param sem #PSemaphore to release.
 * @return TRUE in case of success, FALSE otherwise.
 * @since 0.0.1
 */
P_LIB_API pboolean	p_semaphore_release	(PSemaphore		*sem);

/**
 * @brief Frees semaphore object.
 * @param sem #PSemaphore to free.
 * @since 0.0.1
 *
 * It doesn't release acquired semaphore, be careful to not to make deadlock
 * while removing locked semaphore.
 */
P_LIB_API void		p_semaphore_free	(PSemaphore		*sem);

P_END_DECLS

#endif /* __PSEMAPHORE_H__ */

