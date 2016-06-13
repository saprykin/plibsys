/*
 * Copyright (C) 2010-2016 Alexander Saprykin <xelfium@gmail.com>
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, see <http://www.gnu.org/licenses/>.
 */

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
				     p_error_get_last_error (),
				     "Failed to remove file");

	return result;
}
