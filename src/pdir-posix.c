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

#include "pmem.h"
#include "pstring.h"
#include "pfile.h"
#include "pdir.h"

#include <stdlib.h>
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
p_dir_new (const pchar	*path)
{
	PDir	*ret;
	DIR	*dir;
	pchar	*pathp;

	if (path == NULL)
		return NULL;

	if ((dir = opendir (path)) == NULL) {
		P_ERROR ("PDir: failed to open directory");
		return NULL;
	}

	if ((ret = p_malloc0 (sizeof (PDir))) == NULL) {
		P_ERROR ("PDir: failed to allocate memory");
		closedir (dir);
		return NULL;
	}

	ret->dir	= dir;
	ret->path	= p_strdup (path);
	ret->orig_path	= p_strdup (path);

	pathp = ret->path + strlen (ret->path) - 1;

	if (*pathp == '/' || *pathp == '\\')
		pathp = '\0';

	return ret;
}

P_LIB_API pboolean
p_dir_create (const pchar	*path,
	      pint		mode)
{
	if (path == NULL)
		return FALSE;

	if (p_dir_is_exists (path))
		return TRUE;

	return (mkdir (path, mode) == 0) ? TRUE : FALSE;
}

P_LIB_API pboolean
p_dir_remove (const pchar *path)
{
	if (path == NULL)
		return FALSE;

	if (!p_dir_is_exists (path))
		return FALSE;

	return rmdir (path) == 0 ? TRUE : FALSE;
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
p_dir_get_next_entry (PDir *dir)
{
	PDirEntry	*ret;
#ifdef P_OS_SOLARIS
	struct dirent	*dirent_st;
#else
	struct dirent	dirent_st;
#endif
	struct stat	sb;
	pchar		*entry_path;
	pint		path_len;

	if (dir == NULL || dir->dir == NULL)
		return NULL;

#ifdef P_OS_SOLARIS
	if ((dirent_st = p_malloc0 (sizeof (struct dirent) + FILENAME_MAX + 1)) == NULL) {
		P_ERROR ("PDir: failed to allocate memory");
		return NULL;
	}
#endif

#ifdef P_OS_SOLARIS
	if (readdir_r (dir->dir, dirent_st, &dir->dir_result) != 0) {
		P_ERROR ("PDir: failed to call readdir_r()");
		p_free (dirent_st);
		return NULL;
	}
#else
	if (readdir_r (dir->dir, &dirent_st, &dir->dir_result) != 0) {	
		P_ERROR ("PDir: failed to call readdir_r()");
		return NULL;
	}
#endif

	if (dir->dir_result == NULL) {
#ifdef P_OS_SOLARIS
		p_free (dirent_st);
#endif
		return NULL;
	}

	if ((ret = p_malloc0 (sizeof (PDirEntry))) == NULL) {
		P_ERROR ("PDir: failed to allocate memory");
#ifdef P_OS_SOLARIS
		p_free (dirent_st);
#endif
		return NULL;
	}

#ifdef P_OS_SOLARIS
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

P_LIB_API void
p_dir_rewind (PDir *dir)
{
	if (dir == NULL || dir->dir == NULL)
		return;

	rewinddir (dir->dir);
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
