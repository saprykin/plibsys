/*
 * Copyright (C) 2010-2016 Alexander Saprykin <xelfium@gmail.com>
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, see <http://www.gnu.org/licenses/>.
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
#ifdef P_OS_WIN
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
