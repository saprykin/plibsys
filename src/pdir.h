/*
 * The MIT License
 *
 * Copyright (C) 2015-2016 Alexander Saprykin <saprykin.spb@gmail.com>
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
 * @file pdir.h
 * @brief Filesystem interface
 * @author Alexander Saprykin
 *
 * A traditional filesystem can be presented as a combination of directories and
 * files within a defined hierarchy. A directory contains the so called entries:
 * files and other directories. #PDir allows to iterate through these entries
 * without reading their contents, thus building a filesystem hierarchy tree.
 *
 * Think of this module as an interface to the well-known `dirent` API.
 *
 * First you need to open a directory for iterating through its content entries
 * using p_dir_new(). After that every next entry inside the directory can be
 * read with the p_dir_get_next_entry() call until it returns NULL (though it's
 * better to check an error code to be sure no error occurred).
 *
 * Also some directory manipulation routines are provided to create, remove and
 * check existance.
 */

#if !defined (PLIBSYS_H_INSIDE) && !defined (PLIBSYS_COMPILATION)
#  error "Header files shouldn't be included directly, consider using <plibsys.h> instead."
#endif

#ifndef PLIBSYS_HEADER_PDIR_H
#define PLIBSYS_HEADER_PDIR_H

#include <pmacros.h>
#include <ptypes.h>
#include <perror.h>

P_BEGIN_DECLS

/** Directory opaque data structure. */
typedef struct PDir_ PDir;

/** Directory entry types. */
typedef enum PDirEntryType_ {
	P_DIR_ENTRY_TYPE_DIR	= 1,	/**< Directory.	*/
	P_DIR_ENTRY_TYPE_FILE	= 2,	/**< File.	*/
	P_DIR_ENTRY_TYPE_OTHER	= 3	/**< Other.	*/
} PDirEntryType;

/** Structure with directory entry information. */
typedef struct PDirEntry_ {
	char		*name;	/**< Name.	*/
	PDirEntryType	type;	/**< Type.	*/
} PDirEntry;

/**
 * @brief Creates a new #PDir object.
 * @param path Directory path.
 * @return Pointer to a newly created #PDir object in case of success, NULL
 * otherwise.
 * @param[out] error Error report object, NULL to ignore.
 * @since 0.0.1
 * @note If you want to create a new directory on a filesystem, use
 * p_dir_create() instead.
 */
P_LIB_API PDir *	p_dir_new		(const pchar	*path,
						 PError		**error);

/**
 * @brief Creates a new directory on a filesystem.
 * @param path Directory path.
 * @param mode Directory permissions to use, ignored on Windows.
 * @param[out] error Error report object, NULL to ignore.
 * @return TRUE in case of success, FALSE otherwise.
 * @since 0.0.1
 * @note Call returns TRUE if the directory @a path is already exists.
 * @note On OpenVMS operating system it creates intermediate directories as
 * well.
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
 * Caller takes ownership of the returned string. Use p_free() to free memory
 * after usage.
 */
P_LIB_API pchar *	p_dir_get_path		(const PDir	*dir);

/**
 * @brief Gets the next directory entry info.
 * @param dir Directory to get the next entry from.
 * @param[out] error Error report object, NULL to ignore.
 * @return Info for the next entry in case of success, NULL otherwise.
 * @since 0.0.1
 *
 * Caller takes ownership of the returned object. Use p_dir_entry_free() to free
 * memory of the directory entry after usage.
 *
 * An error is set only if it is occurred. You should check the @a error object
 * for #P_ERROR_IO_NO_MORE code.
 */
P_LIB_API PDirEntry *	p_dir_get_next_entry	(PDir		*dir,
						 PError		**error);

/**
 * @brief Resets a directory entry pointer.
 * @param dir Directory to reset the entry pointer.
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

#endif /* PLIBSYS_HEADER_PDIR_H */
