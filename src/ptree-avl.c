/*
 * Copyright (C) 2016 Alexander Saprykin <xelfium@gmail.com>
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
#include "ptree-avl.h"

typedef struct _PTreeAVLNode {
	struct _PTreeBaseNode	base;
	struct _PTreeAVLNode	*parent;
	pint			balance_factor;
} __PTreeAVLNode;

static void
__p_tree_avl_rotate_left (__PTreeAVLNode *node, __PTreeBaseNode **root)
{
	node->parent->base.right = node->base.left;

	if (node->base.left != NULL)
		((__PTreeAVLNode *) node->base.left)->parent = (__PTreeAVLNode *) node->parent;

	node->base.left = (__PTreeBaseNode *) node->parent;
	node->parent = ((__PTreeAVLNode *) node->base.left)->parent;
	((__PTreeAVLNode *) node->base.left)->parent = node;

	if (P_LIKELY (node->parent != NULL)) {
		if (node->parent->base.left == node->base.left)
			node->parent->base.left = (__PTreeBaseNode *) node;
		else
			node->parent->base.right = (__PTreeBaseNode *) node;
	} else
		*root = (__PTreeBaseNode *) node;

	/* Restore balance factor */
	((__PTreeAVLNode *) node)->balance_factor +=1;
	((__PTreeAVLNode *) node->base.left)->balance_factor = -((__PTreeAVLNode *) node)->balance_factor;
}

static void
__p_tree_avl_rotate_right (__PTreeAVLNode *node, __PTreeBaseNode **root)
{
	node->parent->base.left = node->base.right;

	if (node->base.right != NULL)
		((__PTreeAVLNode *) node->base.right)->parent = (__PTreeAVLNode *) node->parent;

	node->base.right = (__PTreeBaseNode *) node->parent;
	node->parent = ((__PTreeAVLNode *) node->base.right)->parent;
	((__PTreeAVLNode *) node->base.right)->parent = node;

	if (P_LIKELY (node->parent != NULL)) {
		if (node->parent->base.left == node->base.right)
			node->parent->base.left = (__PTreeBaseNode *) node;
		else
			node->parent->base.right = (__PTreeBaseNode *) node;
	} else
		*root = (__PTreeBaseNode *) node;

	/* Restore balance factor */
	((__PTreeAVLNode *) node)->balance_factor -= 1;
	((__PTreeAVLNode *) node->base.right)->balance_factor = -((__PTreeAVLNode *) node)->balance_factor;
}

static void
__p_tree_avl_rotate_left_right (__PTreeAVLNode *node, __PTreeBaseNode **root)
{
	__PTreeAVLNode *tmp_node;

	tmp_node = (__PTreeAVLNode *) node->base.right;
	node->base.right = tmp_node->base.left;

	if (node->base.right != NULL)
		((__PTreeAVLNode *) node->base.right)->parent = node;

	tmp_node->parent = node->parent->parent;

	if (P_LIKELY (tmp_node->parent != NULL)) {
		if (tmp_node->parent->base.left == (__PTreeBaseNode *) node->parent)
			tmp_node->parent->base.left = (__PTreeBaseNode *) tmp_node;
		else
			tmp_node->parent->base.right = (__PTreeBaseNode *) tmp_node;
	} else
		*root = (__PTreeBaseNode *) tmp_node;

	node->parent->base.left = tmp_node->base.right;

	if (node->parent->base.left != NULL)
		((__PTreeAVLNode *) node->parent->base.left)->parent = node->parent;

	tmp_node->base.right = (__PTreeBaseNode *) node->parent;
	((__PTreeAVLNode *) tmp_node->base.right)->parent = tmp_node;

	tmp_node->base.left = (__PTreeBaseNode *) node;
	node->parent = tmp_node;

	/* Restore balance factor */
	if (tmp_node->balance_factor == 1) {
		((__PTreeAVLNode *) tmp_node->base.left)->balance_factor  = 0;
		((__PTreeAVLNode *) tmp_node->base.right)->balance_factor = -1;
	} else if (tmp_node->balance_factor == -1) {
		((__PTreeAVLNode *) tmp_node->base.left)->balance_factor  = 1;
		((__PTreeAVLNode *) tmp_node->base.right)->balance_factor = 0;
	} else {
		((__PTreeAVLNode *) tmp_node->base.left)->balance_factor  = 0;
		((__PTreeAVLNode *) tmp_node->base.right)->balance_factor = 0;
	}

	tmp_node->balance_factor = 0;
}

static void
__p_tree_avl_rotate_right_left (__PTreeAVLNode *node, __PTreeBaseNode **root)
{
	__PTreeAVLNode *tmp_node;

	tmp_node = (__PTreeAVLNode *) node->base.left;
	node->base.left = tmp_node->base.right;

	if (node->base.left != NULL)
		((__PTreeAVLNode *) node->base.left)->parent = node;

	tmp_node->parent = node->parent->parent;

	if (P_LIKELY (tmp_node->parent != NULL)) {
		if (tmp_node->parent->base.left == (__PTreeBaseNode *) node->parent)
			tmp_node->parent->base.left = (__PTreeBaseNode *) tmp_node;
		else
			tmp_node->parent->base.right = (__PTreeBaseNode *) tmp_node;
	} else
		*root = (__PTreeBaseNode *) tmp_node;

	node->parent->base.right = tmp_node->base.left;

	if (node->parent->base.right != NULL)
		((__PTreeAVLNode *) node->parent->base.right)->parent = node->parent;

	tmp_node->base.left = (__PTreeBaseNode *) node->parent;
	((__PTreeAVLNode *) tmp_node->base.left)->parent = tmp_node;

	tmp_node->base.right = (__PTreeBaseNode *) node;
	node->parent = tmp_node;

	/* Restore balance factor */
	if (tmp_node->balance_factor == 1) {
		((__PTreeAVLNode *) tmp_node->base.left)->balance_factor  = 0;
		((__PTreeAVLNode *) tmp_node->base.right)->balance_factor = -1;
	} else if (tmp_node->balance_factor == -1) {
		((__PTreeAVLNode *) tmp_node->base.left)->balance_factor  = 1;
		((__PTreeAVLNode *) tmp_node->base.right)->balance_factor = 0;
	} else {
		((__PTreeAVLNode *) tmp_node->base.left)->balance_factor  = 0;
		((__PTreeAVLNode *) tmp_node->base.right)->balance_factor = 0;
	}

	tmp_node->balance_factor = 0;
}

static void
__p_tree_avl_balance_insert (__PTreeAVLNode *node, __PTreeBaseNode **root)
{
	__PTreeAVLNode *parent;

	while (TRUE) {
		parent = node->parent;

		if (P_UNLIKELY (parent == NULL))
			break;

		if (parent->base.left == (__PTreeBaseNode *) node) {
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
					__p_tree_avl_rotate_left_right (node, root);
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
					__p_tree_avl_rotate_right (node, root);

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
					__p_tree_avl_rotate_right_left (node, root);
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
					__p_tree_avl_rotate_left (node, root);

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

pboolean
__p_tree_avl_insert (__PTreeBaseNode	**root_node,
		     PCompareDataFunc	compare_func,
		     ppointer		data,
		     PDestroyFunc	key_destroy_func,
		     PDestroyFunc	value_destroy_func,
		     ppointer		key,
		     ppointer		value)
{
	__PTreeBaseNode	**cur_node;
	__PTreeBaseNode	*parent_node;
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

	if (P_UNLIKELY ((*cur_node = p_malloc0 (sizeof (__PTreeAVLNode))) == NULL))
		return FALSE;

	(*cur_node)->key   = key;
	(*cur_node)->value = value;

	((__PTreeAVLNode *) *cur_node)->balance_factor = 0;
	((__PTreeAVLNode *) *cur_node)->parent         = (__PTreeAVLNode *) parent_node;

	/* Balance the tree */
	__p_tree_avl_balance_insert (((__PTreeAVLNode *) *cur_node), root_node);

	return TRUE;
}

static void
__p_tree_avl_balance_remove (__PTreeAVLNode *node, __PTreeBaseNode **root)
{
	__PTreeAVLNode	*parent;
	__PTreeAVLNode	*sibling;
	pint		sibling_balance;

	while (TRUE) {
		parent = node->parent;

		if (P_UNLIKELY (parent == NULL))
			break;

		if (parent->base.left == (__PTreeBaseNode *) node) {
			if (parent->balance_factor == -1) {
				sibling         = (__PTreeAVLNode *) parent->base.right;
				sibling_balance = sibling->balance_factor;

				if (sibling->balance_factor == 1)
					/* Case 1 */
					__p_tree_avl_rotate_right_left (sibling, root);
				 else
					/* Case 2 */
					__p_tree_avl_rotate_left (sibling, root);

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
				sibling         = (__PTreeAVLNode *) parent->base.left;
				sibling_balance = sibling->balance_factor;

				if (sibling->balance_factor == -1)
					/* Case 1 */
					__p_tree_avl_rotate_left_right (sibling, root);
				else
					/* Case 2 */
					__p_tree_avl_rotate_right (sibling, root);

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
__p_tree_avl_remove (__PTreeBaseNode	**root_node,
		     PCompareDataFunc	compare_func,
		     ppointer		data,
		     PDestroyFunc	key_destroy_func,
		     PDestroyFunc	value_destroy_func,
		     pconstpointer	key)
{
	__PTreeBaseNode	*cur_node;
	__PTreeBaseNode	*prev_node;
	__PTreeBaseNode	*child_node;
	__PTreeAVLNode	*child_parent;
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
		__p_tree_avl_balance_remove ((__PTreeAVLNode *) cur_node, root_node);

	/* Replace node with its child */
	if (P_UNLIKELY (cur_node == *root_node)) {
		*root_node   = child_node;
		child_parent = NULL;
	} else {
		child_parent = ((__PTreeAVLNode *) cur_node)->parent;

		if (child_parent->base.left == cur_node)
			child_parent->base.left = child_node;
		else
			child_parent->base.right = child_node;
	}

	if (child_node != NULL) {
		((__PTreeAVLNode *) child_node)->parent = child_parent;

		/* Balance the tree */
		__p_tree_avl_balance_remove ((__PTreeAVLNode *) child_node, root_node);
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
__p_tree_avl_node_free (__PTreeBaseNode *node)
{
	p_free (node);
}
