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
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

#if !defined (__PLIBSYS_H_INSIDE__) && !defined (PLIBSYS_COMPILATION)
#  error "Header files shouldn't be included directly, consider using <plibsys.h> instead."
#endif

#ifndef __PLIBSYSPRIVATE_H__
#define __PLIBSYSPRIVATE_H__

#include "pmacros.h"
#include "ptypes.h"
#include "perrortypes.h"
#include "puthread.h"

P_BEGIN_DECLS

/** Base thread structure */
typedef struct _PUThreadBase {
	pint			ref_count;	/**< Reference counter.	*/
	pint			ret_code;	/**< Return code.	*/
	pboolean		ours;		/**< Our thread flag.	*/
	pboolean		joinable;	/**< Joinable flag.	*/
	PUThreadFunc		func;		/**< Thread routine.	*/
	ppointer		data;		/**< Thread input data.	*/
	PUThreadPriority	prio;		/**< Thread priority.	*/
} __PUThreadBase;

/** Base tree leaf structure. */
typedef struct _PTreeBaseNode {
	struct _PTreeBaseNode	*left;		/**< Left child.	*/
	struct _PTreeBaseNode	*right;		/**< Right child.	*/
	ppointer		key;		/**< Node key.		*/
	ppointer		value;		/**< Node value.	*/
} __PTreeBaseNode;

#ifndef PLIBSYS_HAS_SOCKLEN_T
typedef int socklen_t;
#endif

#ifndef PLIBSYS_HAS_SOCKADDR_STORAGE
/* According to RFC 2553 */
#  define _PLIBSYS_SS_MAXSIZE	128
#  define _PLIBSYS_SS_ALIGNSIZE	(sizeof (pint64))

#  ifdef PLIBSYS_SOCKADDR_HAS_SA_LEN
#    define _PLIBSYS_SS_PAD1SIZE	(_PLIBSYS_SS_ALIGNSIZE - (sizeof (puchar) + sizeof (puchar)))
#  else
#    define _PLIBSYS_SS_PAD1SIZE	(_PLIBSYS_SS_ALIGNSIZE - sizeof (puchar))
#  endif

#  define _PLIBSYS_SS_PAD2SIZE	(_PLIBSYS_SS_MAXSIZE - (sizeof (puchar) + _PLIBSYS_SS_PAD1SIZE + _PLIBSYS_SS_ALIGNSIZE))

struct sockaddr_storage {
#  ifdef PLIBSYS_SOCKADDR_HAS_SA_LEN
	puchar		ss_len;
#  endif
	puchar		ss_family;
	pchar		__ss_pad1[_PLIBSYS_SS_PAD1SIZE];
	pint64		__ss_align;
	pchar		__ss_pad2[_PLIBSYS_SS_PAD2SIZE];
};
#endif

#ifndef P_OS_WIN
/**
 * @brief Gets a temporary directory on UNIX systems.
 * @return Temporary directory.
 * @since 0.0.1
 */
pchar *		__p_ipc_unix_get_temp_dir		(void);

/* Create file for System V IPC, if needed
 * Returns: -1 = error, 0 = file successfully created, 1 = file already exists */
/**
 * @brief Creates a file for System V IPC usage.
 * @param file_name File name to create.
 * @return -1 in case of error, 0 if all was OK, and 1 if the file already
 * exists.
 * @since 0.0.1
 */
pint		__p_ipc_unix_create_key_file		(const pchar	*file_name);

/**
 * @brief Wrapps ftok() UNIX call for a uniquer IPC key.
 * @param file_name File name for ftok() call.
 * @return Key in case of success, -1 otherwise.
 * @since 0.0.1
 */
pint		__p_ipc_unix_get_ftok_key		(const pchar	*file_name);
#endif /* !P_OS_WIN */

/**
 * @brief Generates a platform independent key for IPC usage, an object name for
 * Windows and a file name to use with ftok () for UNIX-like systems.
 * @param name Object name.
 * @param posix TRUE if the key will be used for the POSIX IPC calls,
 * otherwise FALSE. This parameter is not used on Windows platform.
 * @return Platform independent key for IPC usage.
 * @since 0.0.1
 */
pchar *		__p_ipc_get_platform_key		(const pchar	*name,
							 pboolean	posix);

/**
 * @brief Gets an IO error code from a system error code.
 * @param err_code System error code.
 * @return IO error code.
 */
PErrorIO	__p_error_get_io_from_system		(pint		err_code);

/**
 * @brief Gets an IO error code from the last call result.
 * @return IO error code.
 */
PErrorIO	__p_error_get_last_io			(void);

/**
 * @brief Gets an IPC error code from a system error code
 * @param err_code System error code.
 * @return IPC error code.
 */
PErrorIPC	__p_error_get_ipc_from_system		(pint		err_code);

/**
 * @brief Gets an IPC error code from the last call result.
 * @return IPC error code.
 */
PErrorIPC	__p_error_get_last_ipc			(void);

/**
 * @brief Gets the last native error code.
 * @return Last native error code.
 */
pint		__p_error_get_last_error		(void);

P_END_DECLS

#endif /* __PLIBSYSPRIVATE_H__ */
