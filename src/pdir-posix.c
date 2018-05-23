/*
 * The MIT License
 *
 * Copyright (C) 2015-2017 Alexander Saprykin <saprykin.spb@gmail.com>
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
#include "perror.h"
#include "pfile.h"
#include "pmem.h"
#include "pstring.h"
#include "perror-private.h"

#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <limits.h>
#include <sys/stat.h>
#include <sys/types.h>

#if defined (__GLIBC__) && (__GLIBC__ >= 2) && (__GLIBC_MINOR__ >= 24)
#  define P_DIR_NON_REENTRANT 1
# elif defined (P_OS_SOLARIS) || defined (P_OS_QNX6) || defined (P_OS_UNIXWARE) || \
       defined (P_OS_SCO)     || defined (P_OS_IRIX) || defined (P_OS_HAIKU)
#  define P_DIR_NEED_BUF_ALLOC 1
#endif

#ifdef P_DIR_NEED_BUF_ALLOC
#  if defined (P_OS_SCO)
#    define P_DIR_NEED_SIMPLE_R 1
#  endif
#else
#  if defined (P_OS_BEOS) || defined (P_OS_AMIGA)
#    define P_DIR_NON_REENTRANT 1
#  endif
#endif

struct PDir_ {
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

	if (P_UNLIKELY (path == NULL)) {
		p_error_set_error_p (error,
				     (pint) P_ERROR_IO_INVALID_ARGUMENT,
				     0,
				     "Invalid input argument");
		return NULL;
	}

	if (P_UNLIKELY ((dir = opendir (path)) == NULL)) {
		p_error_set_error_p (error,
				     (pint) p_error_get_last_io (),
				     p_error_get_last_system (),
				     "Failed to call opendir() to open directory stream");
		return NULL;
	}

	if (P_UNLIKELY ((ret = p_malloc0 (sizeof (PDir))) == NULL)) {
		p_error_set_error_p (error,
				     (pint) P_ERROR_IO_NO_RESOURCES,
				     0,
				     "Failed to allocate memory for directory structure");
		closedir (dir);
		return NULL;
	}

	ret->dir       = dir;
	ret->path      = p_strdup (path);
	ret->orig_path = p_strdup (path);

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
	if (P_UNLIKELY (path == NULL)) {
		p_error_set_error_p (error,
				     (pint) P_ERROR_IO_INVALID_ARGUMENT,
				     0,
				     "Invalid input argument");
		return FALSE;
	}

	if (p_dir_is_exists (path))
		return TRUE;

	if (P_UNLIKELY (mkdir (path, (mode_t) mode) != 0)) {
		p_error_set_error_p (error,
				     (pint) p_error_get_last_io (),
				     p_error_get_last_system (),
				     "Failed to call mkdir() to create directory");
		return FALSE;
	} else
		return TRUE;
}

P_LIB_API pboolean
p_dir_remove (const pchar	*path,
	      PError		**error)
{
	if (P_UNLIKELY (path == NULL)) {
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

	if (P_UNLIKELY (rmdir (path) != 0)) {
		p_error_set_error_p (error,
				     (pint) p_error_get_last_io (),
				     p_error_get_last_system (),
				     "Failed to call rmdir() to remove directory");
		return FALSE;
	} else
		return TRUE;
}

P_LIB_API pboolean
p_dir_is_exists (const pchar *path)
{
	struct stat sb;

	if (P_UNLIKELY (path == NULL))
		return FALSE;

	return (stat (path, &sb) == 0 && S_ISDIR (sb.st_mode)) ? TRUE : FALSE;
}

P_LIB_API pchar *
p_dir_get_path (const PDir *dir)
{
	if (P_UNLIKELY (dir == NULL))
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
#elif !defined (P_DIR_NON_REENTRANT)
	struct dirent	dirent_st;
#endif
	struct stat	sb;
	pchar		*entry_path;
	psize		path_len;
#ifdef P_DIR_NEED_BUF_ALLOC
	pint		name_max;
#endif

	if (P_UNLIKELY (dir == NULL || dir->dir == NULL)) {
		p_error_set_error_p (error,
				     (pint) P_ERROR_IO_INVALID_ARGUMENT,
				     0,
				     "Invalid input argument");
		return NULL;
	}

#ifdef P_DIR_NEED_BUF_ALLOC
#  if defined (P_OS_SOLARIS)
	name_max = (pint) (FILENAME_MAX);
#  elif defined (P_OS_SCO) || defined (P_OS_IRIX)
	name_max = (pint) pathconf (dir->orig_path, _PC_NAME_MAX);

	if (name_max == -1) {
		if (p_error_get_last_system () == 0)
			name_max = _POSIX_PATH_MAX;
		else {
			p_error_set_error_p (error,
					     (pint) P_ERROR_IO_FAILED,
					     0,
					     "Failed to get NAME_MAX using pathconf()");
			return NULL;
		}
	}
#  elif defined (P_OS_QNX6) || defined (P_OS_UNIXWARE) || defined (P_OS_HAIKU)
	name_max = (pint) (NAME_MAX);
#  endif

	if (P_UNLIKELY ((dirent_st = p_malloc0 (sizeof (struct dirent) + name_max + 1)) == NULL)) {
		p_error_set_error_p (error,
				     (pint) P_ERROR_IO_NO_RESOURCES,
				     0,
				     "Failed to allocate memory for internal directory entry");
		return NULL;
	}

#  ifdef P_DIR_NEED_SIMPLE_R
	p_error_set_last_system (0);

	if ((dir->dir_result = readdir_r (dir->dir, dirent_st)) == NULL) {
		if (P_UNLIKELY (p_error_get_last_system () != 0)) {
			p_error_set_error_p (error,
					     (pint) p_error_get_last_io (),
					     p_error_get_last_system (),
					     "Failed to call readdir_r() to read directory stream");
			p_free (dirent_st);
			return NULL;
		}
	}
#  else
	if (P_UNLIKELY (readdir_r (dir->dir, dirent_st, &dir->dir_result) != 0)) {
		p_error_set_error_p (error,
				     (pint) p_error_get_last_io (),
				     p_error_get_last_system (),
				     "Failed to call readdir_r() to read directory stream");
		p_free (dirent_st);
		return NULL;
	}
#  endif
#else
#  ifdef P_DIR_NON_REENTRANT
	p_error_set_last_system (0);

	if ((dir->dir_result = readdir (dir->dir)) == NULL) {
		if (P_UNLIKELY (p_error_get_last_system () != 0)) {
			p_error_set_error_p (error,
					     (pint) p_error_get_last_io (),
					     p_error_get_last_system (),
					     "Failed to call readdir() to read directory stream");
			return NULL;
		}
	}
#  else
	if (P_UNLIKELY (readdir_r (dir->dir, &dirent_st, &dir->dir_result) != 0)) {
		p_error_set_error_p (error,
				     (pint) p_error_get_last_io (),
				     p_error_get_last_system (),
				     "Failed to call readdir_r() to read directory stream");
		return NULL;
	}
#  endif
#endif

	if (dir->dir_result == NULL) {
#ifdef P_DIR_NEED_BUF_ALLOC
		p_free (dirent_st);
#endif
		return NULL;
	}

	if (P_UNLIKELY ((ret = p_malloc0 (sizeof (PDirEntry))) == NULL)) {
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
#  ifdef P_DIR_NON_REENTRANT
	ret->name = p_strdup (dir->dir_result->d_name);
#  else
	ret->name = p_strdup (dirent_st.d_name);
#  endif
#endif

	path_len = strlen (dir->path);

	if (P_UNLIKELY ((entry_path = p_malloc0 (path_len + strlen (ret->name) + 2)) == NULL)) {
		P_WARNING ("PDir::p_dir_get_next_entry: failed to allocate memory for stat()");
		ret->type = P_DIR_ENTRY_TYPE_OTHER;
		return ret;
	}

	strcat (entry_path, dir->path);
	*(entry_path + path_len) = '/';
	strcat (entry_path + path_len + 1, ret->name);

	if (P_UNLIKELY (stat (entry_path, &sb) != 0)) {
		P_WARNING ("PDir::p_dir_get_next_entry: stat() failed");
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
	if (P_UNLIKELY (dir == NULL || dir->dir == NULL)) {
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
	if (P_UNLIKELY (dir == NULL))
		return;

	if (P_LIKELY (dir->dir != NULL)) {
		if (P_UNLIKELY (closedir (dir->dir) != 0))
			P_ERROR ("PDir::p_dir_free: closedir() failed");
	}

	p_free (dir->path);
	p_free (dir->orig_path);
	p_free (dir);
}
