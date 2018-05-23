/*
 * The MIT License
 *
 * Copyright (C) 2015-2016 Alexander Saprykin <saprykin.spb@gmail.com>
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
