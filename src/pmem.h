/* 
 * Copyright (C) 2010-2014 Alexander Saprykin <xelfium@gmail.com>
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
 * @file pmem.h
 * @brief Memory management
 * @author Alexander Saprykin
 * 
 * While using PLib you should use corresponding memory allocation/free functions.
 * For now, all these functions are wrappers around system memory related functions
 * (like malloc, free, etc). But this approach gives you much more power to
 * control memory allocation inside application. If you want to use your own allocator
 * consider using #PMemVTable structure and p_mem_set_vtable() method. This way
 * you can easly change memory management in the whole PLib. Do not forget to set
 * default memory management functions before calling p_lib_shutdown().
 */

#if !defined (__PLIB_H_INSIDE__) && !defined (PLIB_COMPILATION)
#  error "Header files shouldn't be included directly, consider using <plib.h> instead."
#endif

#ifndef __PMEM_H__
#define __PMEM_H__

#include <ptypes.h>
#include <pmacros.h>

P_BEGIN_DECLS

/** Structure to set memory management methods */
typedef struct _PMemVTable {
	ppointer	(*malloc)	(psize n);			/**< malloc() implementation */
	ppointer	(*realloc)	(ppointer mem, psize n_bytes);	/**< realloc() implementation */
	void		(*free)		(ppointer mem);			/**< free() implementation */
} PMemVTable;

/**
 * @brief Allocates memory for specified number of bytes.
 * @param n_bytes Bytes to allocate memory for.
 * @return Pointer to newly allocated memory in case of success,
 * NULL otherwise.
 * @since 0.0.1
 */
P_LIB_API ppointer	p_malloc		(psize n_bytes);

/**
 * @brief Allocates memory for specified number of bytes and fills it
 * with zeros.
 * @param n_bytes Bytes to allocate memory for.
 * @return Pointer to newly allocated memory filled with zeros in case of success,
 * NULL otherwise.
 * @since 0.0.1
 */
P_LIB_API ppointer	p_malloc0		(psize n_bytes);

/**
 * @brief Changes memory block's size.
 * @param mem Pointer to memory block.
 * @param n_bytes New size for @a mem block.
 * @return Pointer to newlly allocated memory in case of success (if @a mem is NULL
 * then it acts like p_malloc()), NULL otherwise.
 * @since 0.0.1
 */
P_LIB_API ppointer	p_realloc		(ppointer mem, psize n_bytes);

/**
 * @brief Frees memory by it's pointer.
 * @param mem Pointer to memory to free.
 * @since 0.0.1
 *
 * You should only call this function for pointers which were obtained using
 * p_malloc(), p_malloc0() and p_realloc() function, otherwise behaviour
 * is unpredictable. It checks pointer for NULL value.
 */
P_LIB_API void		p_free			(ppointer mem);

/**
 * @brief Sets custom memory management functions.
 * @param table Table of memory functions to use.
 * @return TRUE if table was accepted, FALSE otherwise.
 * @note All members of @a table must be non-NULL.
 * @warning Do not forget to set default memory management functions
 * before calling p_lib_shutdown() because p_lib_init() always use
 * system allocator.
 * @since 0.0.1
 *
 * In most cases you do not need to use this function. Use it only
 * when you know what are you doing!
 */
P_LIB_API pboolean	p_mem_set_vtable	(PMemVTable *table);

/**
 * @brief Gets memory from the system using mmap() call.
 * @param n_bytes Bytes of memory to allocate.
 * @return Pointer to allocated memory in case of success, NULL
 * otherwise.
 * @since 0.0.1
 *
 * Note that some systems can allocate memory only in chunks of
 * page size, so if @a n_bytes is less than page size it will
 * try to allocate a chunk of memory equal to page size instead.
 */
P_LIB_API ppointer	p_mem_mmap		(psize n_bytes);

/**
 * @brief Unmaps memory to system back using munmap() call.
 * @param mem Pointer to memory previously allocated using
 * p_mem_mmap() call.
 * @param n_bytes Bytes of memory to unmap.
 * @return TRUE in case of success, FALSE otherwise.
 * @since 0.0.1
 */
P_LIB_API pboolean	p_mem_munmap		(ppointer mem, psize n_bytes);

P_END_DECLS

#endif /* __PMEM_H__ */
