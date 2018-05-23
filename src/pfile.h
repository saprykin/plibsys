/*
 * The MIT License
 *
 * Copyright (C) 2010-2016 Alexander Saprykin <saprykin.spb@gmail.com>
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

/**
 * @file pfile.h
 * @brief File operations
 * @author Alexander Saprykin
 *
 * To check file existance use p_file_is_exists(). To remove an exisiting file
 * use p_file_remove().
 *
 * #P_DIR_SEPARATOR provides a platform independent directory separator symbol
 * which you can use to form file or directory path.
 */

#if !defined (PLIBSYS_H_INSIDE) && !defined (PLIBSYS_COMPILATION)
#  error "Header files shouldn't be included directly, consider using <plibsys.h> instead."
#endif

#ifndef PLIBSYS_HEADER_PFILE_H
#define PLIBSYS_HEADER_PFILE_H

#include <pmacros.h>
#include <ptypes.h>
#include <perror.h>

/**
 * @def P_DIR_SEPARATOR
 * @brief A directory separator.
 */
#if defined (P_OS_WIN) || defined (P_OS_OS2)
#  define P_DIR_SEPARATOR "\\"
#else
#  define P_DIR_SEPARATOR "/"
#endif

P_BEGIN_DECLS

/**
 * @brief Checks whether a file exists or not.
 * @param file File name to check.
 * @return TRUE if the file exists, FALSE otherwise.
 * @since 0.0.1
 *
 * On Windows this call doesn't resolve symbolic links, while on UNIX systems
 * does.
 */
P_LIB_API pboolean p_file_is_exists	(const pchar	*file);

/**
 * @brief Removes a file from the disk.
 * @param file File name to remove.
 * @param[out] error Error report object, NULL to ignore.
 * @return TRUE if the file was successully removed, FALSE otherwise.
 * @since 0.0.1
 *
 * This call doesn't resolve symbolic links and remove a symbolic link if the
 * given path points to it.
 */
P_LIB_API pboolean p_file_remove	(const pchar	*file,
					 PError		**error);

P_END_DECLS

#endif /* PLIBSYS_HEADER_PFILE_H */
