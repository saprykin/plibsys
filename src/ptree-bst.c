/*
 * Copyright (C) 2015 Alexander Saprykin <xelfium@gmail.com>
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

#include "pmem.h"
#include "ptree-bst.h"

pboolean
__p_tree_bst_insert (__PTreeBaseNode	**root_node,
		     PCompareDataFunc	compare_func,
		     ppointer		data,
		     PDestroyFunc	key_destroy_func,
		     PDestroyFunc	value_destroy_func,
		     ppointer		key,
		     ppointer		value)
{
	__PTreeBaseNode	**cur_node;
	pint		cmp_result;

	if (root_node == NULL || compare_func == NULL)
		return FALSE;

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
		if ((*cur_node = p_malloc0 (sizeof (__PTreeBaseNode))) == NULL)
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
__p_tree_bst_remove (__PTreeBaseNode	**root_node,
		     PCompareDataFunc	compare_func,
		     ppointer		data,
		     PDestroyFunc	key_destroy_func,
		     PDestroyFunc	value_destroy_func,
		     pconstpointer	key)
{
	__PTreeBaseNode	*cur_node;
	__PTreeBaseNode	*prev_node;
	__PTreeBaseNode	*prev_parent_node;
	__PTreeBaseNode	**parent_node;
	ppointer	new_key;
	ppointer	new_value;
	pint		cmp_result;
	pint		child_count;
	pboolean	result;

	if (root_node == NULL || *root_node == NULL || compare_func == NULL)
		return FALSE;

	cur_node	= *root_node;
	parent_node	= root_node;

	while (cur_node != NULL) {
		cmp_result = compare_func (key, cur_node->key, data);

		if (cmp_result < 0) {
			parent_node	= &cur_node->left;
			cur_node	= cur_node->left;
		} else if (cmp_result > 0) {
			parent_node	= &cur_node->right;
			cur_node	= cur_node->right;
		} else
			break;
	}

	if (cur_node == NULL)
		return FALSE;

	child_count = 0;

	if (cur_node->left != NULL)
		++child_count;

	if (cur_node->right != NULL)
		++child_count;

	if (child_count == 0)
		*parent_node = NULL;
	else if (child_count == 1)
		*parent_node = cur_node->left == NULL ? cur_node->right : cur_node->left;
	else {
		prev_node		= cur_node->right;
		prev_parent_node	= cur_node;

		while (prev_node->left != NULL) {
			prev_parent_node	= prev_node;
			prev_node		= prev_node->left;
		}

		new_key		= prev_node->key;
		new_value	= prev_node->value;

		result = __p_tree_bst_remove (&prev_parent_node,
					      compare_func,
					      data,
					      key_destroy_func,
					      value_destroy_func,
					      prev_node->key);

		cur_node->key	= new_key;
		cur_node->value	= new_value;

		return result;
	}

	if (key_destroy_func != NULL)
		key_destroy_func (cur_node->key);

	if (value_destroy_func != NULL)
		value_destroy_func (cur_node->value);

	p_free (cur_node);

	return TRUE;
}

void
__p_tree_bst_node_free (__PTreeBaseNode *node)
{
	p_free (node);
}
