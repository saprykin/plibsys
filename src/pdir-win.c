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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA.
 */

#include "pmem.h"
#include "pstring.h"
#include "pdir.h"

#include <winsock2.h>
#include <windows.h>
#include <stdlib.h>
#include <string.h>

struct _PDir {
	WIN32_FIND_DATAA	find_data;
	HANDLE			search_handle;
	pboolean		cached;
	pchar			path[MAX_PATH + 3];
	pchar			*orig_path;
};

P_LIB_API PDir *
p_dir_new (const pchar	*path)
{
	PDir	*ret;
	pchar	*pathp;

	if (path == NULL)
		return NULL;

	if ((ret = p_malloc0 (sizeof (PDir))) == NULL) {
		P_ERROR ("PDir: failed to allocate memory");
		return NULL;
	}

	if (!GetFullPathNameA (path, MAX_PATH, ret->path, NULL)) {
		P_ERROR ("PDir: failed to call GetFullPathNameA()");
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
		P_ERROR ("PDir: failed to call FindFirstFileA()");
		p_free (ret);
		return NULL;
	}

	ret->cached = TRUE;
	ret->orig_path = p_strdup (path);

	return ret;
}

P_LIB_API pboolean
p_dir_create (const pchar	*path,
	      pint		mode)
{
	P_UNUSED (mode);

	if (path == NULL)
		return FALSE;

	if (p_dir_is_exists (path))
		return TRUE;

	return CreateDirectoryA (path, NULL) ? TRUE : FALSE;
}

P_LIB_API pboolean
p_dir_remove (const pchar *path)
{
	if (path == NULL)
		return FALSE;

	if (!p_dir_is_exists (path))
		return FALSE;

	return RemoveDirectoryA (path) ? TRUE : FALSE;
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
p_dir_get_next_entry (PDir *dir)
{
	PDirEntry	*ret;
	DWORD		dwAttrs;

	if (dir == NULL)
		return NULL;

	if (dir->cached == TRUE)
		dir->cached = FALSE;
	else {
		if (dir->search_handle == INVALID_HANDLE_VALUE)
			return NULL;

		if (!FindNextFileA (dir->search_handle, &dir->find_data)) {
			FindClose (dir->search_handle);
			dir->search_handle = INVALID_HANDLE_VALUE;
			return NULL;
		}
	}

	if ((ret = p_malloc0 (sizeof (PDirEntry))) == NULL) {
		P_ERROR ("PDir: failed to allocate memory");
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

P_LIB_API void
p_dir_rewind (PDir *dir)
{
	if (dir == NULL)
		return;

	if (dir->search_handle != INVALID_HANDLE_VALUE)
		FindClose (dir->search_handle);

	dir->search_handle = FindFirstFileA (dir->path, &dir->find_data);

	if (dir->search_handle == INVALID_HANDLE_VALUE) {
		P_ERROR ("PDir: failed to rewind directory");
		dir->cached = FALSE;
	} else
		dir->cached = TRUE;
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
