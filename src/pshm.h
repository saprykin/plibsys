/* 
 * 24.08.2010
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
 * @file pshm.h
 * @brief Shared memory routines
 * @author Alexander Saprykin
 *
 * Shared memory is an interprocess communication primitive. It can be used to transfer data
 * between several processes. PLib supports different shared memory implementations: System V,
 * POSIX and Win32. PLib is compiled using one of them (depend which of implementations
 * are available on target system). Windows IPC system different from Unix one: Windows doesn't
 * own IPC objects (processes own them), while Unix systems do. Because of that Unix IPC
 * objects can survive application crash: opened shared memory can contain data from previous
 * working session. Keep this fact in mind while developing cross-platform applications.
 * Also note that you must synchronize multi-thread access to shared memory by youself or
 * using p_shm_lock() and p_shm_unlock() routines.
 */

#if !defined (__PLIB_H_INSIDE__) && !defined (PLIB_COMPILATION)
#  error "Header files shouldn't be included directly, consider using <plib.h> instead."
#endif

#ifndef __PSHM_H__
#define __PSHM_H__

#include <pmacros.h>
#include <ptypes.h>

/** Enum with #PShm access permitions */
typedef enum _PShmAccessPerms {
	P_SHM_ACCESS_READONLY	= 0,	/**< Read-only access */
	P_SHM_ACCESS_READWRITE	= 1	/**< Read/write access */
} PShmAccessPerms;

/** #PShm opaque data structure */
typedef struct _PShm PShm;

P_BEGIN_DECLS

/**
 * @brief Creates new #PShm object.
 * @param name Shared memory name.
 * @param size Size of the memory segment, can't be changed later.
 * @param perms Memory segment permissions, see #PShmAccessPerms.
 * @return Pointer to newly created #PShm object in case of success, NULL otherwise.
 * @since 0.0.1
 */
P_LIB_API PShm *	p_shm_new		(const pchar		*name,
						 psize			size,
						 PShmAccessPerms	perms);

/**
 * @brief Frees shared memory object.
 * @param shm #PShm to free.
 * @since 0.0.1
 *
 * It doesn't unlock given shared memory, be careful to not to make deadlock
 * or segfault while freeing memory segment which are under using.
 */
P_LIB_API void		p_shm_free		(PShm *shm);

/**
 * @brief Locks #PShm object for usage. If object is already locked then
 * thread will be slept until object becomes unocked.
 * @param shm #PShm to lock.
 * @return TRUE in case of success, FALSE otherwise.
 * @since 0.0.1
 */
P_LIB_API pboolean	p_shm_lock		(PShm *shm);

/**
 * @brief Unlocks #PShm object.
 * @param shm #PShm to unlock.
 * @return TRUE in case of success, FALSE otherwise.
 * @since 0.0.1
 */
P_LIB_API pboolean	p_shm_unlock		(PShm *shm);

/**
 * @brief Gets starting address of the #PShm memory segment.
 * @param shm #PShm to get address for.
 * @return Pointer to starting address in case of success, NULL otherwise.
 * @since 0.0.1
 */
P_LIB_API ppointer	p_shm_get_address	(PShm *shm);

/**
 * @brief Gets size of the #PShm memory segment.
 * @param shm #PShm to get size for.
 * @return Size of the given memory segment in case of success, 0 otherwise.
 * @since 0.0.1
 *
 * Note that returned size would be a slight bigger then specified during
 * p_shm_new() call due to service information stored there.
 */
P_LIB_API psize		p_shm_get_size		(PShm *shm);

P_END_DECLS

#endif /* __PSHM_H__ */

