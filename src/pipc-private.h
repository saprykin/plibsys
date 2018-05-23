/*
 * The MIT License
 *
 * Copyright (C) 2016-2017 Alexander Saprykin <saprykin.spb@gmail.com>
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

#if !defined (PLIBSYS_H_INSIDE) && !defined (PLIBSYS_COMPILATION)
#  error "Header files shouldn't be included directly, consider using <plibsys.h> instead."
#endif

#ifndef PLIBSYS_HEADER_PIPC_PRIVATE_H
#define PLIBSYS_HEADER_PIPC_PRIVATE_H

#include "pmacros.h"
#include "ptypes.h"

P_BEGIN_DECLS

#if !defined (P_OS_WIN) && !defined (P_OS_OS2)
/**
 * @brief Gets a temporary directory on UNIX systems.
 * @return Temporary directory.
 */
pchar *		p_ipc_unix_get_temp_dir		(void);

/* Create file for System V IPC, if needed
 * Returns: -1 = error, 0 = file successfully created, 1 = file already exists */
/**
 * @brief Creates a file for System V IPC usage.
 * @param file_name File name to create.
 * @return -1 in case of error, 0 if all was OK, and 1 if the file already
 * exists.
 */
pint		p_ipc_unix_create_key_file	(const pchar	*file_name);

/**
 * @brief Wrapps the ftok() UNIX call for a unique IPC key.
 * @param file_name File name for ftok() call.
 * @return Key in case of success, -1 otherwise.
 */
pint		p_ipc_unix_get_ftok_key		(const pchar	*file_name);
#endif /* !P_OS_WIN && !P_OS_OS2 */

/**
 * @brief Generates a platform independent key for IPC usage, an object name for
 * Windows and a file name to use with ftok () for UNIX-like systems.
 * @param name Object name.
 * @param posix TRUE if the key will be used for the POSIX IPC calls, otherwise
 * FALSE. This parameter is not used on the Windows platform.
 * @return Platform independent key for IPC usage.
 */
pchar *		p_ipc_get_platform_key		(const pchar	*name,
						 pboolean	posix);

P_END_DECLS

#endif /* PLIBSYS_HEADER_PIPC_PRIVATE_H */
