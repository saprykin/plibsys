/*
 * Copyright (C) 2010-2016 Alexander Saprykin <xelfium@gmail.com>
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

/* Hash table organized like this: table[hash key]->[list with values]
 * Note: this implementation is not intended to use on huge loads */

#include "pmem.h"
#include "phashtable.h"

#include <stdlib.h>

typedef struct PHashTableNode_ PHashTableNode;

struct PHashTableNode_ {
	PHashTableNode	*next;
	ppointer	key;
	ppointer	value;
};

struct PHashTable_ {
	PHashTableNode	**table;
	psize		size;
};

/* Size of unique hash keys in hash table */
#define P_HASH_TABLE_SIZE 101

static puint pp_hash_table_calc_hash (pconstpointer pointer, psize modulo);
static PHashTableNode * pp_hash_table_find_node (const PHashTable *table, pconstpointer key);

static puint
pp_hash_table_calc_hash (pconstpointer pointer, psize modulo)
{
	/* As simple as we can :) */
	return (puint) (((psize) (P_POINTER_TO_INT (pointer) + 37)) % modulo);
}

static PHashTableNode *
pp_hash_table_find_node (const PHashTable *table, pconstpointer key)
{
	puint		hash;
	PHashTableNode	*ret;

	hash = pp_hash_table_calc_hash (key, table->size);

	for (ret = table->table[hash]; ret != NULL; ret = ret->next)
		if (ret->key == key)
			return ret;

	return NULL;
}

P_LIB_API PHashTable *
p_hash_table_new (void)
{
	PHashTable *ret;

	if (P_UNLIKELY ((ret = p_malloc0 (sizeof (PHashTable))) == NULL)) {
		P_ERROR ("PHashTable: failed to allocate memory");
		return NULL;
	}

	if (P_UNLIKELY ((ret->table = p_malloc0 (P_HASH_TABLE_SIZE * sizeof (PHashTableNode *))) == NULL)) {
		P_ERROR ("PHashTable: failed to allocate memory");
		p_free (ret);
		return NULL;
	}

	ret->size = P_HASH_TABLE_SIZE;

	return ret;
}

P_LIB_API void
p_hash_table_insert (PHashTable *table, ppointer key, ppointer value)
{
	PHashTableNode	*node;
	puint		hash;

	if (P_UNLIKELY (table == NULL))
		return;

	if ((node = pp_hash_table_find_node (table, key)) == NULL) {
		if (P_UNLIKELY ((node = p_malloc0 (sizeof (PHashTableNode))) == NULL)) {
			P_ERROR ("PHashTable: failed to allocate memory");
			return;
		}

		hash = pp_hash_table_calc_hash (key, table->size);

		/* Insert a new node in front of others */
		node->key   = key;
		node->value = value;
		node->next  = table->table[hash];

		table->table[hash] = node;
	} else
		node->value = value;
}

P_LIB_API ppointer
p_hash_table_lookup (const PHashTable *table, pconstpointer key)
{
	PHashTableNode *node;

	if (P_UNLIKELY (table == NULL))
		return NULL;

	return ((node = pp_hash_table_find_node (table, key)) == NULL) ? (ppointer) (-1) : node->value;
}

P_LIB_API PList *
p_hash_table_keys (const PHashTable *table)
{
	PList		*ret = NULL;
	PHashTableNode	*node;
	puint		i;

	if (P_UNLIKELY (table == NULL))
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

	if (P_UNLIKELY (table == NULL))
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

	if (P_UNLIKELY (table == NULL))
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

	if (P_UNLIKELY (table == NULL))
		return;

	if (pp_hash_table_find_node (table, key) != NULL) {
		hash = pp_hash_table_calc_hash (key, table->size);
		node = table->table[hash];
		prev_node = NULL;

		while (node != NULL) {
			if (node->key == key) {
				if (prev_node == NULL)
					table->table[hash] = node->next;
				else
					prev_node->next = node->next;

				p_free (node);
				break;
			} else {
				prev_node = node;
				node = node->next;
			}
		}
	}
}

P_LIB_API PList *
p_hash_table_lookup_by_value (const PHashTable *table, pconstpointer val, PCompareFunc func)
{
	PList		*ret = NULL;
	PHashTableNode	*node;
	puint		i;
	pboolean	res;

	if (P_UNLIKELY (table == NULL))
		return NULL;

	for (i = 0; i < table->size; ++i)
		for (node = table->table[i]; node != NULL; node = node->next) {
			if (func == NULL)
				res = (node->value == val);
			else
				res = (func (node->value, val) == 0);

			if (res)
				ret = p_list_append (ret, node->key);
		}

	return ret;
}
