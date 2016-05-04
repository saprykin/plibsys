/*
 * Copyright (C) 2015-2016 Alexander Saprykin <xelfium@gmail.com>
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

#include "pdir.h"

#include <stdlib.h>

struct _PDir {
	pint	hdl;
};

P_LIB_API PDir *
p_dir_new (const pchar	*path,
	   PError	**error)
{
	P_UNUSED (path);

	p_error_set_error_p (error,
			     (pint) P_ERROR_IO_NOT_IMPLEMENTED,
			     0,
			     "No directory implementation");

	return NULL;
}

P_LIB_API pboolean
p_dir_create (const pchar	*path,
	      pint		mode,
	      PError		**error)
{
	P_UNUSED (path);
	P_UNUSED (mode);

	p_error_set_error_p (error,
			     (pint) P_ERROR_IO_NOT_IMPLEMENTED,
			     0,
			     "No directory implementation");

	return FALSE;
}

P_LIB_API pboolean
p_dir_remove (const pchar	*path,
	      PError		**error)
{
	P_UNUSED (path);

	p_error_set_error_p (error,
			     (pint) P_ERROR_IO_NOT_IMPLEMENTED,
			     0,
			     "No directory implementation");

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
p_dir_get_next_entry (PDir	*dir,
		      PError	**error)
{
	P_UNUSED (dir);

	p_error_set_error_p (error,
			     (pint) P_ERROR_IO_NOT_IMPLEMENTED,
			     0,
			     "No directory implementation");

	return NULL;
}

P_LIB_API pboolean
p_dir_rewind (PDir	*dir,
	      PError	**error)
{
	P_UNUSED (dir);

	p_error_set_error_p (error,
			     (pint) P_ERROR_IO_NOT_IMPLEMENTED,
			     0,
			     "No directory implementation");

	return FALSE;
}

P_LIB_API void
p_dir_free (PDir *dir)
{
	P_UNUSED (dir);
}
