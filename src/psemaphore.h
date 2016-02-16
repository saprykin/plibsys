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
 * POSIX and Windows. PLib is compiled using one of them (depend which of implementations
 * are available on target system).
 *
 * Please note the following platform specific differences:
 *
 * - Windows doesn't own IPC objects (processes own them), which means that semaphore will
 * be removed after the last process or thread removes it (or after terminating
 * all the processes and threads holding opened semaphore).
 *
 * - UNIX systems own IPC objects. Because of that UNIX IPC objects can survive application
 * crash: already used semaphore can be opened in locked state and application can fail into
 * the deadlock or inconsistent state. This could happen if you have not removed all the
 * opened semaphores explicitly before terminating the application.
 *
 * Use third argument as #P_SEM_ACCESS_CREATE in p_semaphore_new() to reset semaphore value
 * while openning it. This argument is ignored on Windows. System V semaphores are more
 * resistant to crashes (leaving in locked state) than POSIX ones.
 *
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
#include <perror.h>

P_BEGIN_DECLS

/** Enum with #PSemaphore errors */
typedef enum _PSemaphoreError {
	P_SEM_ERROR_NONE		= 0,	/**< No error.						*/
	P_SEM_ERROR_ACCESS		= 1,	/**< Not enough rights to access semaphore or its key.
						     Possible permission problem.			*/
	P_SEM_ERROR_EXISTS		= 2,	/**< Semaphore already exists and no proper open
						     flags were specified.				*/
	P_SEM_ERROR_NOT_EXISTS		= 3,	/**< Semaphore doesn't exist or was removed before, and
						     no proper create flags were specified.		*/
	P_SEM_ERROR_NO_RESOURCES	= 4,	/**< Not enough system resources or memory to perform
						     operation.						*/
	P_SEM_ERROR_OVERFLOW		= 5,	/**< Semaphore value overflow.				*/
	P_SEM_ERROR_NAMETOOLONG		= 6,	/**< Semaphore name is too long.			*/
	P_SEM_ERROR_INVALID_ARGUMENT	= 7,	/**< Invalid argument (parameter) specified.		*/
	P_SEM_ERROR_NOT_IMPLEMENTED	= 8,	/**< Operation not implemented (for example when using
						     some file systems).				*/
	P_SEM_ERROR_DEADLOCK		= 9,	/**< Deadlock detected.					*/
	P_SEM_ERROR_FAILED		= 10	/**< General error.					*/
} PSemaphoreError;

/** Enum with #PSemaphore creation modes */
typedef enum _PSemaphoreAccessMode {
	P_SEM_ACCESS_OPEN	= 0,	/**< Open existing semaphore or create one with given value.	*/
	P_SEM_ACCESS_CREATE	= 1	/**< Create semaphore, reset to given value if exists.		*/
} PSemaphoreAccessMode;

/** #PSemaphore opaque data structure */
typedef struct _PSemaphore PSemaphore;

/**
 * @brief Creates new #PSemaphore object.
 * @param name Semaphore name.
 * @param init_val Initial semaphore value.
 * @param mode Creation mode.
 * @param[out] error Error report object, NULL to ignore.
 * @return Pointer to newly created #PSemaphore object in case of success, NULL otherwise.
 * @since 0.0.1
 *
 * @a init_val used only in one of following cases: semaphore with such name doesn't
 * exist, semaphore with such name exists but @a mode specified as #P_SEM_ACCESS_CREATE
 * (non-Windows platforms only). In other cases @a init_val ignored. @a name is
 * system-wide, so any process can open semaphore with the same name.
 */
P_LIB_API PSemaphore *	p_semaphore_new			(const pchar		*name,
							 pint			init_val,
							 PSemaphoreAccessMode	mode,
							 PError			**error);

/**
 * @brief Takes ownership of the semaphore.
 * @param sem Semaphore to take ownership.
 * @since 0.0.1
 *
 * If you take ownership of the semaphore object, p_semaphore_free()
 * will try to completely unlink it and remove from the system.
 * This is useful on UNIX systems with POSIX and System V IPC implementations, where shared
 * memory can survive the application crash. On Windows platform this call has no effect.
 * The common usage of this call is upon application startup to ensure that
 * semaphore from the previous crash can be unlinked from the system. To
 * do that, call p_semaphore_new(), take ownership of the semaphore object and remove it with
 * p_semaphore_free() call. After that, create it again.
 * You can also do the same thing upon semaphore creation passing #P_SEM_ACCESS_CREATE
 * to p_semaphore_new(). The only difference is that you should already know whether
 * this semaphore object is from the previous crash or not.
 */
P_LIB_API void		p_semaphore_take_ownership	(PSemaphore		*sem);

/**
 * @brief Acquires semaphore.
 * @param sem #PSemaphore to acquire.
 * @param[out] error Error report object, NULL to ignore.
 * @return TRUE in case of success, FALSE otherwise.
 * @since 0.0.1
 */
P_LIB_API pboolean	p_semaphore_acquire		(PSemaphore		*sem,
							 PError			**error);

/**
 * @brief Releases semaphore.
 * @param sem #PSemaphore to release.
 * @param[out] error Error report object, NULL to ignore.
 * @return TRUE in case of success, FALSE otherwise.
 * @since 0.0.1
 */
P_LIB_API pboolean	p_semaphore_release		(PSemaphore		*sem,
							 PError			**error);

/**
 * @brief Frees semaphore object.
 * @param sem #PSemaphore to free.
 * @since 0.0.1
 *
 * It doesn't release acquired semaphore, be careful to not to make deadlock
 * while removing locked semaphore.
 */
P_LIB_API void		p_semaphore_free		(PSemaphore		*sem);

P_END_DECLS

#endif /* __PSEMAPHORE_H__ */
