/* 
 * 01.09.2010
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

#ifndef __PHASHTABLE_H__
#define __PHASHTABLE_H__

#include <pmacros.h>
#include <plist.h>

typedef struct _PHashTableNode PHashTableNode;
typedef struct _PHashTable PHashTable;

struct _PHashTable {
	PHashTableNode **table;
	psize size;
};

P_BEGIN_DECLS

P_LIB_API PHashTable *	p_hash_table_new	(void);
P_LIB_API void		p_hash_table_insert	(PHashTable *table, ppointer key, ppointer value);
P_LIB_API ppointer	p_hash_table_lookup	(PHashTable *table, ppointer key);
P_LIB_API PList *	p_hash_table_keys	(PHashTable *table);
P_LIB_API PList *	p_hash_table_values	(PHashTable *table);
P_LIB_API void		p_hash_table_free	(PHashTable *table);

P_END_DECLS

#endif /* __PHASHTABLE_H__ */

