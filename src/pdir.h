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
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

/**
 * @file pdir.h
 * @brief Filesystem interface
 * @author Alexander Saprykin
 *
 * Traditional filesystem can be presented as a combination of directories and
 * files within a defined hierarchy. Directory contains the so called entries:
 * files and other directories. #PDir allows to iterate through these entries
 * without reading their contents, thus building a filesystem hierarchy tree.
 *
 * Think of this module as an interface to the well-known `dirent` API.
 *
 * First you need to open a directory for iterating through its content entries
 * using p_dir_new(). After that every next entry inside the directory can be
 * read with p_dir_get_next_entry() call until it returns NULL (though it's
 * better to check error code to be sure no error occurred).
 *
 * Also some directory manipulation routines are provided to create, remove and
 * check existance.
 */

#if !defined (__PLIBSYS_H_INSIDE__) && !defined (PLIBSYS_COMPILATION)
#  error "Header files shouldn't be included directly, consider using <plibsys.h> instead."
#endif

#ifndef __PDIR_H__
#define __PDIR_H__

#include <pmacros.h>
#include <ptypes.h>
#include <perror.h>

P_BEGIN_DECLS

/** Directory opaque data structure. */
typedef struct _PDir PDir;

/** Directory entry types. */
typedef enum _PDirEntryType {
	P_DIR_ENTRY_TYPE_DIR	= 1,	/**< Directory.	*/
	P_DIR_ENTRY_TYPE_FILE	= 2,	/**< File.	*/
	P_DIR_ENTRY_TYPE_OTHER	= 3	/**< Other.	*/
} PDirEntryType;

/** Structure with a directory entry information. */
typedef struct _PDirEntry {
	char		*name;	/**< Name.	*/
	PDirEntryType	type;	/**< Type.	*/
} PDirEntry;

/**
 * @brief Creates a new #PDir object.
 * @param path Directory path.
 * @return Pointer to a newly created #PDir object in case of success,
 * NULL otherwise.
 * @param[out] error Error report object, NULL to ignore.
 * @since 0.0.1
 * @note If you want to create a new directory on filesystem, use
 * p_dir_create() instead.
 */
P_LIB_API PDir *	p_dir_new		(const pchar	*path,
						 PError		**error);

/**
 * @brief Creates a new directory on filesystem.
 * @param path Directory path.
 * @param mode Directory permissions to use, ignored on Windows.
 * @param[out] error Error report object, NULL to ignore.
 * @return TRUE in case of success, FALSE otherwise.
 * @since 0.0.1
 * @note Call returns TRUE if the directory @a path is already exists.
 */
P_LIB_API pboolean	p_dir_create		(const pchar	*path,
						 pint		mode,
						 PError		**error);

/**
 * @brief Removes an empty directory.
 * @param path Directory path to remove.
 * @param[out] error Error report object, NULL to ignore.
 * @return TRUE in case of success, FALSE otherwise.
 * @since 0.0.1
 *
 * The directory @a path should be empty to be removed successfully.
 */
P_LIB_API pboolean	p_dir_remove		(const pchar	*path,
						 PError		**error);

/**
 * @brief Checks whether a directory exists or not.
 * @param path Directory path.
 * @return TRUE in case of success, FALSE otherwise.
 * @since 0.0.1
 */
P_LIB_API pboolean	p_dir_is_exists		(const pchar	*path);

/**
 * @brief Gets the original directory path used to create a #PDir object.
 * @param dir #PDir object to retrieve the path from.
 * @return The directory path in case of success, NULL otherwise.
 * @since 0.0.1
 *
 * Caller takes ownership of the returned string. Use p_free()
 * to free memory after usage.
 */
P_LIB_API pchar *	p_dir_get_path		(const PDir	*dir);

/**
 * @brief Gets the next directory entry info.
 * @param dir Directory to get the next entry from.
 * @param[out] error Error report object, NULL to ignore.
 * @return Info for the next entry in case of success, NULL otherwise.
 * @since 0.0.1
 *
 * Caller takes ownership of the returned object. Use p_dir_entry_free()
 * to free memory of a directory entry after usage.
 *
 * Error is set only if it is occurred. You should check @a error object
 * for #P_ERROR_IO_NO_MORE code.
 */
P_LIB_API PDirEntry *	p_dir_get_next_entry	(PDir		*dir,
						 PError		**error);

/**
 * @brief Resets the directory entry pointer.
 * @param dir Directory to reset entry pointer.
 * @param[out] error Error report object, NULL to ignore.
 * @return TRUE in case of success, FALSE otherwise.
 * @since 0.0.1
 */
P_LIB_API pboolean	p_dir_rewind		(PDir		*dir,
						 PError		**error);

/**
 * @brief Frees #PDirEntry object.
 * @param entry #PDirEntry to free.
 * @since 0.0.1
 */
P_LIB_API void		p_dir_entry_free	(PDirEntry	*entry);

/**
 * @brief Frees #PDir object.
 * @param dir #PDir to free.
 * @since 0.0.1
 */
P_LIB_API void		p_dir_free		(PDir		*dir);

P_END_DECLS

#endif /* __PDIR_H__ */
