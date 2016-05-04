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
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

#if !defined (__PLIBSYS_H_INSIDE__) && !defined (PLIBSYS_COMPILATION)
#  error "Header files shouldn't be included directly, consider using <plibsys.h> instead."
#endif

#ifndef __PTREEBST_H__
#define __PTREEBST_H__

#include "pmacros.h"
#include "ptypes.h"
#include "plibsys-private.h"

P_BEGIN_DECLS

pboolean	__p_tree_bst_insert	(__PTreeBaseNode	**root_node,
					 PCompareDataFunc	compare_func,
					 ppointer		data,
					 PDestroyFunc		key_destroy_func,
					 PDestroyFunc		value_destroy_func,
					 ppointer		key,
					 ppointer		value);

pboolean	__p_tree_bst_remove	(__PTreeBaseNode	**root_node,
					 PCompareDataFunc	compare_func,
					 ppointer		data,
					 PDestroyFunc		key_destroy_func,
					 PDestroyFunc		value_destroy_func,
					 pconstpointer		key);

void		__p_tree_bst_node_free	(__PTreeBaseNode	*node);

P_END_DECLS

#endif /* __PTREEBST_H__ */
