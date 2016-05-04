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

#include "pmem.h"
#include "pstring.h"
#include "pdir.h"
#include "plibsys-private.h"

#include <stdlib.h>
#include <string.h>

#include <windows.h>

struct _PDir {
	WIN32_FIND_DATAA	find_data;
	HANDLE			search_handle;
	pboolean		cached;
	pchar			path[MAX_PATH + 3];
	pchar			*orig_path;
};

P_LIB_API PDir *
p_dir_new (const pchar	*path,
	   PError	**error)
{
	PDir	*ret;
	pchar	*pathp;

	if (path == NULL) {
		p_error_set_error_p (error,
				     (pint) P_ERROR_IO_INVALID_ARGUMENT,
				     0,
				     "Invalid input argument");
		return NULL;
	}

	if ((ret = p_malloc0 (sizeof (PDir))) == NULL) {
		p_error_set_error_p (error,
				     (pint) P_ERROR_IO_NO_RESOURCES,
				     0,
				     "Failed to allocate memory for directory structure");
		return NULL;
	}

	if (!GetFullPathNameA (path, MAX_PATH, ret->path, NULL)) {
		p_error_set_error_p (error,
				     (pint) __p_error_get_last_io (),
				     __p_error_get_last_error (),
				     "Failed to call GetFullPathNameA() to get directory path");
		p_free (ret);
		return NULL;
	}

	/* Append the search pattern "\\*\0" to the directory name */
	pathp = strchr (ret->path, '\0');

	if (ret->path < pathp  &&  *(pathp - 1) != '\\'  &&  *(pathp - 1) != ':')
		*pathp++ = '\\';

	*pathp++ = '*';
	*pathp = '\0';

	/* Open directory stream and retrieve the first entry */
	ret->search_handle = FindFirstFileA (ret->path, &ret->find_data);

	if (ret->search_handle == INVALID_HANDLE_VALUE) {
		p_error_set_error_p (error,
				     (pint) __p_error_get_last_io (),
				     __p_error_get_last_error (),
				     "Failed to call FindFirstFileA() to open directory stream");
		p_free (ret);
		return NULL;
	}

	ret->cached = TRUE;
	ret->orig_path = p_strdup (path);

	return ret;
}

P_LIB_API pboolean
p_dir_create (const pchar	*path,
	      pint		mode,
	      PError		**error)
{
	P_UNUSED (mode);

	if (path == NULL) {
		p_error_set_error_p (error,
				     (pint) P_ERROR_IO_INVALID_ARGUMENT,
				     0,
				     "Invalid input argument");
		return FALSE;
	}

	if (p_dir_is_exists (path))
		return TRUE;

	if (CreateDirectoryA (path, NULL) == 0) {
		p_error_set_error_p (error,
				     (pint) __p_error_get_last_io (),
				     __p_error_get_last_error (),
				     "Failed to call CreateDirectoryA() to create directory");
		return FALSE;
	} else
		return TRUE;
}

P_LIB_API pboolean
p_dir_remove (const pchar	*path,
	      PError		**error)
{
	if (path == NULL) {
		p_error_set_error_p (error,
				     (pint) P_ERROR_IO_INVALID_ARGUMENT,
				     0,
				     "Invalid input argument");
		return FALSE;
	}

	if (!p_dir_is_exists (path)) {
		p_error_set_error_p (error,
				     (pint) P_ERROR_IO_NOT_EXISTS,
				     0,
				     "Specified directory doesn't exist");
		return FALSE;
	}

	if (RemoveDirectoryA (path) == 0) {
		p_error_set_error_p (error,
				     (pint) __p_error_get_last_io (),
				     __p_error_get_last_error (),
				     "Failed to call RemoveDirectoryA() to remove directory");
		return FALSE;
	} else
		return TRUE;
}

P_LIB_API pboolean
p_dir_is_exists (const pchar *path)
{
	DWORD	dwAttrs;

	if (path == NULL)
		return FALSE;

	dwAttrs = GetFileAttributesA (path);

	return (dwAttrs != INVALID_FILE_ATTRIBUTES) && (dwAttrs & FILE_ATTRIBUTE_DIRECTORY);
}

P_LIB_API pchar *
p_dir_get_path (const PDir *dir)
{
	if (dir == NULL)
		return NULL;

	return p_strdup (dir->orig_path);
}

P_LIB_API PDirEntry *
p_dir_get_next_entry (PDir	*dir,
		      PError	**error)
{
	PDirEntry	*ret;
	DWORD		dwAttrs;

	if (dir == NULL) {
		p_error_set_error_p (error,
				     (pint) P_ERROR_IO_INVALID_ARGUMENT,
				     0,
				     "Invalid input argument");
		return NULL;
	}

	if (dir->cached == TRUE)
		dir->cached = FALSE;
	else {
		if (dir->search_handle == INVALID_HANDLE_VALUE) {
			p_error_set_error_p (error,
					     (pint) P_ERROR_IO_INVALID_ARGUMENT,
					     0,
					     "Not a valid (or closed) directory stream");
			return NULL;
		}

		if (!FindNextFileA (dir->search_handle, &dir->find_data)) {
			p_error_set_error_p (error,
					     (pint) __p_error_get_last_io (),
					     __p_error_get_last_error (),
					     "Failed to call FindNextFileA() to read directory stream");
			FindClose (dir->search_handle);
			dir->search_handle = INVALID_HANDLE_VALUE;
			return NULL;
		}
	}

	if ((ret = p_malloc0 (sizeof (PDirEntry))) == NULL) {
		p_error_set_error_p (error,
				     (pint) P_ERROR_IO_NO_RESOURCES,
				     0,
				     "Failed to allocate memory for directory entry");
		return NULL;
	}

	ret->name = p_strdup (dir->find_data.cFileName);

	dwAttrs = dir->find_data.dwFileAttributes;

	if (dwAttrs & FILE_ATTRIBUTE_DIRECTORY)
		ret->type = P_DIR_ENTRY_TYPE_DIR;
	else if (dwAttrs & FILE_ATTRIBUTE_DEVICE)
		ret->type = P_DIR_ENTRY_TYPE_OTHER;
	else
		ret->type = P_DIR_ENTRY_TYPE_FILE;

	return ret;
}

P_LIB_API pboolean
p_dir_rewind (PDir	*dir,
	      PError	**error)
{
	if (dir == NULL) {
		p_error_set_error_p (error,
				     (pint) P_ERROR_IO_INVALID_ARGUMENT,
				     0,
				     "Invalid input argument");
		return FALSE;
	}

	if (dir->search_handle != INVALID_HANDLE_VALUE) {
		if (FindClose (dir->search_handle) == 0) {
			p_error_set_error_p (error,
					     (pint) __p_error_get_last_io (),
					     __p_error_get_last_error (),
					     "Failed to call FindClose() to close directory stream");
			return FALSE;
		}
	}

	dir->search_handle = FindFirstFileA (dir->path, &dir->find_data);

	if (dir->search_handle == INVALID_HANDLE_VALUE) {
		p_error_set_error_p (error,
				     (pint) __p_error_get_last_io (),
				     __p_error_get_last_error (),
				     "Failed to call FindFirstFileA() to open directory stream");
		dir->cached = FALSE;
		return FALSE;
	} else {
		dir->cached = TRUE;
		return TRUE;
	}
}

P_LIB_API void
p_dir_free (PDir *dir)
{
	if (dir == NULL)
		return;

	if (dir->search_handle != INVALID_HANDLE_VALUE) {
		if (!FindClose (dir->search_handle))
			P_ERROR ("PDir: error while closing the directory");
	}

	p_free (dir->orig_path);
	p_free (dir);
}
