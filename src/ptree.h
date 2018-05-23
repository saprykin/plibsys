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

/**
 * @file ptree.h
 * @brief Binary tree data structure
 * @author Alexander Saprykin
 *
 * #PTree represents a binary search tree structure for faster lookup than
 * a plain array or a list. It has average O(logN) time complexity to search
 * a key-value pair, and O(N) in the worst case (when a tree is degenerated into
 * the list).
 *
 * Currently #PTree supports the following tree types:
 * - unbalanced binary search tree;
 * - red-black self-balancing tree;
 * - AVL self-balancing tree.
 *
 * Use p_tree_new(), or its detailed variations like p_tree_new_with_data() and
 * p_tree_new_full() to create a tree structure. Take attention that a caller
 * owns the key and the value data passed when inserting new nodes, so you
 * should manually free the memory after the tree usage. Or you can provide
 * destroy notification functions for the keys and the values separately.
 *
 * New key-value pairs can be inserted with p_tree_insert() and removed with
 * p_tree_remove().
 *
 * Use p_tree_lookup() to find the value by a given key. You can also traverse
 * the tree in-order with p_tree_foreach().
 *
 * Release memory with p_tree_free() or clear a tree with p_tree_clear(). Keys
 * and values would be destroyed only if the corresponding notification
 * functions were provided.
 *
 * Note: all operations with the tree are non-recursive, only iterative calls
 * are used.
 */

#if !defined (PLIBSYS_H_INSIDE) && !defined (PLIBSYS_COMPILATION)
#  error "Header files shouldn't be included directly, consider using <plibsys.h> instead."
#endif

#ifndef PLIBSYS_HEADER_PTREE_H
#define PLIBSYS_HEADER_PTREE_H

#include <pmacros.h>
#include <ptypes.h>

P_BEGIN_DECLS

/** Tree opaque data structure. */
typedef struct PTree_ PTree;

/** Internal data organization algorithm for #PTree. */
typedef enum PTreeType_ {
	P_TREE_TYPE_BINARY	= 0,	/**< Unbalanced binary tree.		*/
	P_TREE_TYPE_RB		= 1,	/**< Red-black self-balancing tree.	*/
	P_TREE_TYPE_AVL		= 2	/**< AVL self-balancing tree.		*/
} PTreeType;

/**
 * @brief Initializes new #PTree.
 * @param type Tree algorithm type to use, can't be changed later.
 * @param func Key compare function.
 * @return Newly initialized #PTree object in case of success, NULL otherwise.
 * @since 0.0.1
 *
 * The caller takes ownership of all the keys and the values passed to the tree.
 */
P_LIB_API PTree *	p_tree_new		(PTreeType		type,
						 PCompareFunc		func);

/**
 * @brief Initializes new #PTree with additional data.
 * @param type Tree algorithm type to use, can't be changed later.
 * @param func Key compare function.
 * @param data Data to be passed to @a func along with the keys.
 * @return Newly initialized #PTree object in case of success, NULL otherwise.
 * @since 0.0.1
 *
 * The caller takes ownership of all the keys and the values passed to the tree.
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
 * @return Newly initialized #PTree object in case of success, NULL otherwise.
 * @since 0.0.1
 *
 * Upon every node destruction the corresponding key and value functions would
 * be called.
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
 * @param value Value corresponding to the given @a key.
 * @since 0.0.1
 *
 * If the @a key already exists in the tree then it will be replaced with the
 * new one. If a key destroy function was provided it would be called on the old
 * key. If a value destroy function was provided it would be called on the old
 * value.
 */
P_LIB_API void		p_tree_insert		(PTree			*tree,
						 ppointer		key,
						 ppointer		value);

/**
 * @brief Removes a key from a tree.
 * @param tree #PTree to remove a key from.
 * @param key A key to lookup.
 * @return TRUE if the key was removed, FALSE if the key was not found.
 * @since 0.0.1
 *
 * If a key destroy function was provided it would be called on the key. If a
 * value destroy function was provided it would be called on the old value.
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
 * @note Morris (non-recursive, non-stack) traversing algorithm is being used.
 *
 * The tree should not be modified while traversing. The internal tree structure
 * can be modified along the traversing process, so keep it in mind for
 * concurrent access.
 */
P_LIB_API void		p_tree_foreach		(PTree			*tree,
						 PTraverseFunc		traverse_func,
						 ppointer		user_data);

/**
 * @brief Clears a tree.
 * @param tree #PTree to clear.
 * @since 0.0.1
 * @note Modified Morris (non-recursive, non-stack) traversing algorithm is
 * being used.
 *
 * All the keys will be deleted. Key and value destroy functions would be called
 * on every node if any of them was provided.
 */
P_LIB_API void		p_tree_clear		(PTree			*tree);

/**
 * @brief Gets a tree algorithm type.
 * @param tree #PTree object to get the type for.
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
 * If the tree is empty or an invalid pointer is given it returns 0.
 */
P_LIB_API pint		p_tree_get_nnodes	(const PTree		*tree);

/**
 * @brief Frees a previously initialized tree object.
 * @param tree #PTree object to free.
 * @since 0.0.1
 * @note Modified Morris (non-recursive, non-stack) traversing algorithm is
 * being used.
 *
 * All the keys will be deleted. Key and value destroy functions would be called
 * on every node if any of them was provided.
 */
P_LIB_API void		p_tree_free		(PTree			*tree);

P_END_DECLS

#endif /* PLIBSYS_HEADER_PTREE_H */
