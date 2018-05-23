/*
 * The MIT License
 *
 * Copyright (C) 2015-2017 Alexander Saprykin <saprykin.spb@gmail.com>
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

#include "pmem.h"
#include "ptree.h"
#include "ptree-avl.h"
#include "ptree-bst.h"
#include "ptree-rb.h"

typedef pboolean	(*PTreeInsertNode)	(PTreeBaseNode		**root_node,
						 PCompareDataFunc	compare_func,
						 ppointer		data,
						 PDestroyFunc		key_destroy_func,
						 PDestroyFunc		value_destroy_func,
						 ppointer		key,
						 ppointer		value);

typedef pboolean	(*PTreeRemoveNode)	(PTreeBaseNode		**root_node,
						 PCompareDataFunc	compare_func,
						 ppointer		data,
						 PDestroyFunc		key_destroy_func,
						 PDestroyFunc		value_destroy_func,
						 pconstpointer		key);

typedef void		(*PTreeFreeNode)	(PTreeBaseNode	*node);

struct PTree_ {
	PTreeBaseNode		*root;
	PTreeInsertNode		insert_node_func;
	PTreeRemoveNode		remove_node_func;
	PTreeFreeNode		free_node_func;
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

	if (P_UNLIKELY (!(type >= P_TREE_TYPE_BINARY && type <= P_TREE_TYPE_AVL)))
		return NULL;

	if (P_UNLIKELY (func == NULL))
		return NULL;

	if (P_UNLIKELY ((ret = p_malloc0 (sizeof (PTree))) == NULL)) {
		P_ERROR ("PTree::p_tree_new_full: failed to allocate memory");
		return NULL;
	}

	ret->type               = type;
	ret->compare_func       = func;
	ret->data               = data;
	ret->key_destroy_func   = key_destroy;
	ret->value_destroy_func	= value_destroy;

	switch (type) {
	case P_TREE_TYPE_BINARY:
		ret->insert_node_func = p_tree_bst_insert;
		ret->remove_node_func = p_tree_bst_remove;
		ret->free_node_func   = p_tree_bst_node_free;
		break;
	case P_TREE_TYPE_RB:
		ret->insert_node_func = p_tree_rb_insert;
		ret->remove_node_func = p_tree_rb_remove;
		ret->free_node_func   = p_tree_rb_node_free;
		break;
	case P_TREE_TYPE_AVL:
		ret->insert_node_func = p_tree_avl_insert;
		ret->remove_node_func = p_tree_avl_remove;
		ret->free_node_func   = p_tree_avl_node_free;
		break;
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
	PTreeBaseNode	*cur_node;
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
	PTreeBaseNode	*cur_node;
	PTreeBaseNode	*prev_node;
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
	PTreeBaseNode	*cur_node;
	PTreeBaseNode	*prev_node;
	PTreeBaseNode	*next_node;

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
