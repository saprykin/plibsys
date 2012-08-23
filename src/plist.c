/* 
 * 05.08.2010
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

#include "pmem.h"
#include "plist.h"

#include <stdlib.h>

P_LIB_API PList *
p_list_append (PList *list, ppointer data)
{
	PList *item, *cur;

	if ((item = p_malloc0 (sizeof (PList))) == NULL) {
		P_ERROR ("PList: failed to allocate memory");
		return list;
	}

	item->data = data;

	/* List is empty */
	if (list == NULL)
		return item;

	for (cur = list; cur->next != NULL; cur = cur->next)
		;
	cur->next = item;

	return list;
}

P_LIB_API PList *
p_list_remove (PList *list, ppointer data)
{
	PList *cur, *prev, *head;

	if (list == NULL)
		return NULL;

	for (head = list, prev = NULL, cur = list; cur != NULL;  prev = cur, cur = cur->next) {
		if (cur->data == data) {
			if (prev == NULL)
				head = cur->next;
			else
				prev->next = cur->next;

			p_free (cur);

			break;
		}
	}

	return head;
}

P_LIB_API void
p_list_foreach (PList *list, PFunc func, ppointer user_data)
{
	PList *cur;

	if (list == NULL || func == NULL)
		return;

	for (cur = list; cur != NULL; cur = cur->next)
		func (cur->data, user_data);
}

P_LIB_API void
p_list_free (PList *list)
{
	PList *cur, *next;

	if (list == NULL)
		return;

	for (next = cur = list; cur != NULL && next != NULL; cur = next)  {
		next = cur->next;
		p_free (cur);
	}
}

P_LIB_API PList *
p_list_last (PList *list)
{
	PList *cur;

	if (!list)
		return NULL;

	for (cur = list; cur->next != NULL; cur = cur->next)
		;

	return cur;
}

P_LIB_API psize
p_list_length (PList *list)
{
	PList	*cur;
	psize	ret;

	if (!list)
		return 0;

	for (cur = list, ret = 1; cur->next != NULL; cur = cur->next, ++ret)
		;

	return ret;
}

P_LIB_API PList *
p_list_prepend	(PList *list, ppointer data)
{
	PList *item, *cur;

	if ((item = p_malloc0 (sizeof (PList))) == NULL) {
		P_ERROR ("PList: failed to allocate memory");
		return list;
	}

	item->data = data;

	/* List is empty */
	if (list == NULL)
		return item;
	
	item->next = list;	

	return item;
}