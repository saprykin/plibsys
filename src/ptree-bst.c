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

#include "pmem.h"
#include "ptree-bst.h"

pboolean
p_tree_bst_insert (PTreeBaseNode	**root_node,
		   PCompareDataFunc	compare_func,
		   ppointer		data,
		   PDestroyFunc		key_destroy_func,
		   PDestroyFunc		value_destroy_func,
		   ppointer		key,
		   ppointer		value)
{
	PTreeBaseNode	**cur_node;
	pint		cmp_result;

	cur_node = root_node;

	while (*cur_node != NULL) {
		cmp_result = compare_func (key, (*cur_node)->key, data);

		if (cmp_result < 0)
			cur_node = &(*cur_node)->left;
		else if (cmp_result > 0)
			cur_node = &(*cur_node)->right;
		else
			break;
	}

	if ((*cur_node) == NULL) {
		if (P_UNLIKELY ((*cur_node = p_malloc0 (sizeof (PTreeBaseNode))) == NULL))
			return FALSE;

		(*cur_node)->key   = key;
		(*cur_node)->value = value;

		return TRUE;
	} else {
		if (key_destroy_func != NULL)
			key_destroy_func ((*cur_node)->key);

		if (value_destroy_func != NULL)
			value_destroy_func ((*cur_node)->value);

		(*cur_node)->key   = key;
		(*cur_node)->value = value;

		return FALSE;
	}
}

pboolean
p_tree_bst_remove (PTreeBaseNode	**root_node,
		   PCompareDataFunc	compare_func,
		   ppointer		data,
		   PDestroyFunc		key_destroy_func,
		   PDestroyFunc		value_destroy_func,
		   pconstpointer	key)
{
	PTreeBaseNode	*cur_node;
	PTreeBaseNode	*prev_node;
	PTreeBaseNode	**node_pointer;
	pint		cmp_result;

	cur_node     = *root_node;
	node_pointer = root_node;

	while (cur_node != NULL) {
		cmp_result = compare_func (key, cur_node->key, data);

		if (cmp_result < 0) {
			node_pointer = &cur_node->left;
			cur_node     = cur_node->left;
		} else if (cmp_result > 0) {
			node_pointer = &cur_node->right;
			cur_node     = cur_node->right;
		} else
			break;
	}

	if (P_UNLIKELY (cur_node == NULL))
		return FALSE;

	if (cur_node->left != NULL && cur_node->right != NULL) {
		node_pointer = &cur_node->left;
		prev_node    = cur_node->left;

		while (prev_node->right != NULL) {
			node_pointer = &prev_node->right;
			prev_node    = prev_node->right;
		}

		cur_node->key   = prev_node->key;
		cur_node->value = prev_node->value;

		cur_node = prev_node;
	}

	*node_pointer = cur_node->left == NULL ? cur_node->right : cur_node->left;

	if (key_destroy_func != NULL)
		key_destroy_func (cur_node->key);

	if (value_destroy_func != NULL)
		value_destroy_func (cur_node->value);

	p_free (cur_node);

	return TRUE;
}

void
p_tree_bst_node_free (PTreeBaseNode *node)
{
	p_free (node);
}
