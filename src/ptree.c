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
#include "ptree.h"
#include "ptree-avl.h"
#include "ptree-bst.h"
#include "ptree-rb.h"

typedef pboolean	(*__PTreeInsertNode)	(__PTreeBaseNode	**root_node,
						 PCompareDataFunc	compare_func,
						 ppointer		data,
						 PDestroyFunc		key_destroy_func,
						 PDestroyFunc		value_destroy_func,
						 ppointer		key,
						 ppointer		value);

typedef pboolean	(*__PTreeRemoveNode)	(__PTreeBaseNode	**root_node,
						 PCompareDataFunc	compare_func,
						 ppointer		data,
						 PDestroyFunc		key_destroy_func,
						 PDestroyFunc		value_destroy_func,
						 pconstpointer		key);

typedef void		(*__PTreeFreeNode)	(__PTreeBaseNode	*node);

struct _PTree {
	__PTreeBaseNode		*root;
	__PTreeInsertNode	insert_node_func;
	__PTreeRemoveNode	remove_node_func;
	__PTreeFreeNode		free_node_func;
	PDestroyFunc		key_destroy_func;
	PDestroyFunc		value_destroy_func;
	PCompareDataFunc	compare_func;
	ppointer		data;
	PTreeType		type;
	pint			nnodes;
};

P_LIB_API PTree *
p_tree_new (PTreeType		type,
	    PCompareFunc	func)
{
	return p_tree_new_full (type, (PCompareDataFunc) func, NULL, NULL, NULL);
}

P_LIB_API PTree *
p_tree_new_with_data (PTreeType		type,
		      PCompareDataFunc	func,
		      ppointer		data)
{
	return p_tree_new_full (type, func, data, NULL, NULL);
}

P_LIB_API PTree *
p_tree_new_full (PTreeType		type,
		 PCompareDataFunc	func,
		 ppointer		data,
		 PDestroyFunc		key_destroy,
		 PDestroyFunc		value_destroy)
{
	PTree *ret;

	if (P_UNLIKELY (type < P_TREE_TYPE_BINARY || type > P_TREE_TYPE_AVL))
		return NULL;

	if (P_UNLIKELY (func == NULL))
		return NULL;

	if (P_UNLIKELY ((ret = p_malloc0 (sizeof (PTree))) == NULL))
		return NULL;

	ret->type               = type;
	ret->compare_func       = func;
	ret->data               = data;
	ret->key_destroy_func   = key_destroy;
	ret->value_destroy_func	= value_destroy;

	switch (type) {
	case P_TREE_TYPE_BINARY:
		ret->insert_node_func = __p_tree_bst_insert;
		ret->remove_node_func = __p_tree_bst_remove;
		ret->free_node_func   = __p_tree_bst_node_free;
		break;
	case P_TREE_TYPE_RB:
		ret->insert_node_func = __p_tree_rb_insert;
		ret->remove_node_func = __p_tree_rb_remove;
		ret->free_node_func   = __p_tree_rb_node_free;
		break;
	case P_TREE_TYPE_AVL:
		ret->insert_node_func = __p_tree_avl_insert;
		ret->remove_node_func = __p_tree_avl_remove;
		ret->free_node_func   = __p_tree_avl_node_free;
		break;
	default:
		p_free (ret);
		return NULL;
	}

	return ret;
}

P_LIB_API void
p_tree_insert (PTree	*tree,
	       ppointer	key,
	       ppointer	value)
{
	pboolean result;

	if (P_UNLIKELY (tree == NULL))
		return;

	result = tree->insert_node_func (&tree->root,
					 tree->compare_func,
					 tree->data,
					 tree->key_destroy_func,
					 tree->value_destroy_func,
					 key,
					 value);

	if (result == TRUE)
		++tree->nnodes;
}

P_LIB_API pboolean
p_tree_remove (PTree		*tree,
	       pconstpointer	key)
{
	pboolean result;

	if (P_UNLIKELY (tree == NULL || tree->root == NULL))
		return FALSE;

	result = tree->remove_node_func (&tree->root,
					 tree->compare_func,
					 tree->data,
					 tree->key_destroy_func,
					 tree->value_destroy_func,
					 key);
	if (result == TRUE)
		--tree->nnodes;

	return result;
}

P_LIB_API ppointer
p_tree_lookup (PTree		*tree,
	       pconstpointer	key)
{
	__PTreeBaseNode	*cur_node;
	pint		cmp_result;

	if (P_UNLIKELY (tree == NULL))
		return NULL;

	cur_node = tree->root;

	while (cur_node != NULL) {
		cmp_result = tree->compare_func (key, cur_node->key, tree->data);

		if (cmp_result < 0)
			cur_node = cur_node->left;
		else if (cmp_result > 0)
			cur_node = cur_node->right;
		else
			return cur_node->value;
	}

	return NULL;
}

P_LIB_API void
p_tree_foreach (PTree		*tree,
		PTraverseFunc	traverse_func,
		ppointer	user_data)
{
	__PTreeBaseNode	*cur_node;
	__PTreeBaseNode	*prev_node;
	pint		mod_counter;
	pboolean	need_stop;

	if (P_UNLIKELY (tree == NULL || traverse_func == NULL))
		return;

	if (P_UNLIKELY (tree->root == NULL))
		return;

	cur_node    = tree->root;
	mod_counter = 0;
	need_stop   = FALSE;

	while (cur_node != NULL) {
		if (cur_node->left == NULL) {
			if (need_stop == FALSE)
				need_stop = traverse_func (cur_node->key,
							   cur_node->value,
							   user_data);

			cur_node = cur_node->right;
		} else {
			prev_node = cur_node->left;

			while (prev_node->right != NULL && prev_node->right != cur_node)
				prev_node = prev_node->right;

			if (prev_node->right == NULL) {
				prev_node->right = cur_node;
				cur_node         = cur_node->left;

				++mod_counter;
			} else {
				if (need_stop == FALSE)
					need_stop = traverse_func (cur_node->key,
								   cur_node->value,
								   user_data);

				cur_node         = cur_node->right;
				prev_node->right = NULL;

				--mod_counter;

				if (need_stop == TRUE && mod_counter == 0)
					return;
			}
		}
	}
}

P_LIB_API void
p_tree_clear (PTree *tree)
{
	__PTreeBaseNode	*cur_node;
	__PTreeBaseNode	*prev_node;
	__PTreeBaseNode	*next_node;

	if (P_UNLIKELY (tree == NULL || tree->root == NULL))
		return;

	cur_node = tree->root;

	while (cur_node != NULL) {
		if (cur_node->left == NULL) {
			next_node = cur_node->right;

			if (tree->key_destroy_func != NULL)
				tree->key_destroy_func (cur_node->key);

			if (tree->value_destroy_func != NULL)
				tree->value_destroy_func (cur_node->value);

			tree->free_node_func (cur_node);
			--tree->nnodes;

			cur_node = next_node;
		} else {
			prev_node = cur_node->left;

			while (prev_node->right != NULL)
				prev_node = prev_node->right;

			prev_node->right = cur_node;
			next_node        = cur_node->left;
			cur_node->left   = NULL;
			cur_node         = next_node;
		}
	}

	tree->root = NULL;
}

P_LIB_API PTreeType
p_tree_get_type (const PTree *tree)
{
	if (P_UNLIKELY (tree == NULL))
		return (PTreeType) -1;

	return tree->type;
}

P_LIB_API pint
p_tree_get_nnodes (const PTree *tree)
{
	if (P_UNLIKELY (tree == NULL))
		return 0;

	return tree->nnodes;
}

P_LIB_API void
p_tree_free (PTree *tree)
{
	p_tree_clear (tree);
	p_free (tree);
}
