/*
 * The MIT License
 *
 * Copyright (C) 2010-2016 Alexander Saprykin <saprykin.spb@gmail.com>
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
 * @file phashtable.h
 * @brief Hash table
 * @author Alexander Saprykin
 *
 * A hash table is a data structure used to map keys to values. The hash table
 * consists of several internal slots which hold a list of values. A hash
 * function is used to compute an index in the array of the slots from a given
 * key. The hash function itself is fast and it takes a constant time to compute
 * the internal slot index.
 *
 * When the number of pairs in the hash table is small the lookup and insert
 * (remove) operations are very fast and have average complexity O(1), because
 * every slot holds almost the only one pair. As the number of internal slots is
 * fixed, the increasing number of pairs will lead to degraded performance and
 * the average complexity of the operations can drop to O(N) in the worst case.
 * This is because the more pairs are inserted the more longer the list of
 * values is placed in every slot.
 *
 * This is a simple hash table implementation which is not intended for heavy
 * usage (several thousands), see #PTree if you need the best performance on
 * large data sets. This implementation doesn't support multi-inserts when
 * several values belong to the same key.
 *
 * Note that #PHashTable stores keys and values only as pointers, so you need
 * to free used memory manually, p_hash_table_free() will not do it in any way.
 *
 * Integers (up to 32 bits) can be stored in pointers using #P_POINTER_TO_INT
 * and #P_INT_TO_POINTER macros.
 */

#if !defined (PLIBSYS_H_INSIDE) && !defined (PLIBSYS_COMPILATION)
#  error "Header files shouldn't be included directly, consider using <plibsys.h> instead."
#endif

#ifndef PLIBSYS_HEADER_PHASHTABLE_H
#define PLIBSYS_HEADER_PHASHTABLE_H

#include <pmacros.h>
#include <ptypes.h>
#include <plist.h>

P_BEGIN_DECLS

/** Opaque data structure for a hash table. */
typedef struct PHashTable_ PHashTable;

/**
 * @brief Initializes a new hash table.
 * @return Pointer to a	 newly initialized #PHashTable structure in case of
 * success, NULL otherwise.
 * @since 0.0.1
 * @note Free with p_hash_table_free() after usage.
 */
P_LIB_API PHashTable *	p_hash_table_new		(void);

/**
 * @brief Inserts a new key-value pair into a hash table.
 * @param table Initialized hash table.
 * @param key Key to insert.
 * @param value Value to insert.
 * @since 0.0.1
 *
 * This function only stores pointers, so you need to manually free pointed
 * data after using the hash table.
 */
P_LIB_API void		p_hash_table_insert		(PHashTable		*table,
							 ppointer		key,
							 ppointer		value);

/**
 * @brief Searches for a specifed key in the hash table.
 * @param table Hash table to lookup in.
 * @param key Key to lookup for.
 * @return Value related to its key pair (can be NULL), (#ppointer) -1 if no
 * value was found.
 * @since 0.0.1
 */
P_LIB_API ppointer	p_hash_table_lookup		(const PHashTable	*table,
							 pconstpointer		key);

/**
 * @brief Gives a list of all the stored keys in the hash table.
 * @param table Hash table to collect the keys from.
 * @return List of all the stored keys, the list can be empty if no keys were
 * found.
 * @since 0.0.1
 * @note You should manually free the returned list with p_list_free() after
 * using it.
 */
P_LIB_API PList *	p_hash_table_keys		(const PHashTable	*table);

/**
 * @brief Gives a list of all the stored values in the hash table.
 * @param table Hash table to collect the values from.
 * @return List of all the stored values, the list can be empty if no keys were
 * found.
 * @since 0.0.1
 * @note You should manually free the returned list with p_list_free() after
 * using it.
 */
P_LIB_API PList *	p_hash_table_values		(const PHashTable	*table);

/**
 * @brief Frees a previously initialized #PHashTable.
 * @param table Hash table to free.
 * @since 0.0.1
 */
P_LIB_API void		p_hash_table_free		(PHashTable		*table);

/**
 * @brief Removes @a key from a hash table.
 * @param table Hash table to remove the key from.
 * @param key Key to remove (if exists).
 * @since 0.0.1
 */
P_LIB_API void		p_hash_table_remove		(PHashTable		*table,
							 pconstpointer		key);

/**
 * @brief Searches for a specifed key in the hash table by its value.
 * @param table Hash table to lookup in.
 * @param val Value to lookup keys for.
 * @param func Function to compare table's values with @a val, if NULL then
 * values will be compared as pointers.
 * @return List of the keys with @a val (can be NULL), NULL if no keys were
 * found.
 * @since 0.0.1
 * @note Caller is responsible to call p_list_free() on the returned list after
 * usage.
 *
 * The compare function should return 0 if a value from the hash table (the
 * first parameter) is accepted related to the given lookup value (the second
 * parameter), and -1 or 1 otherwise.
 */
P_LIB_API PList *	p_hash_table_lookup_by_value	(const PHashTable	*table,
							 pconstpointer		val,
							 PCompareFunc		func);

P_END_DECLS

#endif /* PLIBSYS_HEADER_PHASHTABLE_H */
