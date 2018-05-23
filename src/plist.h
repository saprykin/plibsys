/*
 * The MIT License
 *
 * Copyright (C) 2010-2016 Alexander Saprykin <saprykin.spb@gmail.com>
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
 * @file plist.h
 * @brief Singly linked list
 * @author Alexander Saprykin
 *
 * A singly linked list is a data structure consists of the nodes which
 * represent a sequence. Each node contains a data pointer and a pointer to the
 * next node. Every node has a link only to the next node, hence list is a
 * singly linked (in a single direction).
 *
 * As the singly linked list is a linear collection of the nodes with the
 * sequential access, it has an O(N) average complexity for appending, removing
 * and searching operations. Prepending a node takes O(1) constant time. Thus it
 * is not intended for heavy usage, please refer to #PHashTable or #PTree if you
 * are working with large data sets.
 *
 * Before the first usage you must initialize a #PList variable to NULL. After
 * that you can use the p_list_append(), p_list_prepend(), p_list_remove() and
 * p_list_reverse() routines to update that variable:
 * @code
 * PList       *list;
 * ppointer    data;
 *
 * list = NULL;
 * data = my_obj_new ();
 *
 * list = p_list_append (list, data);
 * @endcode
 * #PList stores only the pointers to the data, so you must free used memory
 * manually, p_list_free() only frees list's internal memory, not the data it
 * stores the pointers for. The best approach to free used memory is the
 * p_list_foreach() routine:
 * @code
 * PList    *list;
 * ...
 * p_list_foreach (list, (PFunc) my_free_func, my_data);
 * p_list_free (list);
 * @endcode
 * Also you can use #P_INT_TO_POINTER and #P_POINTER_TO_INT macros to store
 * integers (up to 32-bit) without allocating memory for them:
 * @code
 * PList    *list;
 * pint     a;
 *
 * list = p_list_append (list, P_INT_TO_POINTER (12));
 * a = P_POINTER_TO_INT (list->data);
 * @endcode
 * #PList can store several nodes with the same pointer value, but
 * p_list_remove() will remove only the first matching node.
 *
 * If you need to add large amount of nodes at once it is better to prepend them
 * and then reverse the list.
 */

#if !defined (PLIBSYS_H_INSIDE) && !defined (PLIBSYS_COMPILATION)
#  error "Header files shouldn't be included directly, consider using <plibsys.h> instead."
#endif

#ifndef PLIBSYS_HEADER_PLIST_H
#define PLIBSYS_HEADER_PLIST_H

#include <pmacros.h>
#include <ptypes.h>

P_BEGIN_DECLS

/** Typedef for a list node. */
typedef struct PList_ PList;

/** Node for a singly linked list. */
struct PList_ {
	ppointer	data;	/**< Pointer to the node data.	*/
	PList		*next;	/**< Next list node.		*/
};

/**
 * @brief Appends data to a list.
 * @param list #PList for appending the data.
 * @param data Data to append.
 * @return Pointer to the updated list in case of success, @a list otherwise.
 * @since 0.0.1
 *
 * Before appending the first node to the list, @a list argument must be
 * initialized with NULL. Otherwise behavior is unpredictable.
 */
P_LIB_API PList *	p_list_append	(PList		*list,
					 ppointer	data) P_GNUC_WARN_UNUSED_RESULT;

/**
 * @brief Removes data from a list.
 * @param list List to remove the data from.
 * @param data Data to remove.
 * @return Pointer to the updated list in case of success, @a list otherwise.
 * @since 0.0.1
 *
 * It searches for the first matching occurrence in the @a list and removes
 * that node. Note that it removes only the pointer from the @a list, not the
 * data it pointers to, so you need to free the data manually.
 */
P_LIB_API PList *	p_list_remove	(PList		*list,
					 ppointer	data) P_GNUC_WARN_UNUSED_RESULT;

/**
 * @brief Calls a specified function for each list node.
 * @param list List to go through.
 * @param func Pointer for the callback function.
 * @param user_data User defined data, may be NULL.
 * @since 0.0.1
 *
 * This function goes through the whole @a list and calls @a func for each node.
 * The @a func will receive pointer to the node's data and @a user_data. You can
 * use it to free the data:
 * @code
 * p_list_foreach (list, (PFunc) free, NULL);
 * p_list_free (list);
 * @endcode
 */
P_LIB_API void		p_list_foreach	(PList		*list,
					 PFunc		func,
					 ppointer	user_data);

/**
 * @brief Frees list memory.
 * @param list List to free.
 * @since 0.0.1
 *
 * This function frees only the list's internal memory, not the data in the
 * pointers stored in the nodes. Don't forget to free all the data stored in the
 * list manually.
 */
P_LIB_API void		p_list_free	(PList		*list);

/**
 * @brief Gets the last node from the list.
 * @param list List to get the node from.
 * @return Pointer to the last @a list node, NULL if the @a list is empty.
 * @since 0.0.1
 */
P_LIB_API PList *	p_list_last	(PList		*list);

/**
 * @brief Gets the number of list nodes.
 * @param list List to count nodes in.
 * @return Number of nodes in the @a list.
 * @since 0.0.1
 * @note This function will iterate through the whole @a list, so don't use it
 * in condition of the for-loop or in the code which is repeated a lot of times.
 */
P_LIB_API psize		p_list_length	(const PList	*list);

/**
 * @brief Prepends data to a list.
 * @param list #PList for prepending the data.
 * @param data Data to prepend.
 * @return Pointer to the updated list in case of success, @a list otherwise.
 * @since 0.0.1
 *
 * Before prepending the first node to the list, @a list argument must be
 * initialized with NULL. Otherwise behavior is unpredictable.
 */
P_LIB_API PList *	p_list_prepend	(PList		*list,
					 ppointer	data) P_GNUC_WARN_UNUSED_RESULT;

/**
 * @brief Reverses the list order.
 * @param list #PList to reverse the order.
 * @return Pointer to the top of the reversed list.
 * @since 0.0.1
 */
P_LIB_API PList *	p_list_reverse	(PList		*list) P_GNUC_WARN_UNUSED_RESULT;

P_END_DECLS

#endif /* PLIBSYS_HEADER_PLIST_H */
