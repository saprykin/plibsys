/* 
 * Copyright (C) 2010 Alexander Saprykin <xelfium@gmail.com>
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
 * @file plist.h
 * @brief Single-linked list
 * @author Alexander Saprykin
 * @todo Prepend and reverse operations
 *
 * #PList is used to store pointers in single-linked list. Before usage you
 * must initialize it to NULL. p_list_append() and p_list_remove() routines
 * return pointers to updated #PList in case of success, so you need to replace
 * previously used pointer with a new one:
 * @code
 * PList       *list;
 * ppointer    data;
 *
 * list = NULL;
 * data = my_obj_new ();
 *
 * list = p_list_append (list, data);
 * @endcode
 * #PList store only the pointers to
 * data, and you must free used memory manually, p_list_free() only frees list's
 * internal memory, not the data it stores pointers for:
 * @code
 * PList    *list;
 * ...
 * p_list_foreach (list, (PFunc) my_free_func, my_data);
 * p_list_free (list);
 * @endcode
 * Also you can use P_INT_TO_POINTER and P_POINTER_TO_INT macros to store
 * integers (up to 32-bit) without allocating memory for them:
 * @code
 * PList    *list;
 * pint     a;
 * 
 * list = p_list_append (list, P_INT_TO_POINTER (12));
 * a = P_POINTER_TO_INT (list->data);
 * @endcode
 * #PList can store several elements with the same pointer value, but
 * p_list_remove() will remove only the first matching node.
 */

#if !defined (__PLIB_H_INSIDE__) && !defined (PLIB_COMPILATION)
#  error "Header files shouldn't be included directly, consider using <plib.h> instead."
#endif

#ifndef __PLIBLIST_H__
#define __PLIBLIST_H__

#include <pmacros.h>

P_BEGIN_DECLS

/** General purpose function */
typedef void (*PFunc) (ppointer data, ppointer user_data);

/** Typedef for list node */
typedef struct _PList PList;

/**
 * @struct _PList
 * @brief Node for single-linked list.
 */
struct _PList {
	ppointer	data;	/**< Pointer to node data.	*/
	PList		*next;	/**< Next list node.		*/
};

/**
 * @brief Appends data to the list.
 * @param list #PList for appending data.
 * @param data Data to append.
 * @return Pointer to updated list in case of success, @a list otherwise.
 * @since 0.0.1
 *
 * Before appending first element into the list, @a list argument must be
 * initialized with NULL. Otherwise behaviour is unpredictable.
 */
P_LIB_API PList *	p_list_append	(PList *list, ppointer data) P_GNUC_WARN_UNUSED_RESULT;

/**
 * @brief Removes data from the list.
 * @param list List to remove data from.
 * @param data Data to remove.
 * @return Pointer to updated list in case of success, @a list otherwise.
 * @since 0.0.1
 *
 * It searches for the first matching occurance in the @a list and removes
 * that node. Note that it removes only pointer from the @a list, not the data
 * it pointers to, so you need to free data manually.
 */
P_LIB_API PList *	p_list_remove	(PList *list, ppointer data) P_GNUC_WARN_UNUSED_RESULT;

/**
 * @brief Calls specified function for each list element.
 * @param list List to go through.
 * @param func Pointer for callback function.
 * @param user_data User defined data, may be NULL.
 * @since 0.0.1
 *
 * This function goes through the whole @a list and calls @a func for each node.
 * @a func will receive pointer to node's data and @a user_data. You can use
 * it to free the data:
 * @code
 * p_list_foreach (list, (PFunc) free, NULL);
 * p_list_free (list);
 * @endcode
 */
P_LIB_API void		p_list_foreach	(PList *list, PFunc func, ppointer user_data);

/**
 * @brief Frees list memory.
 * @param list List to free.
 * @since 0.0.1
 *
 * This function frees only list's internal memory, not the pointers stored in nodes.
 * Don't forget to free all the data stored in the list manually.
 */
P_LIB_API void		p_list_free	(PList *list);

/**
 * @brief Gets last element in the list.
 * @param list List to get element from.
 * @return Pointer to the last @a list node, NULL if the @a list is empty.
 * @since 0.0.1
 */
P_LIB_API PList *	p_list_last	(PList *list);

/**
 * @brief Gets the count of list elements.
 * @param list List to count elements in.
 * @return Number of elements in the @a list.
 * @since 0.0.1
 * @note This function will iterate through the whole @a list, so don't use it
 * in condition of the for-loop or in code which repeats a lot of time.
 */
P_LIB_API psize		p_list_length	(PList *list);

/**
 * @brief Prepends data to the list.
 * @param list #PList for prepending data.
 * @param data Data to prepend.
 * @return Pointer to updated list in case of success, @a list otherwise.
 * @since 0.0.1
 *
 * Before prepending first element to the list, @a list argument must be
 * initialized with NULL. Otherwise behaviour is unpredictable.
 */
P_LIB_API PList *	p_list_prepend	(PList *list, ppointer data) P_GNUC_WARN_UNUSED_RESULT;

P_END_DECLS

#endif /* __PLIBLIST_H__ */
