/*
 * Copyright (C) 2015-2016 Alexander Saprykin <xelfium@gmail.com>
 * Illustrations have been taken from the Linux kernel rbtree.c
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
#include "ptree-rb.h"

typedef enum _PTreeRBColor {
	P_TREE_RB_COLOR_RED	= 0x01,
	P_TREE_RB_COLOR_BLACK	= 0x02
} __PTreeRBColor;

typedef struct _PTreeRBNode {
	struct _PTreeBaseNode	base;
	struct _PTreeRBNode	*parent;
	__PTreeRBColor		color;
} __PTreeRBNode;

static pboolean
__p_tree_rb_is_black (__PTreeRBNode *node)
{
	if (node == NULL)
		return TRUE;

	return ((node->color) & P_TREE_RB_COLOR_BLACK) > 0 ? TRUE : FALSE;
}

static pboolean
__p_tree_rb_is_red (__PTreeRBNode *node)
{
	return ((node->color) & P_TREE_RB_COLOR_RED) > 0 ? TRUE : FALSE;
}

static __PTreeRBNode *
__p_tree_rb_get_gparent (__PTreeRBNode *node)
{
	return node->parent->parent;
}

static __PTreeRBNode *
__p_tree_rb_get_uncle (__PTreeRBNode *node)
{
	__PTreeRBNode *gparent = __p_tree_rb_get_gparent (node);

	if ((__PTreeRBNode *) gparent->base.left == node->parent)
		return (__PTreeRBNode *) gparent->base.right;
	else
		return (__PTreeRBNode *) gparent->base.left;
}

static __PTreeRBNode *
__p_tree_rb_get_sibling (__PTreeRBNode *node)
{
	if (node->parent->base.left == (__PTreeBaseNode *) node)
		return (__PTreeRBNode *) node->parent->base.right;
	else
		return (__PTreeRBNode *) node->parent->base.left;
}

static void
__p_tree_rb_rotate_left (__PTreeRBNode *node, __PTreeBaseNode **root)
{
	__PTreeBaseNode *tmp_node;

	tmp_node = node->base.right;

	if (P_LIKELY (node->parent != NULL)) {
		if (node->parent->base.left == (__PTreeBaseNode *) node)
			node->parent->base.left = tmp_node;
		else
			node->parent->base.right = tmp_node;
	}

	node->base.right = tmp_node->left;

	if (tmp_node->left != NULL)
		((__PTreeRBNode *) tmp_node->left)->parent = node;

	tmp_node->left = (__PTreeBaseNode *) node;
	((__PTreeRBNode *) tmp_node)->parent = node->parent;
	node->parent = (__PTreeRBNode *) tmp_node;

	if (P_UNLIKELY (((__PTreeRBNode *) tmp_node)->parent == NULL))
		*root = tmp_node;
}

static void
__p_tree_rb_rotate_right (__PTreeRBNode *node, __PTreeBaseNode **root)
{
	__PTreeBaseNode *tmp_node;

	tmp_node = node->base.left;

	if (P_LIKELY (node->parent != NULL)) {
		if (node->parent->base.left == (__PTreeBaseNode *) node)
			node->parent->base.left = tmp_node;
		else
			node->parent->base.right = tmp_node;
	}

	node->base.left = tmp_node->right;

	if (tmp_node->right != NULL)
		((__PTreeRBNode *) tmp_node->right)->parent = node;

	tmp_node->right = (__PTreeBaseNode *) node;
	((__PTreeRBNode *) tmp_node)->parent = node->parent;
	node->parent = (__PTreeRBNode *) tmp_node;

	if (P_UNLIKELY (((__PTreeRBNode *) tmp_node)->parent == NULL))
		*root = tmp_node;
}

static void
__p_tree_rb_balance_insert (__PTreeRBNode *node, __PTreeBaseNode **root)
{
	__PTreeRBNode *uncle;
	__PTreeRBNode *gparent;

	while (TRUE) {
		/* Case 1: We are at the root  */
		if (P_UNLIKELY (node->parent == NULL)) {
			node->color = P_TREE_RB_COLOR_BLACK;
			break;
		}

		/* Case 2: We have a black parent */
		if (__p_tree_rb_is_black (node->parent) == TRUE)
			break;

		uncle   = __p_tree_rb_get_uncle (node);
		gparent = __p_tree_rb_get_gparent (node);

		/* Case 3: Both parent and uncle are red, flip colors
		 *
		 *       G            g
		 *      / \          / \
		 *     p   u  -->   P   U
		 *    /            /
		 *   n            n
		 */
		if (uncle != NULL && __p_tree_rb_is_red (uncle) == TRUE) {
			node->parent->color = P_TREE_RB_COLOR_BLACK;
			uncle->color        = P_TREE_RB_COLOR_BLACK;
			gparent->color      = P_TREE_RB_COLOR_RED;

			/* Continue iteratively from gparent */
			node = gparent;
			continue;
		}

		if (node->parent == (__PTreeRBNode *) gparent->base.left) {
			if (node == (__PTreeRBNode *) node->parent->base.right) {
				/* Case 4a: Left rotate at parent
				 *
				 *      G             G
				 *     / \           / \
				 *    p   U  -->    n   U
				 *     \           /
				 *      n         p
				 */
				__p_tree_rb_rotate_left (node->parent, root);

				node = (__PTreeRBNode *) node->base.left;
			}

			gparent->color      = P_TREE_RB_COLOR_RED;
			node->parent->color = P_TREE_RB_COLOR_BLACK;

			/* Case 5a: Right rotate at gparent
			 *
			 *        G           P
			 *       / \         / \
			 *      p   U  -->  n   g
			 *     /                 \
			 *    n                   U
			 */
			__p_tree_rb_rotate_right (gparent, root);

			break;
		} else {
			if (node == (__PTreeRBNode *) node->parent->base.left) {
				/* Case 4b: Right rotate at parent */
				__p_tree_rb_rotate_right (node->parent, root);

				node = (__PTreeRBNode *) node->base.right;
			}

			gparent->color      = P_TREE_RB_COLOR_RED;
			node->parent->color = P_TREE_RB_COLOR_BLACK;

			/* Case 5b: Left rotate at gparent*/
			__p_tree_rb_rotate_left (gparent, root);

			break;
		}
	}
}

pboolean
__p_tree_rb_insert (__PTreeBaseNode	**root_node,
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

	if (P_UNLIKELY ((*cur_node = p_malloc0 (sizeof (__PTreeRBNode))) == NULL))
		return FALSE;

	(*cur_node)->key   = key;
	(*cur_node)->value = value;

	((__PTreeRBNode *) *cur_node)->color  = P_TREE_RB_COLOR_RED;
	((__PTreeRBNode *) *cur_node)->parent = (__PTreeRBNode *) parent_node;

	/* Balance the tree */
	__p_tree_rb_balance_insert ((__PTreeRBNode *) *cur_node, root_node);

	return TRUE;
}

static void
__p_tree_rb_balance_remove (__PTreeRBNode *node, __PTreeBaseNode **root)
{
	__PTreeRBNode *sibling;

	while (TRUE) {
		/* Case 1: We are at the root */
		if (P_UNLIKELY (node->parent == NULL))
			break;

		sibling = __p_tree_rb_get_sibling (node);

		if (__p_tree_rb_is_red (sibling) == TRUE) {
			/*
			 * Case 2: Left (right) rotate at parent
			 *
			 *     P               S
			 *    / \             / \
			 *   N   s    -->    p   Sr
			 *      / \         / \
			 *     Sl  Sr      N   Sl
			 */
			node->parent->color = P_TREE_RB_COLOR_RED;
			sibling->color      = P_TREE_RB_COLOR_BLACK;

			if ((__PTreeBaseNode *) node == node->parent->base.left)
				__p_tree_rb_rotate_left (node->parent, root);
			else
				__p_tree_rb_rotate_right (node->parent, root);

			sibling = __p_tree_rb_get_sibling (node);
		}

		/*
		 * Case 3: Sibling (parent) color flip
		 *
		 *    (p)           (p)
		 *    / \           / \
		 *   N   S    -->  N   s
		 *      / \           / \
		 *     Sl  Sr        Sl  Sr
		 */
		if (__p_tree_rb_is_black ((__PTreeRBNode *) sibling->base.left)  == TRUE &&
		    __p_tree_rb_is_black ((__PTreeRBNode *) sibling->base.right) == TRUE) {
			sibling->color = P_TREE_RB_COLOR_RED;

			if (__p_tree_rb_is_black (node->parent) == TRUE) {
				node = node->parent;
				continue;
			} else {
				node->parent->color = P_TREE_RB_COLOR_BLACK;
				break;
			}
		}

		/*
		 * Case 4: Right (left) rotate at sibling
		 *
		 *   (p)           (p)
		 *   / \           / \
		 *  N   S    -->  N   Sl
		 *     / \             \
		 *    sl  Sr            s
		 *                       \
		 *                        Sr
		 */
		if ((__PTreeBaseNode *) node == node->parent->base.left &&
		    __p_tree_rb_is_black ((__PTreeRBNode *) sibling->base.right) == TRUE) {
			sibling->color = P_TREE_RB_COLOR_RED;
			((__PTreeRBNode *) sibling->base.left)->color = P_TREE_RB_COLOR_BLACK;

			__p_tree_rb_rotate_right (sibling, root);

			sibling = __p_tree_rb_get_sibling (node);
		} else if ((__PTreeBaseNode *) node == node->parent->base.right &&
			   __p_tree_rb_is_black ((__PTreeRBNode *) sibling->base.left) == TRUE) {
			sibling->color = P_TREE_RB_COLOR_RED;
			((__PTreeRBNode *) sibling->base.right)->color = P_TREE_RB_COLOR_BLACK;

			__p_tree_rb_rotate_left (sibling, root);

			sibling = __p_tree_rb_get_sibling (node);
		}

		/*
		 * Case 5: Left (right) rotate at parent and color flips
		 *
		 *      (p)             (s)
		 *      / \             / \
		 *     N   S     -->   P   Sr
		 *        / \         / \
		 *      (sl) sr      N  (sl)
		 */
		sibling->color      = node->parent->color;
		node->parent->color = P_TREE_RB_COLOR_BLACK;

		if ((__PTreeBaseNode *) node == node->parent->base.left) {
			((__PTreeRBNode *) sibling->base.right)->color = P_TREE_RB_COLOR_BLACK;
			__p_tree_rb_rotate_left (node->parent, root);
		} else {
			((__PTreeRBNode *) sibling->base.left)->color = P_TREE_RB_COLOR_BLACK;
			__p_tree_rb_rotate_right (node->parent, root);
		}

		break;
	}
}

pboolean
__p_tree_rb_remove (__PTreeBaseNode	**root_node,
		    PCompareDataFunc	compare_func,
		    ppointer		data,
		    PDestroyFunc	key_destroy_func,
		    PDestroyFunc	value_destroy_func,
		    pconstpointer	key)
{
	__PTreeBaseNode	*cur_node;
	__PTreeBaseNode	*prev_node;
	__PTreeBaseNode	*child_node;
	__PTreeRBNode	*child_parent;
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

	if (child_node == NULL && __p_tree_rb_is_black ((__PTreeRBNode *) cur_node) == TRUE)
		__p_tree_rb_balance_remove ((__PTreeRBNode *) cur_node, root_node);

	/* Replace node with its child */
	if (cur_node == *root_node) {
		*root_node   = child_node;
		child_parent = NULL;
	} else {
		child_parent = ((__PTreeRBNode *) cur_node)->parent;

		if (child_parent->base.left == cur_node)
			child_parent->base.left = child_node;
		else
			child_parent->base.right = child_node;
	}

	if (child_node != NULL) {
		((__PTreeRBNode *) child_node)->parent = child_parent;

		/* Check if we need to repaint the node */
		if (__p_tree_rb_is_black ((__PTreeRBNode *) cur_node) == TRUE)
				((__PTreeRBNode *) child_node)->color = P_TREE_RB_COLOR_BLACK;
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
__p_tree_rb_node_free (__PTreeBaseNode *node)
{
	p_free (node);
}
