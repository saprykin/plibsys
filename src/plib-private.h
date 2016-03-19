/*
 * Copyright (C) 2013-2016 Alexander Saprykin <xelfium@gmail.com>
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

#if !defined (__PLIB_H_INSIDE__) && !defined (PLIB_COMPILATION)
#  error "Header files shouldn't be included directly, consider using <plib.h> instead."
#endif

#ifndef __PLIBPRIVATE_H__
#define __PLIBPRIVATE_H__

#include "pmacros.h"
#include "ptypes.h"
#include "perror.h"

/* Disable false "no prototype" warnings for Borland */
#ifdef P_CC_BORLAND
#  pragma warn -8065
#endif

P_BEGIN_DECLS

/** Base tree leaf structure */
typedef struct _PTreeBaseNode {
	struct _PTreeBaseNode	*left;	/**< Left child		*/
	struct _PTreeBaseNode	*right;	/**< Right child	*/
	ppointer		key;	/**< Node key		*/
	ppointer		value;	/**< Node value		*/
} __PTreeBaseNode;

#ifndef PLIB_HAS_SOCKLEN_T
typedef int socklen_t;
#endif

#ifndef PLIB_HAS_SOCKADDR_STORAGE
/* According to RFC 2553 */
#  define _PLIB_SS_MAXSIZE	128
#  define _PLIB_SS_ALIGNSIZE	(sizeof (pint64))

#  ifdef PLIB_SOCKADDR_HAS_SA_LEN
#    define _PLIB_SS_PAD1SIZE	(_PLIB_SS_ALIGNSIZE - (sizeof (puchar) + sizeof (puchar)))
#  else
#    define _PLIB_SS_PAD1SIZE	(_PLIB_SS_ALIGNSIZE - sizeof (puchar))
#  endif

#  define _PLIB_SS_PAD2SIZE	(_PLIB_SS_MAXSIZE - (sizeof (puchar) + _PLIB_SS_PAD1SIZE + _PLIB_SS_ALIGNSIZE))

struct sockaddr_storage {
#  ifdef PLIB_SOCKADDR_HAS_SA_LEN
	puchar		ss_len;
#  endif
	puchar		ss_family;
	pchar		__ss_pad1[_PLIB_SS_PAD1SIZE];
	pint64		__ss_align;
	pchar		__ss_pad2[_PLIB_SS_PAD2SIZE];
};
#endif

#ifndef P_OS_WIN
/**
 * @brief Gets temporary directory on UNIX systems.
 * @return Temporary directory.
 * @since 0.0.1
 */
pchar *		__p_ipc_unix_get_temp_dir		(void);

/* Create file for System V IPC, if needed
 * Returns: -1 = error, 0 = file successfully created, 1 = file already exists */
/**
 * @brief Creates file for System V IPC usage.
 * @param file_name File name to create.
 * @return -1 in case of error, 0 if all was OK, and 1 if file already exists.
 * @since 0.0.1
 */
pint		__p_ipc_unix_create_key_file		(const pchar *file_name);

/**
 * @brief Wrapps ftok() UNIX call for uniquer IPC key.
 * @param file_name File name for ftok() call.
 * @return Key in case of success, -1 otherwise.
 * @since 0.0.1
 */
pint		__p_ipc_unix_get_ftok_key		(const pchar *file_name);
#endif /* !P_OS_WIN */

/**
 * @brief Generates platform-independent key for IPC usage, object name for Windows and
 * file name to use with ftok () for UNIX-like systems.
 * @param name Object name.
 * @param posix TRUE if key will be used for POSIX IPC calls, otherwise FALSE. This
 * parameter is not used on Windows platform.
 * @return Platform-independent key for IPC usage.
 * @since 0.0.1
 */
pchar *		__p_ipc_get_platform_key		(const pchar *name, pboolean posix);

/**
 * @brief Gets IO error code from the system error code
 * @param err_code System error code.
 * @return IO error code.
 */
PErrorIO	__p_error_get_io_from_system		(pint err_code);

/**
 * @brief Gets IO error code from the last call result
 * @return IO error code.
 */
PErrorIO	__p_error_get_last_io			();

/**
 * @brief Gets IPC error code from the system error code
 * @param err_code System error code.
 * @return IPC error code.
 */
PErrorIPC	__p_error_get_ipc_from_system		(pint err_code);

/**
 * @brief Gets IPC error code from the last call result
 * @return IPC error code.
 */
PErrorIPC	__p_error_get_last_ipc			();

/**
 * @brief Gets last native error code
 * @return Last native error code.
 */
pint		__p_error_get_last_error		();

P_END_DECLS

#endif /* __PLIBPRIVATE_H__ */
