/*
 * Copyright (C) 2010-2016 Alexander Saprykin <xelfium@gmail.com>
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
#include "psemaphore.h"
#include "pcryptohash.h"
#include "pstring.h"

#include <stdlib.h>
#include <string.h>

#ifndef P_OS_WIN
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ipc.h>
#else
#include <windows.h>
#endif

#ifndef P_OS_WIN
pchar *
__p_ipc_unix_get_temp_dir (void)
{
	pchar	*str, *ret;
	pint	len;

#ifdef P_tmpdir
	if (strlen (P_tmpdir) > 0)
		str = p_strdup (P_tmpdir);
	else
		return p_strdup ("/tmp/");
#else
	const pchar *tmp_env;

	tmp_env = getenv ("TMPDIR");

	if (tmp_env != NULL)
		str = p_strdup (tmp_env);
	else
		return p_strdup ("/tmp/");
#endif /* P_tmpdir */

	/* Now we need to ensure that we have only one trailing slash */
	len = strlen (str);
	while (*(str + --len) == '/')
		;
	*(str + ++len) = '\0';

	/* len + / + zero symbol */
	if ((ret = p_malloc0 (len + 2)) == NULL) {
		p_free (str);
		return NULL;
	}

	strcpy (ret, str);
	strcat (ret, "/");

	return ret;
}

/* Create file for System V IPC, if needed
 * Returns: -1 = error, 0 = file successfully created, 1 = file already exists */
pint
__p_ipc_unix_create_key_file (const pchar *file_name)
{
	pint fd;

	if (file_name == NULL)
		return -1;

	if ((fd = open (file_name, O_CREAT | O_EXCL | O_RDONLY, 0640)) == -1)
		/* file already exists */
		return (errno == EEXIST) ? 1 : -1;
	else
		return close (fd);
}

pint
__p_ipc_unix_get_ftok_key (const pchar *file_name)
{
	struct stat st_info;

	if (file_name == NULL)
		return -1;

	if (stat (file_name, &st_info) == -1)
		return -1;

	return ftok (file_name, 'P');
}

PSemaphoreError
__p_ipc_unix_get_semaphore_error ()
{
	pint err_code = errno;

	switch (err_code) {
	case 0:
		return P_SEM_ERROR_NONE;
#ifdef EACCES
	case EACCES:
		return P_SEM_ERROR_ACCESS;
#endif

#ifdef EPERM
	case EPERM:
		return P_SEM_ERROR_ACCESS;
#endif

#ifdef EEXIST
	case EEXIST:
		return P_SEM_ERROR_EXISTS;
#endif

#ifdef E2BIG
	case E2BIG:
		return P_SEM_ERROR_INVALID_ARGUMENT;
#endif

#ifdef EFBIG
	case EFBIG:
		return P_SEM_ERROR_INVALID_ARGUMENT;
#endif

#ifdef EINVAL
	case EINVAL:
		return P_SEM_ERROR_INVALID_ARGUMENT;
#endif

#ifdef ERANGE
	case ERANGE:
		return P_SEM_ERROR_INVALID_ARGUMENT;
#endif

#ifdef ENOMEM
	case ENOMEM:
		return P_SEM_ERROR_NO_RESOURCES;
#endif

#ifdef EMFILE
	case EMFILE:
		return P_SEM_ERROR_NO_RESOURCES;
#endif

#ifdef ENFILE
	case ENFILE:
		return P_SEM_ERROR_NO_RESOURCES;
#endif

#ifdef ENOSPC
	case ENOSPC:
		return P_SEM_ERROR_NO_RESOURCES;
#endif

#ifdef EIDRM
	case EIDRM:
		return P_SEM_ERROR_NOT_EXISTS;
#endif

#ifdef ENOENT
	case ENOENT:
		return P_SEM_ERROR_NOT_EXISTS;
#endif

#ifdef EOVERFLOW
	case EOVERFLOW:
		return P_SEM_ERROR_OVERFLOW;
#endif

#ifdef ENOSYS
	case ENOSYS:
		return P_SEM_ERROR_NOT_IMPLEMENTED;
#endif

#ifdef EDEADLK
	case EDEADLK:
		return P_SEM_ERROR_DEADLOCK;
#endif

#ifdef ENAMETOOLONG
	case ENAMETOOLONG:
		return P_SEM_ERROR_NAMETOOLONG;
#endif
	default:
		return P_SEM_ERROR_FAILED;
	}
}

#endif /* !P_OS_WIN */

/* Returns platform-independent key for IPC usage, object name for Windows and
 * file name to use with ftok () for UNIX-like systems */
pchar *
__p_ipc_get_platform_key (const pchar *name, pboolean posix)
{
	PCryptoHash	*sha1;
	pchar		*hash_str;

#ifdef P_OS_WIN
	P_UNUSED (posix);
#else
	pchar		*path_name, *tmp_path;
#endif

	if (name == NULL)
		return NULL;

	if ((sha1 = p_crypto_hash_new (P_CRYPTO_HASH_TYPE_SHA1)) == NULL)
		return NULL;

	p_crypto_hash_update (sha1, (const puchar *) name, strlen (name));

	hash_str = p_crypto_hash_get_string (sha1);
	p_crypto_hash_free (sha1);

	if (hash_str == NULL)
		return NULL;

#ifdef P_OS_WIN
	return hash_str;
#else
	if (posix) {
		/* POSIX semaphores which are named kinda like '/semname' */
#  ifdef P_OS_BSD4
		/* BSD implementation of POSIX semaphores has restriction for the name - max 14
		   characters */
		if ((path_name = p_malloc0 (15)) == NULL) {
#  else
		if ((path_name = p_malloc0 (strlen (hash_str) + 2)) == NULL) {
#  endif
			p_free (hash_str);
			return NULL;
		}

		strcpy (path_name, "/");
		strncat (path_name, hash_str, 13);
	} else {
		tmp_path = __p_ipc_unix_get_temp_dir ();

		/* tmp dir + filename + zero symbol */
		path_name = p_malloc0 (strlen (tmp_path) + strlen (hash_str) + 1);

		if ((path_name) == NULL) {
			p_free (tmp_path);
			p_free (hash_str);
			return NULL;
		}

		strcpy (path_name, tmp_path);
		strcat (path_name, hash_str);
		p_free (tmp_path);
	}

	p_free (hash_str);
	return path_name;
#endif
}
