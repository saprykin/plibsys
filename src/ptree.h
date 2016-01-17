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

/**
 * @file ptree.h
 * @brief Binary tree data structure
 * @author Alexander Saprykin
 *
 * #PTree represents a binary search tree structure for faster lookup than
 * a plain array or a list. It has average O(logN) time complexity to search
 * a key-value pair, and O(N) in worst case (when a tree is degenerated into
 * the list).
 *
 * Currently #PTree supports the following tree types:
 * - unbalanced binary search tree;
 * - red-black self-balancing tree.
 *
 * Use p_tree_new(), or its detailed variations like p_tree_new_with_data() and
 * p_tree_new_full() to create a tree structure. Take attention that a caller
 * owns the key and the value data passed when inserting new nodes, so you
 * should manually free the memory after the tree usage. Or you can provide
 * destroy notification functions for keys and values separately.
 *
 * New key-value pairs can be inserted with p_tree_insert() and removed with
 * p_tree_remove().
 *
 * Use p_tree_lookup() to find the value by a given key. You can also traverse
 * a tree in-order with p_tree_foreach().
 *
 * Release memory with p_tree_free() or clear a tree with p_tree_clear().
 * Keys and values would be destroyed only if corresponding notification
 * functions were provided.
 */

#if !defined (__PLIB_H_INSIDE__) && !defined (PLIB_COMPILATION)
#  error "Header files shouldn't be included directly, consider using <plib.h> instead."
#endif

#ifndef __PTREE_H__
#define __PTREE_H__

#include <pmacros.h>
#include <ptypes.h>

P_BEGIN_DECLS

/** Opaque data structure for handling a tree. */
typedef struct _PTree PTree;

/** Internal data organization algorithm for #PTree. */
typedef enum {
	P_TREE_TYPE_BINARY	= 0,	/**< Unbalanced binary tree.		*/
	P_TREE_TYPE_RB		= 1	/**< Red-black self-balancing tree.	*/
} PTreeType;

/**
 * @brief Initializes new #PTree.
 * @param type Tree algorithm type to use, can't be changed later.
 * @param func Key compare function.
 * @return Newly initialized #PTree object in case of success,
 * NULL otherwise.
 * @since 0.0.1
 *
 * Caller takes ownership of all keys and values passed to the tree.
 */
P_LIB_API PTree *	p_tree_new		(PTreeType		type,
						 PCompareFunc		func);

/**
 * @brief Initializes new #PTree with additional data.
 * @param type Tree algorithm type to use, can't be changed later.
 * @param func Key compare function.
 * @param data Data to be passed to @a func along with the keys.
 * @return Newly initialized #PTree object in case of success,
 * NULL otherwise.
 * @since 0.0.1
 *
 * Caller takes ownership of all the keys and values passed to the tree.
 */
P_LIB_API PTree *	p_tree_new_with_data	(PTreeType		type,
						 PCompareDataFunc	func,
						 ppointer		data);

/**
 * @brief Initializes new #PTree with additional data and memory management.
 * @param type Tree algorithm type to use, can't be changed later.
 * @param func Key compare function.
 * @param data Data to be passed to @a func along with the keys.
 * @param key_destroy Function to call on every key before the node destruction,
 * maybe NULL.
 * @param value_destroy Function to call on every value before the node
 * destruction, maybe NULL.
 * @return Newly initialized #PTree object in case of success,
 * NULL otherwise.
 * @since 0.0.1
 *
 * Upon every node destruction the corresponding key and value functions would be
 * called.
 */
P_LIB_API PTree *	p_tree_new_full		(PTreeType		type,
						 PCompareDataFunc	func,
						 ppointer		data,
						 PDestroyFunc		key_destroy,
						 PDestroyFunc		value_destroy);

/**
 * @brief Inserts a new key-value pair into a tree.
 * @param tree #PTree to insert a node in.
 * @param key Key to insert.
 * @param value Value corresponding to a given @a key.
 * @since 0.0.1
 *
 * If the @a key is already exists in the tree then it will be replaced
 * with a new one. If a key destroy function was provided it would be called
 * on an old key. If a value destroy function was provided it would be called
 * on an old value.
 */
P_LIB_API void		p_tree_insert		(PTree			*tree,
						 ppointer		key,
						 ppointer		value);

/**
 * @brief Removes key from the tree.
 * @param tree #PTree to remove key from.
 * @param key A key to lookup.
 * @return TRUE if key was removed, FALSE if key was not found.
 * @since 0.0.1
 *
 * If a key destroy function was provided it would be called on the key.
 * If a value destroy function was provided it would be called
 * on an old value.
 */
P_LIB_API pboolean	p_tree_remove		(PTree			*tree,
						 pconstpointer		key);

/**
 * @brief Lookups a value by a given key.
 * @param tree #PTree to lookup in.
 * @param key Key to lookup.
 * @return Value for the given @a key in case of success, NULL otherwise.
 * @since 0.0.1
 */
P_LIB_API ppointer	p_tree_lookup		(PTree			*tree,
						 pconstpointer		key);

/**
 * @brief Iterates in-order through the tree nodes.
 * @param tree A tree to traverse.
 * @param traverse_func Function for traversing.
 * @param user_data Additional (maybe NULL) user-provided data for the
 * @a traverse_func.
 * @since 0.0.1
 * @note Morris (non-recursive, non-stack) traversing algorithm is used.
 *
 * The @a tree should not be modified while traversing. Internal tree structure
 * can be modified along the traversing process, so keep it in mind for
 * concurrent access.
 */
P_LIB_API void		p_tree_foreach		(PTree			*tree,
						 PTraverseFunc		traverse_func,
						 ppointer		user_data);

/**
 * @brief Clear the tree.
 * @param tree #PTree to clear.
 * @since 0.0.1
 * @note Modified Morris (non-recursive, non-stack) traversing algorithm is used.
 *
 * All keys will be deleted. Key and value destroy functions would be called
 * on every node if any of them was provided.
 */
P_LIB_API void		p_tree_clear		(PTree			*tree);

/**
 * @brief Gets tree algorithm type.
 * @param tree #PTree object to get type for.
 * @return Tree internal organization algorithm used for a given object.
 * @since 0.0.1
 */
P_LIB_API PTreeType	p_tree_get_type		(const PTree		*tree);

/**
 * @brief Gets node count.
 * @param tree #PTree to get node count for.
 * @return Node count.
 * @since 0.0.1
 *
 * If a tree is empty or an invalid pointer is given it returns 0.
 */
P_LIB_API pint		p_tree_get_nnodes	(const PTree		*tree);

/**
 * @brief Frees previously initialized tree object.
 * @param tree #PTree object to free.
 * @since 0.0.1
 * @note Modified Morris (non-recursive, non-stack) traversing algorithm is used.
 *
 * All keys will be deleted. Key and value destroy functions would be called
 * on every node if any of them was provided.
 */
P_LIB_API void		p_tree_free		(PTree			*tree);

P_END_DECLS

#endif /* __PTREE_H__ */
