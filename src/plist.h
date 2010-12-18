/* 
 * 03.08.2010
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

#if !defined (__PLIB_H_INSIDE__) && !defined (PLIB_COMPILATION)
#  error "Header files shouldn't be included directly, consider using <plib.h> instead."
#endif

#ifndef __PLIBLIST_H__
#define __PLIBLIST_H__

#include <pmacros.h>

typedef void (*PFunc) (ppointer data, ppointer user_data);

typedef struct _PList PList;
struct _PList {
	ppointer	data;
	PList		*next;
};

P_BEGIN_DECLS

P_LIB_API PList *	p_list_append	(PList *list, ppointer data) P_GNUC_WARN_UNUSED_RESULT;
P_LIB_API PList *	p_list_remove	(PList *list, ppointer data) P_GNUC_WARN_UNUSED_RESULT;
P_LIB_API void		p_list_foreach	(PList *list, PFunc func, ppointer user_data);
P_LIB_API void		p_list_free	(PList *list);
P_LIB_API PList *	p_list_last	(PList *list);
P_LIB_API pint		p_list_length	(PList *list);

P_END_DECLS

#endif /* __PLIBLIST_H__ */

