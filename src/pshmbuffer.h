/* 
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
 * @file pshmbuffer.h
 * @brief Shared memory buffer
 * @author Alexander Saprykin
 *
 * #PShmBuffer implements cyclic non-override buffer based on shared memory segment.
 * This allows different processes to exchange data without using sockets as often
 * is done. Thus you'll get a bit smaller overheads then using sockets. Please read
 * documentation for #PShm for known issues with UNIX and Windows IPC systems.
 */

#if !defined (__PLIB_H_INSIDE__) && !defined (PLIB_COMPILATION)
#  error "Header files shouldn't be included directly, consider using <plib.h> instead."
#endif

#ifndef __PSHMBUFFER_H__
#define __PSHMBUFFER_H__

#include <ptypes.h>
#include <pmacros.h>

#include <stdlib.h>

/** #PShmBuffer opaque data structure */
typedef struct _PShmBuffer PShmBuffer;

P_BEGIN_DECLS

/**
 * @brief Creates new #PShmBuffer structure. If buffer with the same name
 * is already exists then size will be ignored and existing buffer will be returned.
 * @param name Unique buffer name.
 * @param size Buffer size in bytes, can't be changed later.
 * @return Pointer to #PShmBuffer structure in case of success, NULL otherwise.
 * @since 0.0.1
 */
P_LIB_API PShmBuffer *	p_shm_buffer_new		(const pchar *name, psize size);

/**
 * @brief Frees #PShmBuffer structure. Note that buffer will be fully removed
 * from the system only after the last instance of the buffer with the same name
 * is closed.
 * @param buf #PShmBuffer to free.
 * @since 0.0.1
 */
P_LIB_API void		p_shm_buffer_free		(PShmBuffer *buf);

/**
 * @brief Takes ownership of the shared memory buffer.
 * @param buf Shared memory buffer.
 * @since 0.0.1
 *
 * If you take ownership of the shared memory buffer, p_shm_buffer_free()
 * will try to completely unlink it and remove from the system.
 * This is useful on UNIX systems with POSIX and System V IPC implementations, where shared
 * memory can survive the application crash. On Windows platform this call has no effect.
 * The common usage of this call is upon application startup to ensure that
 * memory segment from the previous crash can be unlinked from the system. To
 * do that, call p_shm_buffer_new(), and check if its condition is OK (used space,
 * free space). If not, take ownership of the shared memory buffer object and remove it with
 * p_shm_buffer_free() call. After that, create it again.
 */
P_LIB_API void		p_shm_buffer_take_ownership	(PShmBuffer *buf);

/**
 * @brief Tries to read data from shared memory buffer.
 * @param buf #PShmBuffer to read data from.
 * @param[out] storage Output buffer to put data in.
 * @param len @a storage size in bytes.
 * @return Number of read bytes (can be 0 if buffer is empty), or -1 if error occured. 
 * @since 0.0.1
 */
P_LIB_API pint		p_shm_buffer_read		(PShmBuffer *buf, ppointer storage, psize len);

/**
 * @brief Tries to write data into shared memory buffer.
 * @param buf #PShmBuffer to write data into.
 * @param data Data to write.
 * @param len @a data size in bytes.
 * @return Number of written bytes, or -1 if error occured or buffer is full. 
 * @since 0.0.1
 */
P_LIB_API pssize	p_shm_buffer_write		(PShmBuffer *buf, ppointer data, psize len);

/**
 * @brief Gets free space in shared memory buffer.
 * @param buf #PShmBuffer to check space in.
 * @return Free space in bytes in case of success, -1 otherwise.
 * @since 0.0.1
 */
P_LIB_API pssize	p_shm_buffer_get_free_space	(PShmBuffer *buf);

/**
 * @brief Gets used space in shared memory buffer.
 * @param buf #PShmBuffer to check space in.
 * @return Used space in bytes in case of success, -1 otherwise. 
 * @since 0.0.1
 */
P_LIB_API pssize	p_shm_buffer_get_used_space	(PShmBuffer *buf);

/**
 * @brief Clears all data in buffer and fills it with zeros.
 * @param buf #PShmBuffer to clear.
 * @since 0.0.1
 */
P_LIB_API void		p_shm_buffer_clear		(PShmBuffer *buf);

P_END_DECLS

#endif /* __PSHMBUFFER_H__ */
