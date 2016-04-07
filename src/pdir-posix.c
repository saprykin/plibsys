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
#ifdef P_OS_SCO
#  include <limits.h>
#endif
#include <sys/stat.h>
#include <sys/types.h>

#if defined(P_OS_SOLARIS) || defined(P_OS_QNX6) || defined(P_OS_UNIXWARE) || defined(P_OS_SCO) || \
    defined(P_OS_IRIX)
#  define P_DIR_NEED_BUF_ALLOC 1
#endif

#ifdef P_DIR_NEED_BUF_ALLOC
#  if defined(P_OS_SCO)
#    define P_DIR_NEED_SIMPLE_R 1
#  endif
#endif

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
#ifdef P_DIR_NEED_BUF_ALLOC
	struct dirent	*dirent_st;
#else
	struct dirent	dirent_st;
#endif
	struct stat	sb;
	pchar		*entry_path;
	pint		path_len;
#ifdef P_DIR_NEED_BUF_ALLOC
	pint		name_max;
#endif

	if (dir == NULL || dir->dir == NULL) {
		p_error_set_error_p (error,
				     (pint) P_ERROR_IO_INVALID_ARGUMENT,
				     0,
				     "Invalid input argument");
		return NULL;
	}

#if defined(P_OS_SOLARIS)
	name_max = (pint) (FILENAME_MAX);
#elif defined(P_OS_SCO) || defined(P_OS_IRIX)
	name_max = (pint) pathconf (dir->orig_path, _PC_NAME_MAX);

	if (name_max == -1) {
		if (__p_error_get_last_error () == 0)
			name_max = _POSIX_PATH_MAX;
		else {
			p_error_set_error_p (error,
					     (pint) P_ERROR_IO_FAILED,
					     0,
					     "Failed to get NAME_MAX using pathconf()");
			return NULL;
		}
	}
#elif defined(P_OS_QNX6) || defined(P_OS_UNIXWARE)
	name_max = (pint) (NAME_MAX);
#endif

#ifdef P_DIR_NEED_BUF_ALLOC
	if ((dirent_st = p_malloc0 (sizeof (struct dirent) + name_max + 1)) == NULL) {
		p_error_set_error_p (error,
				     (pint) P_ERROR_IO_NO_RESOURCES,
				     0,
				     "Failed to allocate memory for internal directory entry");
		return NULL;
	}

#  ifdef P_DIR_NEED_SIMPLE_R
	if ((dir->dir_result = readdir_r (dir->dir, dirent_st)) == NULL) {
		if (__p_error_get_last_error () != 0) {
			p_error_set_error_p (error,
					     (pint) __p_error_get_last_io (),
					     __p_error_get_last_error (),
					     "Failed to call readdir_r() to read directory stream");
			p_free (dirent_st);
			return NULL;
		}
	}
#  else
	if (readdir_r (dir->dir, dirent_st, &dir->dir_result) != 0) {
		p_error_set_error_p (error,
				     (pint) __p_error_get_last_io (),
				     __p_error_get_last_error (),
				     "Failed to call readdir_r() to read directory stream");
		p_free (dirent_st);
		return NULL;
	}
#  endif
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
#ifdef P_DIR_NEED_BUF_ALLOC
		p_free (dirent_st);
#endif
		return NULL;
	}

	if ((ret = p_malloc0 (sizeof (PDirEntry))) == NULL) {
		p_error_set_error_p (error,
				     (pint) P_ERROR_IO_NO_RESOURCES,
				     0,
				     "Failed to allocate memory for directory entry");
#ifdef P_DIR_NEED_BUF_ALLOC
		p_free (dirent_st);
#endif
		return NULL;
	}

#ifdef P_DIR_NEED_BUF_ALLOC
	ret->name = p_strdup (dirent_st->d_name);
	p_free (dirent_st);
#else
	ret->name = p_strdup (dirent_st.d_name);
#endif

	path_len = strlen (dir->path);

	if ((entry_path = p_malloc0 (path_len + strlen (ret->name) + 2)) == NULL) {
		P_WARNING ("PDir: failed to allocate memory for stat()");
		ret->type = P_DIR_ENTRY_TYPE_OTHER;
		return ret;
	}

	strcat (entry_path, dir->path);
	*(entry_path + path_len) = '/';
	strcat (entry_path + path_len + 1, ret->name);

	if (stat (entry_path, &sb) != 0) {
		P_WARNING ("PDir: failed to call stat()");
		ret->type = P_DIR_ENTRY_TYPE_OTHER;
		p_free (entry_path);
		return ret;
	}

	p_free (entry_path);

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
