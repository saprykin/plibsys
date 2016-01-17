/*
 * Copyright (C) 2015-2016 Alexander Saprykin <xelfium@gmail.com>
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

#ifndef __PTREERB_H__
#define __PTREERB_H__

#include "pmacros.h"
#include "ptypes.h"
#include "plib-private.h"

P_BEGIN_DECLS

pboolean	__p_tree_rb_insert	(__PTreeBaseNode	**root_node,
					 PCompareDataFunc	compare_func,
					 ppointer		data,
					 PDestroyFunc		key_destroy_func,
					 PDestroyFunc		value_destroy_func,
					 ppointer		key,
					 ppointer		value);

pboolean	__p_tree_rb_remove	(__PTreeBaseNode	**root_node,
					 PCompareDataFunc	compare_func,
					 ppointer		data,
					 PDestroyFunc		key_destroy_func,
					 PDestroyFunc		value_destroy_func,
					 pconstpointer		key);

void		__p_tree_rb_node_free	(__PTreeBaseNode	*node);

P_END_DECLS

#endif /* __PTREERB_H__ */
