/*
 * Copyright (C) 2015-2016 Alexander Saprykin <xelfium@gmail.com>
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, see <http://www.gnu.org/licenses/>.
 */

#if !defined (PLIBSYS_H_INSIDE) && !defined (PLIBSYS_COMPILATION)
#  error "Header files shouldn't be included directly, consider using <plibsys.h> instead."
#endif

#ifndef PLIBSYS_HEADER_PTREEBST_H
#define PLIBSYS_HEADER_PTREEBST_H

#include "pmacros.h"
#include "ptypes.h"
#include "ptree-private.h"

P_BEGIN_DECLS

pboolean	p_tree_bst_insert	(PTreeBaseNode		**root_node,
					 PCompareDataFunc	compare_func,
					 ppointer		data,
					 PDestroyFunc		key_destroy_func,
					 PDestroyFunc		value_destroy_func,
					 ppointer		key,
					 ppointer		value);

pboolean	p_tree_bst_remove	(PTreeBaseNode		**root_node,
					 PCompareDataFunc	compare_func,
					 ppointer		data,
					 PDestroyFunc		key_destroy_func,
					 PDestroyFunc		value_destroy_func,
					 pconstpointer		key);

void		p_tree_bst_node_free	(PTreeBaseNode	*node);

P_END_DECLS

#endif /* PLIBSYS_HEADER_PTREEBST_H */
