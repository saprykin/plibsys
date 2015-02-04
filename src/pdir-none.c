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

#include "pdir.h"

#include <stdlib.h>

struct _PDir {
	pint	hdl;
};

P_LIB_API PDir *
p_dir_new (const pchar	*path)
{
	P_UNUSED (path);
	return NULL;
}

P_LIB_API pboolean
p_dir_create (const pchar	*path,
	      pint		mode)
{
	P_UNUSED (path);
	P_UNUSED (mode);
	return FALSE;
}

P_LIB_API pboolean
p_dir_remove (const pchar *path)
{
	P_UNUSED (path);
	return FALSE;
}

P_LIB_API pboolean
p_dir_is_exists (const pchar *path)
{
	P_UNUSED (path);
	return FALSE;
}

P_LIB_API pchar *
p_dir_get_path (const PDir *dir)
{
	P_UNUSED (dir);
	return NULL;
}

P_LIB_API PDirEntry *
p_dir_get_next_entry (PDir *dir)
{
	P_UNUSED (dir);
	return NULL;
}

P_LIB_API void
p_dir_rewind (PDir *dir)
{
	P_UNUSED (dir);
}

P_LIB_API void
p_dir_free (PDir *dir)
{
	P_UNUSED (dir);
}
