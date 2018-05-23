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

#include "perror.h"
#include "pfile.h"
#include "perror-private.h"

#ifndef P_OS_WIN
#  include <unistd.h>
#endif

P_LIB_API pboolean
p_file_is_exists (const pchar *file)
{
#ifdef P_OS_WIN
	DWORD attrs;
#endif

	if (P_UNLIKELY (file == NULL))
		return FALSE;

#ifdef P_OS_WIN
	attrs = GetFileAttributesA ((LPCSTR) file);

	return (attrs != INVALID_FILE_ATTRIBUTES && (attrs & FILE_ATTRIBUTE_DIRECTORY) == 0);
#else
	return access (file, F_OK) == 0;
#endif
}

P_LIB_API pboolean
p_file_remove (const pchar	*file,
	       PError		**error)
{
	pboolean result;

	if (P_UNLIKELY (file == NULL)) {
		p_error_set_error_p (error,
				     (pint) P_ERROR_IO_INVALID_ARGUMENT,
				     0,
				     "Invalid input argument");
		return FALSE;
	}

#ifdef P_OS_WIN
	result = (DeleteFileA ((LPCSTR) file) != 0);
#else
	result = (unlink (file) == 0);
#endif

	if (P_UNLIKELY (!result))
		p_error_set_error_p (error,
				     (pint) p_error_get_last_io (),
				     p_error_get_last_system (),
				     "Failed to remove file");

	return result;
}
