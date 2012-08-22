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

/**
 * @file phashtable.h
 * @brief Simple hash-table implememtation
 * @author Alexander Saprykin
 * 
 * Hash-table is using to store key-value pairs. You can perform
 * fast lookup for value associated with the given key. Note, that
 * #PHashTable stores keys and values only as pointers, so you need
 * to free used memory manually, p_hash_table_free() will not do it
 * in any way. Integers (up to 32 bits) can be stored in pointers
 * using #P_POINTER_TO_INT and #P_INT_TO_POINTER macros.
 */

#if !defined (__PLIB_H_INSIDE__) && !defined (PLIB_COMPILATION)
#  error "Header files shouldn't be included directly, consider using <plib.h> instead."
#endif

#ifndef __PHASHTABLE_H__
#define __PHASHTABLE_H__

#include <pmacros.h>
#include <plist.h>

P_BEGIN_DECLS

/** Opaque data structure for hash-table node. */
typedef struct _PHashTableNode PHashTableNode;

/** Opaque data structure for hash-table. */
typedef struct _PHashTable PHashTable;

/**
 * @brief Initializes new hash-table. Free with p_hash_table_free() after using.
 * @return Pointer to newly initialized #PHashTable structure in case of success,
 * NULL otherwise.
 * @since 0.0.1
 */
P_LIB_API PHashTable *	p_hash_table_new	(void);

/**
 * @brief Inserts new key-value pair into the hash-table.
 * @param table Initialized hash-table.
 * @param key Key value to insert.
 * @param value Value to insert.
 * @since 0.0.1
 *
 * This function only stores pointers, so you need to manually free pointed
 * data after using it.
 */
P_LIB_API void		p_hash_table_insert	(PHashTable *table, ppointer key, ppointer value);

/**
 * @brief Searches for specifed key in the hash-table.
 * @param table Hash-table to lookup in.
 * @param key Key to lookup for.
 * @return Value related to its key pair (can be NULL),
 * (ppointer) -1 if no value was found.
 * @since 0.0.1
 */
P_LIB_API ppointer	p_hash_table_lookup	(PHashTable *table, ppointer key);

/**
 * @brief Gives the list of all stored keys in hash-table.
 * @param table Hash-table to collect keys from.
 * @return List of all stores keys, list can be empty if no keys were found.
 * You should manually free this list with p_list_free() after using it.
 * @since 0.0.1
 */
P_LIB_API PList *	p_hash_table_keys	(PHashTable *table);

/**
 * @brief Gives the list of all stored values in hash-table.
 * @param table Hash-table to collect values from.
 * @return List of all stores values, list can be empty if no keys were found.
 * You should manually free this list with p_list_free() after using.
 * @since 0.0.1
 */
P_LIB_API PList *	p_hash_table_values	(PHashTable *table);

/**
 * @brief Frees previously initialized #PHashTable.
 * @param table Hash-table to free.
 * @since 0.0.1
 */
P_LIB_API void		p_hash_table_free	(PHashTable *table);

/**
 * @brief Removes @a key from hash table.
 * @param table Hash-table to remove key from.
 * @param key Key to remove (if exists).
 * @since 0.0.1
 */
P_LIB_API void		p_hash_table_remove	(PHashTable *table, ppointer key);

P_END_DECLS

#endif /* __PHASHTABLE_H__ */

