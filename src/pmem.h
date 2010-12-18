/* 
 * 09.12.2010
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

#if !defined (__PLIB_H_INSIDE__) && !defined (PLIB_COMPILATION)
#  error "Header files shouldn't be included directly, consider using <plib.h> instead."
#endif

#ifndef __PMEM_H__
#define __PMEM_H__

#include <ptypes.h>
#include <pmacros.h>

P_BEGIN_DECLS

typedef struct _PMemVTable {
	ppointer	(*malloc)	(psize n);
	ppointer	(*realloc)	(ppointer mem, psize n_bytes);
	void		(*free)		(ppointer mem);
} PMemVTable;

P_LIB_API ppointer	p_malloc		(psize n_bytes);
P_LIB_API ppointer	p_malloc0		(psize n_bytes);
P_LIB_API ppointer	p_realloc		(ppointer mem, psize n_bytes);
P_LIB_API void		p_free			(ppointer mem);
P_LIB_API pboolean	p_mem_set_vtable	(PMemVTable *table);

P_END_DECLS

#endif /* __PMEM_H__ */
