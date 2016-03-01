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
#include "pfile.h"
#include "pdir.h"
#include "plib-private.h"

#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

struct _PDir {
	DIR *		dir;
	struct dirent	*dir_result;
	pchar		*path;
	pchar		*orig_path;
};

P_LIB_API PDir *
p_dir_new (const pchar	*path,
	   PError	**error)
{
	PDir	*ret;
	DIR	*dir;
	pchar	*pathp;

	if (path == NULL) {
		p_error_set_error_p (error,
				     (pint) P_ERROR_IO_INVALID_ARGUMENT,
				     0,
				     "Invalid input argument");
		return NULL;
	}

	if ((dir = opendir (path)) == NULL) {
		p_error_set_error_p (error,
				     (pint) __p_error_get_last_io (),
				     __p_error_get_last_error (),
				     "Failed to call opendir() to open directory stream");
		return NULL;
	}

	if ((ret = p_malloc0 (sizeof (PDir))) == NULL) {
		p_error_set_error_p (error,
				     (pint) P_ERROR_IO_NO_RESOURCES,
				     0,
				     "Failed to allocate memory for directory structure");
		closedir (dir);
		return NULL;
	}

	ret->dir	= dir;
	ret->path	= p_strdup (path);
	ret->orig_path	= p_strdup (path);

	pathp = ret->path + strlen (ret->path) - 1;

	if (*pathp == '/' || *pathp == '\\')
		*pathp = '\0';

	return ret;
}

P_LIB_API pboolean
p_dir_create (const pchar	*path,
	      pint		mode,
	      PError		**error)
{
	if (path == NULL) {
		p_error_set_error_p (error,
				     (pint) P_ERROR_IO_INVALID_ARGUMENT,
				     0,
				     "Invalid input argument");
		return FALSE;
	}

	if (p_dir_is_exists (path))
		return TRUE;

	if (mkdir (path, mode) != 0) {
		p_error_set_error_p (error,
				     (pint) __p_error_get_last_io (),
				     __p_error_get_last_error (),
				     "Failed to call mkdir() to create directory");
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

	if (rmdir (path) != 0) {
		p_error_set_error_p (error,
				     (pint) __p_error_get_last_io (),
				     __p_error_get_last_error (),
				     "Failed to call rmdir() to remove directory");
		return FALSE;
	} else
		return TRUE;
}

P_LIB_API pboolean
p_dir_is_exists (const pchar *path)
{
	struct stat sb;

	if (path == NULL)
		return FALSE;

	return (stat (path, &sb) == 0 && S_ISDIR (sb.st_mode)) ? TRUE : FALSE;
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
#if defined(P_OS_SOLARIS) || defined (P_OS_QNX6)
	struct dirent	*dirent_st;
#else
	struct dirent	dirent_st;
#endif
	struct stat	sb;
	pchar		*entry_path;
	pint		path_len;

	if (dir == NULL || dir->dir == NULL) {
		p_error_set_error_p (error,
				     (pint) P_ERROR_IO_INVALID_ARGUMENT,
				     0,
				     "Invalid input argument");
		return NULL;
	}

#if defined(P_OS_SOLARIS)
	if ((dirent_st = p_malloc0 (sizeof (struct dirent) + FILENAME_MAX + 1)) == NULL) {
		p_error_set_error_p (error,
				     (pint) P_ERROR_IO_NO_RESOURCES,
				     0,
				     "Failed to allocate memory for internal directory entry");
		return NULL;
	}
#elif defined(P_OS_QNX6)
	if ((dirent_st = p_malloc0 (sizeof (struct dirent) + NAME_MAX + 1)) == NULL) {
		p_error_set_error_p (error,
				     (pint) P_ERROR_IO_NO_RESOURCES,
				     0,
				     "Failed to allocate memory for internal directory entry");
		return NULL;
	}
#endif

#if defined(P_OS_SOLARIS) || defined(P_OS_QNX6)
	if (readdir_r (dir->dir, dirent_st, &dir->dir_result) != 0) {
		p_error_set_error_p (error,
				     (pint) __p_error_get_last_io (),
				     __p_error_get_last_error (),
				     "Failed to call readdir_r() to read directory stream");
		p_free (dirent_st);
		return NULL;
	}
#else
	if (readdir_r (dir->dir, &dirent_st, &dir->dir_result) != 0) {
		p_error_set_error_p (error,
				     (pint) __p_error_get_last_io (),
				     __p_error_get_last_error (),
				     "Failed to call readdir_r() to read directory stream");
		return NULL;
	}
#endif

	if (dir->dir_result == NULL) {
#if defined(P_OS_SOLARIS) || defined(P_OS_QNX6)
		p_free (dirent_st);
#endif
		return NULL;
	}

	if ((ret = p_malloc0 (sizeof (PDirEntry))) == NULL) {
		p_error_set_error_p (error,
				     (pint) P_ERROR_IO_NO_RESOURCES,
				     0,
				     "Failed to allocate memory for directory entry");
#if defined(P_OS_SOLARIS) || defined(P_OS_QNX6)
		p_free (dirent_st);
#endif
		return NULL;
	}

#if defined(P_OS_SOLARIS) || defined(P_OS_QNX6)
	ret->name = p_strdup (dirent_st->d_name);
	p_free (dirent_st);
#else
	ret->name = p_strdup (dirent_st.d_name);
#endif

	path_len = strlen (dir->path);
	entry_path = p_malloc0 (path_len + strlen (ret->name) + 2);

	strcat (entry_path, dir->path);
	*(entry_path + path_len) = '/';
	strcat (entry_path + path_len + 1, ret->name);

	if (stat (entry_path, &sb) != 0) {
		P_WARNING ("PDir: failed to call stat()");
		ret->type = P_DIR_ENTRY_TYPE_OTHER;
		return ret;
	}

	if (S_ISDIR (sb.st_mode))
		ret->type = P_DIR_ENTRY_TYPE_DIR;
	else if (S_ISREG (sb.st_mode))
		ret->type = P_DIR_ENTRY_TYPE_FILE;
	else
		ret->type = P_DIR_ENTRY_TYPE_OTHER;

	return ret;
}

P_LIB_API pboolean
p_dir_rewind (PDir	*dir,
	      PError	**error)
{
	if (dir == NULL || dir->dir == NULL) {
		p_error_set_error_p (error,
				     (pint) P_ERROR_IO_INVALID_ARGUMENT,
				     0,
				     "Invalid input argument");
		return FALSE;
	}

	rewinddir (dir->dir);

	return TRUE;
}

P_LIB_API void
p_dir_free (PDir *dir)
{
	if (dir == NULL)
		return;

	if (dir->dir != NULL) {
		if (closedir (dir->dir) != 0)
			P_ERROR ("PDir: error while closing the directory");
	}

	p_free (dir->path);
	p_free (dir->orig_path);
	p_free (dir);
}
