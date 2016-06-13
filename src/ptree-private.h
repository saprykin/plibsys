/*
 * Copyright (C) 2016 Alexander Saprykin <xelfium@gmail.com>
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

#ifndef PLIBSYS_HEADER_PTREE_PRIVATE_H
#define PLIBSYS_HEADER_PTREE_PRIVATE_H

#include "pmacros.h"
#include "ptypes.h"

P_BEGIN_DECLS

/** Base tree leaf structure. */
typedef struct PTreeBaseNode_ {
	struct PTreeBaseNode_	*left;	/**< Left child.	*/
	struct PTreeBaseNode_	*right;	/**< Right child.	*/
	ppointer		key;	/**< Node key.		*/
	ppointer		value;	/**< Node value.	*/
} PTreeBaseNode;

P_END_DECLS

#endif /* PLIBSYS_HEADER_PTREE_PRIVATE_H */
