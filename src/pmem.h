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
 * @file pmem.h
 * @brief Memory management
 * @author Alexander Saprykin
 *
 * Usually system routines for memory management are used: malloc(), realloc(),
 * free() and so on. But it is highly encouraged to use a more general approach:
 * p_malloc(), p_malloc0(), p_realloc() and p_free() family of memory management
 * routines. It gives you several advantages:
 * - automatical checking of all input parameters for NULL values;
 * - ability to use custom memory allocator.
 * You can also mix these two families of calls, but it is not recommended.
 *
 * By default p_* routines are mapped to system calls, thus only NULL-checking
 * is additionally performed. If you want to use custom memory allocator fill in
 * #PMemVTable structure and pass it to the p_mem_set_vtable().
 *
 * Be careful when using custom memory allocator: all memory chunks allocated
 * with custom allocator must be freed with the same allocator. If a custom
 * allocator was installed after the library initialization call p_libsys_init()
 * you must to restore original allocator before calling p_libsys_shutdown().
 *
 * Use p_mem_mmap() to allocate system memory using memory mapping and
 * p_mem_munmap() to release mapped memory.
 */

#if !defined (__PLIBSYS_H_INSIDE__) && !defined (PLIBSYS_COMPILATION)
#  error "Header files shouldn't be included directly, consider using <plibsys.h> instead."
#endif

#ifndef __PMEM_H__
#define __PMEM_H__

#include <ptypes.h>
#include <pmacros.h>
#include <perror.h>

P_BEGIN_DECLS

/** Memory management table. */
typedef struct _PMemVTable {
	ppointer	(*malloc)	(psize		n_bytes);	/**< malloc() implementation.	*/
	ppointer	(*realloc)	(ppointer	mem,
					 psize		n_bytes);	/**< realloc() implementation.	*/
	void		(*free)		(ppointer	mem);		/**< free() implementation.	*/
} PMemVTable;

/**
 * @brief Allocates memory block for the specified number of bytes.
 * @param n_bytes Size of the memory block in bytes.
 * @return Pointer to a newly allocated memory block in case of success, NULL
 * otherwise.
 * @since 0.0.1
 */
P_LIB_API ppointer	p_malloc		(psize			n_bytes);

/**
 * @brief Allocates memory block for the specified number of bytes and fills it
 * with zeros.
 * @param n_bytes Size of the memory block in bytes.
 * @return Pointer to a newly allocated memory block filled with zeros in case
 * of success, NULL otherwise.
 * @since 0.0.1
 */
P_LIB_API ppointer	p_malloc0		(psize			n_bytes);

/**
 * @brief Changes memory block size.
 * @param mem Pointer to the memory block.
 * @param n_bytes New size for @a mem block.
 * @return Pointer to a newlly allocated memory block in case of success (if
 * @a mem is NULL then it acts like p_malloc()), NULL otherwise.
 * @since 0.0.1
 */
P_LIB_API ppointer	p_realloc		(ppointer		mem,
						 psize			n_bytes);

/**
 * @brief Frees memory block by its pointer.
 * @param mem Pointer to the memory block to free.
 * @since 0.0.1
 *
 * You should only call this function for pointers which were obtained using
 * p_malloc(), p_malloc0() and p_realloc() routines, otherwise behavior is
 * unpredictable.
 *
 * Checks pointer for NULL value.
 */
P_LIB_API void		p_free			(ppointer		mem);

/**
 * @brief Sets custom memory management routines.
 * @param table Table of the memory routines to use.
 * @return TRUE if table was accepted, FALSE otherwise.
 * @note All members of @a table must be non-NULL.
 * @warning Do not forget to set original memory management routines before
 * calling p_libsys_shutdown() if you have used p_mem_set_vtable() after the
 * library initialization.
 * @since 0.0.1
 *
 * In most cases you do not need to use this function. Use it only when you know
 * what are you doing!
 */
P_LIB_API pboolean	p_mem_set_vtable	(const PMemVTable	*table);

/**
 * @brief Gets memory mapped block from the system.
 * @param n_bytes Size of the memory block in bytes.
 * @param[out] error Error report object, NULL to ignore.
 * @return Pointer to allocated memory block in case of success, NULL otherwise.
 * @since 0.0.1
 *
 * Note that some systems can allocate memory only in chunks of the page size,
 * so if @a n_bytes is less than page size it will try to allocate a chunk of
 * memory equal to page size instead.
 *
 * On most systems returned memory is mapped to null or swap device.
 */
P_LIB_API ppointer	p_mem_mmap		(psize			n_bytes,
						 PError			**error);

/**
 * @brief Unmaps memory back to the system.
 * @param mem Pointer to the memory block previously allocated using
 * p_mem_mmap() call.
 * @param n_bytes Size of the memory block in bytes.
 * @param[out] error Error report object, NULL to ignore.
 * @return TRUE in case of success, FALSE otherwise.
 * @since 0.0.1
 */
P_LIB_API pboolean	p_mem_munmap		(ppointer		mem,
						 psize			n_bytes,
						 PError			**error);

P_END_DECLS

#endif /* __PMEM_H__ */
