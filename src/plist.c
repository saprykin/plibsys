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

#include "pmem.h"
#include "plist.h"

#include <stdlib.h>

P_LIB_API PList *
p_list_append (PList *list, ppointer data)
{
	PList *item, *cur;

	if (P_UNLIKELY ((item = p_malloc0 (sizeof (PList))) == NULL)) {
		P_ERROR ("PList::p_list_append: failed to allocate memory");
		return list;
	}

	item->data = data;

	/* List is empty */
	if (P_UNLIKELY (list == NULL))
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

	if (P_UNLIKELY (list == NULL))
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

	if (P_UNLIKELY (list == NULL || func == NULL))
		return;

	for (cur = list; cur != NULL; cur = cur->next)
		func (cur->data, user_data);
}

P_LIB_API void
p_list_free (PList *list)
{
	PList *cur, *next;

	if (P_UNLIKELY (list == NULL))
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

	if (P_UNLIKELY (list == NULL))
		return NULL;

	for (cur = list; cur->next != NULL; cur = cur->next)
		;

	return cur;
}

P_LIB_API psize
p_list_length (const PList *list)
{
	const PList	*cur;
	psize		ret;

	if (P_UNLIKELY (list == NULL))
		return 0;

	for (cur = list, ret = 1; cur->next != NULL; cur = cur->next, ++ret)
		;

	return ret;
}

P_LIB_API PList *
p_list_prepend	(PList *list, ppointer data)
{
	PList *item;

	if (P_UNLIKELY ((item = p_malloc0 (sizeof (PList))) == NULL)) {
		P_ERROR ("PList::p_list_prepend: failed to allocate memory");
		return list;
	}

	item->data = data;

	/* List is empty */
	if (P_UNLIKELY (list == NULL))
		return item;

	item->next = list;

	return item;
}

P_LIB_API PList *
p_list_reverse	(PList *list)
{
	PList *prev, *cur, *tmp;

	if (P_UNLIKELY (list == NULL))
		return NULL;

	prev	   = list;
	cur	   = list->next;
	prev->next = NULL;

	while (cur != NULL) {
		tmp	  = cur->next;
		cur->next = prev;
		prev	  = cur;
		cur	  = tmp;
	}

	return prev;
}
