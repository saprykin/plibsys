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
 * @file plibraryloader.h
 * @brief Shared library loader
 * @author Alexander Saprykin
 *
 * #PLibraryLoader gives ability to load shared libraries and executables as
 * third party objects. After loading you can use its symbols (functions and
 * variables) as locally defined.
 * Use p_library_loader_new() to load library and p_library_loader_get_symbol()
 * to retrieve the pointer to a symbol within it. Close library after usage with
 * p_library_loader_free().
 */

#if !defined (__PLIBSYS_H_INSIDE__) && !defined (PLIBSYS_COMPILATION)
#  error "Header files shouldn't be included directly, consider using <plibsys.h> instead."
#endif

#ifndef __PLIBRARYLOADER_H__
#define __PLIBRARYLOADER_H__

#include <pmacros.h>
#include <ptypes.h>

P_BEGIN_DECLS

/** PLibraryLoader opaque data struct */
typedef struct _PLibraryLoader PLibraryLoader;

/** Pointer to function address */
typedef void (*PFuncAddr) (void);

/**
 * @brief Loads dynamic library.
 * @param path Path to the library file.
 * @return Pointer to #PLibraryLoader in case of success, NULL otherwise.
 * @since 0.0.1
 *
 * If you are loading already loaded library, operating system increments
 * corresponding reference count and decrements it when freeing #PLibraryLoader,
 * so operating system unloads library from address space only when counter
 * becomes zero.
 */
P_LIB_API PLibraryLoader *	p_library_loader_new		(const pchar *path);

/**
 * @brief Gets pointer to a symbol in loaded library.
 * @param loader Pointer to loaded library handle.
 * @param sym Name of the symbol.
 * @return Pointer to the symbol in case of success, NULL otherwise.
 * @since 0.0.1
 *
 * Since symbol may have a NULL value, NULL returned value from this call
 * actually doesn't mean the failed result. You can additionally check error
 * result using p_library_loader_get_last_error().
 */
P_LIB_API PFuncAddr		p_library_loader_get_symbol	(PLibraryLoader *loader, const pchar *sym);

/**
 * @brief Frees memory and allocated resources of #PLibraryLoader.
 * @param loader #PLibraryLoader object to free.
 * @since 0.0.1
 */
P_LIB_API void			p_library_loader_free		(PLibraryLoader *loader);

/**
 * @brief Gets last occurred error.
 * @return Human readable error string in case of success, NULL otherwise.
 * Caller takes ownership for the returned string.
 * @since 0.0.1
 *
 * A NULL result may indicate that no error was occurred since last call.
 *
 * Different operating systems have different behaviour on error indicating.
 * Some systems reset error status before the call, some are not. Some
 * systems write successful call result (usually zero) to error status,
 * thus resetting the error from the previous call.
 */
P_LIB_API pchar *		p_library_loader_get_last_error	(void);

P_END_DECLS

#endif /* __PLIBRARYLOADER_H__ */
