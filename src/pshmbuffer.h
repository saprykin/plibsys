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
 * @file pshmbuffer.h
 * @brief Shared memory buffer
 * @author Alexander Saprykin
 *
 * A shared memory buffer works like any other buffer but it is built upon a
 * shared memory region instead of the process-only address space. Thus it
 * inherits all the advantages and disadvantages of shared memory behavior. You
 * should read about #PShm before using this buffer implementation to understand
 * underlying restrictions.
 *
 * The shared memory buffer is process-wide and identified by its name across
 * the system, thus it can be opened by any process if it knows its name. Use
 * p_shm_buffer_new() to open the shared memory buffer and p_shm_buffer_free()
 * to close it.
 *
 * All read/write operations are completely thread- and process-safe, which
 * means that no other synchronization primitive is required, even for inter-
 * process access. A #PShm locking mechanism is used for access synchronization.
 *
 * The buffer is cyclic and non-overridable which means that you wouldn't get
 * buffer overflow and wouldn't override previously written data until reading
 * it.
 *
 * The read operation checks whether there is any data available and reads it in
 * case of successful check. After reading the data used space in the buffer is
 * marked as free and any subsequent write operation may overwrite it. Thus you
 * couldn't read the same data twice. The read operation is performed with the
 * p_shm_buffer_read() call.
 *
 * The write operation checks whether there is enough free space available and
 * writes a given memory block only if the buffer has enough free space.
 * Otherwise no data is written. The write operation is performed with the
 * p_shm_buffer_write() call.
 *
 * Data can be read and written into the buffer only sequentially. There is no
 * way to access an arbitrary address inside the buffer.
 *
 * You can take ownership of the shared memory buffer with
 * p_shm_buffer_take_ownership() to explicitly remove it from the system after
 * closing. Please refer to the #PShm description to understand the intention of
 * this action.
 */

#if !defined (PLIBSYS_H_INSIDE) && !defined (PLIBSYS_COMPILATION)
#  error "Header files shouldn't be included directly, consider using <plibsys.h> instead."
#endif

#ifndef PLIBSYS_HEADER_PSHMBUFFER_H
#define PLIBSYS_HEADER_PSHMBUFFER_H

#include <ptypes.h>
#include <pmacros.h>
#include <perror.h>

P_BEGIN_DECLS

/** Shared memory buffer opaque data structure. */
typedef struct PShmBuffer_ PShmBuffer;

/**
 * @brief Creates a new #PShmBuffer structure.
 * @param name Unique buffer name.
 * @param size Buffer size in bytes, can't be changed later.
 * @param[out] error Error report object, NULL to ignore.
 * @return Pointer to the #PShmBuffer structure in case of success, NULL
 * otherwise.
 * @since 0.0.1
 *
 * If a buffer with the same name already exists then the @a size will be
 * ignored and the existing buffer will be returned.
 */
P_LIB_API PShmBuffer *	p_shm_buffer_new		(const pchar	*name,
							 psize		size,
							 PError		**error);

/**
 * @brief Frees #PShmBuffer structure.
 * @param buf #PShmBuffer to free.
 * @since 0.0.1
 *
 * Note that a buffer will be completely removed from the system only after the
 * last instance of the buffer with the same name is closed.
 */
P_LIB_API void		p_shm_buffer_free		(PShmBuffer	*buf);

/**
 * @brief Takes ownership of a shared memory buffer.
 * @param buf Shared memory buffer.
 * @since 0.0.1
 *
 * If you take ownership of the shared memory buffer, p_shm_buffer_free() will
 * try to completely unlink it and remove from the system. This is useful on
 * UNIX systems, where shared memory can survive an application crash. On the
 * Windows and OS/2 platforms this call has no effect.
 *
 * The common usage of this call is upon application startup to ensure that the
 * memory segment from the previous crash can be removed from the system. To do
 * that, call p_shm_buffer_new() and check if its condition is normal (used
 * space, free space). If not, take ownership of the shared memory buffer object
 * and remove it with the p_shm_buffer_free() call. After that, create it again.
 */
P_LIB_API void		p_shm_buffer_take_ownership	(PShmBuffer	*buf);

/**
 * @brief Tries to read data from a shared memory buffer.
 * @param buf #PShmBuffer to read data from.
 * @param[out] storage Output buffer to put data in.
 * @param len Storage size in bytes.
 * @param[out] error Error report object, NULL to ignore.
 * @return Number of read bytes (can be 0 if buffer is empty), or -1 if error
 * occured.
 * @since 0.0.1
 */
P_LIB_API pint		p_shm_buffer_read		(PShmBuffer	*buf,
							 ppointer	storage,
							 psize		len,
							 PError		**error);

/**
 * @brief Tries to write data into a shared memory buffer.
 * @param buf #PShmBuffer to write data into.
 * @param data Data to write.
 * @param len Data size in bytes.
 * @param[out] error Error report object, NULL to ignore.
 * @return Number of written bytes (can be 0 if buffer is full), or -1 if error
 * occured.
 * @since 0.0.1
 * @note Write operation is performed only if the buffer has enough space for
 * the given data size.
 */
P_LIB_API pssize	p_shm_buffer_write		(PShmBuffer	*buf,
							 ppointer	data,
							 psize		len,
							 PError		**error);

/**
 * @brief Gets free space in the shared memory buffer.
 * @param buf #PShmBuffer to check space in.
 * @param[out] error Error report object, NULL to ignore.
 * @return Free space in bytes in case of success, -1 otherwise.
 * @since 0.0.1
 */
P_LIB_API pssize	p_shm_buffer_get_free_space	(PShmBuffer	*buf,
							 PError		**error);

/**
 * @brief Gets used space in the shared memory buffer.
 * @param buf #PShmBuffer to check space in.
 * @param[out] error Error report object, NULL to ignore.
 * @return Used space in bytes in case of success, -1 otherwise.
 * @since 0.0.1
 */
P_LIB_API pssize	p_shm_buffer_get_used_space	(PShmBuffer	*buf,
							 PError		**error);

/**
 * @brief Clears all data in the buffer and fills it with zeros.
 * @param buf #PShmBuffer to clear.
 * @since 0.0.1
 */
P_LIB_API void		p_shm_buffer_clear		(PShmBuffer	*buf);

P_END_DECLS

#endif /* PLIBSYS_HEADER_PSHMBUFFER_H */
