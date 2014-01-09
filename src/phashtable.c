/* 
 * Copyright (C) 2010-2013 Alexander Saprykin <xelfium@gmail.com>
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

/* Hash table organized like this: table[hash key]->[list with values]
 * Note: this implementation is not intended to use on huge loads */

#include "pmem.h"
#include "phashtable.h"

#include <stdlib.h>

typedef struct _PHashTableNode PHashTableNode;

struct _PHashTableNode {
	PHashTableNode *next;
	ppointer key;
	ppointer value;
};

struct _PHashTable {
	PHashTableNode **table;
	psize size;
};

/* Size of unique hash keys in hash table */
#define P_HASH_SIZE 101

static puint __p_hash_table_calc_hash (pconstpointer pointer, psize modulo);
static PHashTableNode * __p_hash_table_find_node (const PHashTable *table, pconstpointer key);

static puint
__p_hash_table_calc_hash (pconstpointer pointer, psize modulo)
{
	if (modulo == 0)
		return 0;

	/* As simple as we can :) */
	return (puint) ((P_POINTER_TO_INT (pointer) + 37) % modulo);
}

static PHashTableNode * 
__p_hash_table_find_node (const PHashTable *table, pconstpointer key)
{
	puint		hash;
	PHashTableNode	*ret;

	if (table == NULL)
		return NULL;

	hash = __p_hash_table_calc_hash (key, table->size);
 
	for (ret = table->table[hash]; ret != NULL; ret = ret->next)
		if (ret->key == key)
			return ret;

	return NULL;
}

P_LIB_API PHashTable * 
p_hash_table_new (void)
{
	PHashTable *ret;

	if ((ret = p_malloc0 (sizeof (PHashTable))) == NULL){
		P_ERROR ("PHashTable: failed to allocate memory");
		return NULL;
	}

	if ((ret->table = p_malloc0 (P_HASH_SIZE * sizeof (PHashTableNode *))) == NULL) {
		P_ERROR ("PHashTable: failed to allocate memory");
		p_free (ret);
		return NULL;
	}

	ret->size = P_HASH_SIZE;

	return ret;
}

P_LIB_API void 
p_hash_table_insert (PHashTable *table, ppointer key, ppointer value)
{
	PHashTableNode	*node;
	puint		hash;

	if (table == NULL)
		return;

	if ((node = __p_hash_table_find_node (table, key)) == NULL) {
		if ((node = p_malloc0 (sizeof (PHashTableNode))) == NULL) {
			P_ERROR ("PHashTable: failed to allocate memory");
			return;
		}
		
		hash = __p_hash_table_calc_hash (key, table->size);
		
		/* Insert new node in front of other */
		node->key = key;
		node->value = value;
		node->next = table->table[hash];
		table->table[hash] = node;
	} else
		node->value = value;
}

P_LIB_API ppointer
p_hash_table_lookup (const PHashTable *table, pconstpointer key)
{
	PHashTableNode *node;

	if (table == NULL)
		return NULL;

	return ((node = __p_hash_table_find_node (table, key)) == NULL) ? (ppointer) (-1) : node->value;
}

P_LIB_API PList *
p_hash_table_keys (const PHashTable *table)
{
	PList		*ret = NULL;
	PHashTableNode	*node;
	puint		i;

	if (table == NULL)
		return NULL;

	for (i = 0; i < table->size; ++i)
		for (node = table->table[i]; node != NULL; node = node->next)
			ret = p_list_append (ret, node->key);

	return ret;
}

P_LIB_API PList *
p_hash_table_values (const PHashTable *table)
{
	PList		*ret = NULL;
	PHashTableNode	*node;
	puint		i;

	if (table == NULL)
		return NULL;

	for (i = 0; i < table->size; ++i)
		for (node = table->table[i]; node != NULL; node = node->next)
			ret = p_list_append (ret, node->value);

	return ret;
}

P_LIB_API void
p_hash_table_free (PHashTable *table)
{
	PHashTableNode	*node, *next_node;
	puint		i;

	if (table == NULL)
		return;

	for (i = 0; i < table->size; ++i)
		for (node = table->table[i]; node != NULL; ) {
			next_node = node->next;
			p_free (node);
			node = next_node;
		}

	p_free (table->table);
	p_free (table);
}

P_LIB_API void
p_hash_table_remove (PHashTable *table, pconstpointer key)
{
	PHashTableNode	*node, *prev_node;
	puint		hash;

	if (table == NULL)
		return;

	if (__p_hash_table_find_node (table, key) != NULL) {
		hash = __p_hash_table_calc_hash (key, table->size);
		node = table->table[hash];
		prev_node = NULL;

		while (node != NULL) {
			if (node->key == key) {
				if (prev_node == NULL)
					table->table[hash] = node->next;
				else
					prev_node->next = node->next;

				p_free (node);
				return;
			} else {
				prev_node = node;
				node = node->next;
			}
		}
	} else
		return;
}

P_LIB_API PList *
p_hash_table_lookup_by_value (const PHashTable *table, pconstpointer val, PCompareFunc func)
{
	PList		*ret = NULL;
	PHashTableNode	*node;
	unsigned int	i;
	pboolean	res;

	if (table == NULL)
		return NULL;

	for (i = 0; i < table->size; ++i)
		for (node = table->table[i]; node != NULL; node = node->next) {
			if (func == NULL)
				res = (node->value == val);
			else
				res = (func (node->value, val, NULL) == 0);
			
			if (res)
				ret = p_list_append (ret, node->key);
		}

	return ret;
}
