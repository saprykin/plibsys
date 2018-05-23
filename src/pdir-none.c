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

#include "pdir.h"

#include <stdlib.h>

struct PDir_ {
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
