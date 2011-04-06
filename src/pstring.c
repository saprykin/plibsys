/* 
 * 14.01.2011
 * Copyright (C) 2011 Alexander Saprykin <xelfium@gmail.com>
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

#include <string.h>

#include "pstring.h"
#include "pmem.h"

P_LIB_API pchar *
p_strdup (const pchar *str)
{
	pchar	*ret;
	psize	len;

	if (str == NULL)
		return NULL;

	len = strlen (str) + 1;
	if ((ret = p_malloc (len)) == NULL)
		return NULL;

	memcpy (ret, str, len);

	return ret;
}

