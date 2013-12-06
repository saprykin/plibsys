/* 
 * Copyright (C) 2010-2013 Alexander Saprykin <xelfium@gmail.com>
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

/**
 * @file pfile.h
 * @brief File related functions
 * @author Alexander Saprykin
 */

#if !defined (__PLIB_H_INSIDE__) && !defined (PLIB_COMPILATION)
#  error "Header files shouldn't be included directly, consider using <plib.h> instead."
#endif

#ifndef __PFILE_H__
#define __PFILE_H__

#include <pmacros.h>
#include <ptypes.h>

/**
 * @def P_DIR_SEPARATOR
 * @brief Defines directory separator.
 */
#ifdef P_OS_WIN
#  define P_DIR_SEPARATOR "\\"
#else
#  define P_DIR_SEPARATOR "/"
#endif

P_BEGIN_DECLS

/**
 * @brief Checks whether the file is exists or not.
 * @param file File name to check.
 * @return TRUE if file exists, FALSE otherwise.
 * @since 0.0.1
 *
 *  On Windows this call doesn't resolve symbolic links, while
 *  on UNIX systems does.
 */
P_LIB_API pboolean p_file_is_exists	(const pchar *file);

/**
 * @brief Removes file from the disk.
 * @param file File name to remove.
 * @return TRUE if file was successully removed, FALSE otherwise.
 * @since 0.0.1
 *
 * This call doesn't resolve symbolic links and remove symbolic link
 * if given path points to it.
 */
P_LIB_API pboolean p_file_remove	(const pchar *file);

P_END_DECLS

#endif /* __PFILE_H__ */
