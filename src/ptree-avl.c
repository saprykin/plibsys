/*
 * The MIT License
 *
 * Copyright (C) 2016 Alexander Saprykin <saprykin.spb@gmail.com>
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
#include "ptree-avl.h"

typedef struct PTreeAVLNode_ {
	struct PTreeBaseNode_	base;
	struct PTreeAVLNode_	*parent;
	pint			balance_factor;
} PTreeAVLNode;

static void pp_tree_avl_rotate_left (PTreeAVLNode *node, PTreeBaseNode **root);
static void pp_tree_avl_rotate_right (PTreeAVLNode *node, PTreeBaseNode **root);
static void pp_tree_avl_rotate_left_right (PTreeAVLNode *node, PTreeBaseNode **root);
static void pp_tree_avl_rotate_right_left (PTreeAVLNode *node, PTreeBaseNode **root);
static void pp_tree_avl_balance_insert (PTreeAVLNode *node, PTreeBaseNode **root);
static void pp_tree_avl_balance_remove (PTreeAVLNode *node, PTreeBaseNode **root);

static void
pp_tree_avl_rotate_left (PTreeAVLNode *node, PTreeBaseNode **root)
{
	node->parent->base.right = node->base.left;

	if (node->base.left != NULL)
		((PTreeAVLNode *) node->base.left)->parent = (PTreeAVLNode *) node->parent;

	node->base.left = (PTreeBaseNode *) node->parent;
	node->parent = ((PTreeAVLNode *) node->base.left)->parent;
	((PTreeAVLNode *) node->base.left)->parent = node;

	if (P_LIKELY (node->parent != NULL)) {
		if (node->parent->base.left == node->base.left)
			node->parent->base.left = (PTreeBaseNode *) node;
		else
			node->parent->base.right = (PTreeBaseNode *) node;
	} else
		*root = (PTreeBaseNode *) node;

	/* Restore balance factor */
	((PTreeAVLNode *) node)->balance_factor +=1;
	((PTreeAVLNode *) node->base.left)->balance_factor = -((PTreeAVLNode *) node)->balance_factor;
}

static void
pp_tree_avl_rotate_right (PTreeAVLNode *node, PTreeBaseNode **root)
{
	node->parent->base.left = node->base.right;

	if (node->base.right != NULL)
		((PTreeAVLNode *) node->base.right)->parent = (PTreeAVLNode *) node->parent;

	node->base.right = (PTreeBaseNode *) node->parent;
	node->parent = ((PTreeAVLNode *) node->base.right)->parent;
	((PTreeAVLNode *) node->base.right)->parent = node;

	if (P_LIKELY (node->parent != NULL)) {
		if (node->parent->base.left == node->base.right)
			node->parent->base.left = (PTreeBaseNode *) node;
		else
			node->parent->base.right = (PTreeBaseNode *) node;
	} else
		*root = (PTreeBaseNode *) node;

	/* Restore balance factor */
	((PTreeAVLNode *) node)->balance_factor -= 1;
	((PTreeAVLNode *) node->base.right)->balance_factor = -((PTreeAVLNode *) node)->balance_factor;
}

static void
pp_tree_avl_rotate_left_right (PTreeAVLNode *node, PTreeBaseNode **root)
{
	PTreeAVLNode *tmp_node;

	tmp_node = (PTreeAVLNode *) node->base.right;
	node->base.right = tmp_node->base.left;

	if (node->base.right != NULL)
		((PTreeAVLNode *) node->base.right)->parent = node;

	tmp_node->parent = node->parent->parent;

	if (P_LIKELY (tmp_node->parent != NULL)) {
		if (tmp_node->parent->base.left == (PTreeBaseNode *) node->parent)
			tmp_node->parent->base.left = (PTreeBaseNode *) tmp_node;
		else
			tmp_node->parent->base.right = (PTreeBaseNode *) tmp_node;
	} else
		*root = (PTreeBaseNode *) tmp_node;

	node->parent->base.left = tmp_node->base.right;

	if (node->parent->base.left != NULL)
		((PTreeAVLNode *) node->parent->base.left)->parent = node->parent;

	tmp_node->base.right = (PTreeBaseNode *) node->parent;
	((PTreeAVLNode *) tmp_node->base.right)->parent = tmp_node;

	tmp_node->base.left = (PTreeBaseNode *) node;
	node->parent = tmp_node;

	/* Restore balance factor */
	if (tmp_node->balance_factor == 1) {
		((PTreeAVLNode *) tmp_node->base.left)->balance_factor  = 0;
		((PTreeAVLNode *) tmp_node->base.right)->balance_factor = -1;
	} else if (tmp_node->balance_factor == -1) {
		((PTreeAVLNode *) tmp_node->base.left)->balance_factor  = 1;
		((PTreeAVLNode *) tmp_node->base.right)->balance_factor = 0;
	} else {
		((PTreeAVLNode *) tmp_node->base.left)->balance_factor  = 0;
		((PTreeAVLNode *) tmp_node->base.right)->balance_factor = 0;
	}

	tmp_node->balance_factor = 0;
}

static void
pp_tree_avl_rotate_right_left (PTreeAVLNode *node, PTreeBaseNode **root)
{
	PTreeAVLNode *tmp_node;

	tmp_node = (PTreeAVLNode *) node->base.left;
	node->base.left = tmp_node->base.right;

	if (node->base.left != NULL)
		((PTreeAVLNode *) node->base.left)->parent = node;

	tmp_node->parent = node->parent->parent;

	if (P_LIKELY (tmp_node->parent != NULL)) {
		if (tmp_node->parent->base.left == (PTreeBaseNode *) node->parent)
			tmp_node->parent->base.left = (PTreeBaseNode *) tmp_node;
		else
			tmp_node->parent->base.right = (PTreeBaseNode *) tmp_node;
	} else
		*root = (PTreeBaseNode *) tmp_node;

	node->parent->base.right = tmp_node->base.left;

	if (node->parent->base.right != NULL)
		((PTreeAVLNode *) node->parent->base.right)->parent = node->parent;

	tmp_node->base.left = (PTreeBaseNode *) node->parent;
	((PTreeAVLNode *) tmp_node->base.left)->parent = tmp_node;

	tmp_node->base.right = (PTreeBaseNode *) node;
	node->parent = tmp_node;

	/* Restore balance factor */
	if (tmp_node->balance_factor == 1) {
		((PTreeAVLNode *) tmp_node->base.left)->balance_factor  = 0;
		((PTreeAVLNode *) tmp_node->base.right)->balance_factor = -1;
	} else if (tmp_node->balance_factor == -1) {
		((PTreeAVLNode *) tmp_node->base.left)->balance_factor  = 1;
		((PTreeAVLNode *) tmp_node->base.right)->balance_factor = 0;
	} else {
		((PTreeAVLNode *) tmp_node->base.left)->balance_factor  = 0;
		((PTreeAVLNode *) tmp_node->base.right)->balance_factor = 0;
	}

	tmp_node->balance_factor = 0;
}

static void
pp_tree_avl_balance_insert (PTreeAVLNode *node, PTreeBaseNode **root)
{
	PTreeAVLNode *parent;

	while (TRUE) {
		parent = node->parent;

		if (P_UNLIKELY (parent == NULL))
			break;

		if (parent->base.left == (PTreeBaseNode *) node) {
			if (parent->balance_factor == 1) {
				if (node->balance_factor == -1)
					/* Case 1: Left-right rotate
					 *
					 *      (5)             (4)
					 *      / \            /   \
					 *    (3)  A   -->   (3)    (5)
					 *    / \            / \    / \
					 *   B  (4)         B   C  D   A
					 *      / \
					 *     C   D
					 */
					pp_tree_avl_rotate_left_right (node, root);
				else
					/* Case 2: Right rotate
					 *
					 *        (5)             (4)
					 *        / \            /   \
					 *      (4)  A   -->   (3)    (5)
					 *      / \            / \    / \
					 *    (3)  B          C   D  B   A
					 *    / \
					 *   C   D
					 */
					pp_tree_avl_rotate_right (node, root);

				break;
			} else if (parent->balance_factor == -1) {
				/* Case 3: Increase parent balance factor */
				parent->balance_factor = 0;
				break;
			} else
				/* Case 4: Increase parent balance factor */
				parent->balance_factor = 1;
		} else {
			if (parent->balance_factor == -1) {
				if (node->balance_factor == 1)
					/* Case 1: Right-left rotate
					 *
					 *    (3)               (4)
					 *    / \              /   \
					 *   A  (5)    -->   (3)    (5)
					 *      / \          / \    / \
					 *    (4)  B        A   C  D   B
					 *    / \
					 *   C   D
					 */
					pp_tree_avl_rotate_right_left (node, root);
				else
					/* Case 2: Left rotate
					 *
					 *    (3)              (4)
					 *    / \             /   \
					 *   A  (4)   -->   (3)    (5)
					 *      / \         / \    / \
					 *     B  (5)      A   B  C   D
					 *        / \
					 *       C   D
					 */
					pp_tree_avl_rotate_left (node, root);

				break;
			} else if (parent->balance_factor == 1) {
				/* Case 3: Decrease parent balance factor */
				parent->balance_factor = 0;
				break;
			} else
				/* Case 4: Decrease parent balance factor */
				parent->balance_factor = -1;
		}

		node = node->parent;
	}
}

static void
pp_tree_avl_balance_remove (PTreeAVLNode *node, PTreeBaseNode **root)
{
	PTreeAVLNode	*parent;
	PTreeAVLNode	*sibling;
	pint		sibling_balance;

	while (TRUE) {
		parent = node->parent;

		if (P_UNLIKELY (parent == NULL))
			break;

		if (parent->base.left == (PTreeBaseNode *) node) {
			if (parent->balance_factor == -1) {
				sibling         = (PTreeAVLNode *) parent->base.right;
				sibling_balance = sibling->balance_factor;

				if (sibling->balance_factor == 1)
					/* Case 1 */
					pp_tree_avl_rotate_right_left (sibling, root);
				 else
					/* Case 2 */
					pp_tree_avl_rotate_left (sibling, root);

				node = parent;

				if (sibling_balance == 0)
					break;
			} else if (parent->balance_factor == 0) {
				/* Case 3 */
				parent->balance_factor = -1;
				break;
			} else
				/* Case 4 */
				parent->balance_factor = 0;
		} else {
			if (parent->balance_factor == 1) {
				sibling         = (PTreeAVLNode *) parent->base.left;
				sibling_balance = sibling->balance_factor;

				if (sibling->balance_factor == -1)
					/* Case 1 */
					pp_tree_avl_rotate_left_right (sibling, root);
				else
					/* Case 2 */
					pp_tree_avl_rotate_right (sibling, root);

				node = parent;

				if (sibling_balance == 0)
					break;
			} else if (parent->balance_factor == 0) {
				/* Case 3 */
				parent->balance_factor = 1;
				break;
			} else
				/* Case 4 */
				parent->balance_factor = 0;
		}

		node = node->parent;
	}
}

pboolean
p_tree_avl_insert (PTreeBaseNode	**root_node,
		   PCompareDataFunc	compare_func,
		   ppointer		data,
		   PDestroyFunc		key_destroy_func,
		   PDestroyFunc		value_destroy_func,
		   ppointer		key,
		   ppointer		value)
{
	PTreeBaseNode	**cur_node;
	PTreeBaseNode	*parent_node;
	pint		cmp_result;

	cur_node    = root_node;
	parent_node = *root_node;

	/* Find where to insert the node */
	while (*cur_node != NULL) {
		cmp_result = compare_func (key, (*cur_node)->key, data);

		if (cmp_result < 0) {
			parent_node = *cur_node;
			cur_node    = &(*cur_node)->left;
		} else if (cmp_result > 0) {
			parent_node = *cur_node;
			cur_node    = &(*cur_node)->right;
		} else
			break;
	}

	/* If we have existing one - replace a key-value pair */
	if (*cur_node != NULL) {
		if (key_destroy_func != NULL)
			key_destroy_func ((*cur_node)->key);

		if (value_destroy_func != NULL)
			value_destroy_func ((*cur_node)->value);

		(*cur_node)->key   = key;
		(*cur_node)->value = value;

		return FALSE;
	}

	if (P_UNLIKELY ((*cur_node = p_malloc0 (sizeof (PTreeAVLNode))) == NULL))
		return FALSE;

	(*cur_node)->key   = key;
	(*cur_node)->value = value;

	((PTreeAVLNode *) *cur_node)->balance_factor = 0;
	((PTreeAVLNode *) *cur_node)->parent         = (PTreeAVLNode *) parent_node;

	/* Balance the tree */
	pp_tree_avl_balance_insert (((PTreeAVLNode *) *cur_node), root_node);

	return TRUE;
}

pboolean
p_tree_avl_remove (PTreeBaseNode	**root_node,
		   PCompareDataFunc	compare_func,
		   ppointer		data,
		   PDestroyFunc		key_destroy_func,
		   PDestroyFunc		value_destroy_func,
		   pconstpointer	key)
{
	PTreeBaseNode	*cur_node;
	PTreeBaseNode	*prev_node;
	PTreeBaseNode	*child_node;
	PTreeAVLNode	*child_parent;
	pint		cmp_result;

	cur_node = *root_node;

	while (cur_node != NULL) {
		cmp_result = compare_func (key, cur_node->key, data);

		if (cmp_result < 0)
			cur_node = cur_node->left;
		else if (cmp_result > 0)
			cur_node = cur_node->right;
		else
			break;
	}

	if (P_UNLIKELY (cur_node == NULL))
		return FALSE;

	if (cur_node->left != NULL && cur_node->right != NULL) {
		prev_node = cur_node->left;

		while (prev_node->right != NULL)
			prev_node = prev_node->right;

		cur_node->key   = prev_node->key;
		cur_node->value = prev_node->value;

		/* Mark node for removal */
		cur_node = prev_node;
	}

	child_node = cur_node->left == NULL ? cur_node->right : cur_node->left;

	if (child_node == NULL)
		pp_tree_avl_balance_remove ((PTreeAVLNode *) cur_node, root_node);

	/* Replace node with its child */
	if (P_UNLIKELY (cur_node == *root_node)) {
		*root_node   = child_node;
		child_parent = NULL;
	} else {
		child_parent = ((PTreeAVLNode *) cur_node)->parent;

		if (child_parent->base.left == cur_node)
			child_parent->base.left = child_node;
		else
			child_parent->base.right = child_node;
	}

	if (child_node != NULL) {
		((PTreeAVLNode *) child_node)->parent = child_parent;

		/* Balance the tree */
		pp_tree_avl_balance_remove ((PTreeAVLNode *) child_node, root_node);
	}

	/* Free unused node */
	if (key_destroy_func != NULL)
		key_destroy_func (cur_node->key);

	if (value_destroy_func != NULL)
		value_destroy_func (cur_node->value);

	p_free (cur_node);

	return TRUE;
}

void
p_tree_avl_node_free (PTreeBaseNode *node)
{
	p_free (node);
}
