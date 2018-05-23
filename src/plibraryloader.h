/*
 * The MIT License
 *
 * Copyright (C) 2015-2017 Alexander Saprykin <saprykin.spb@gmail.com>
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
 * @file plibraryloader.h
 * @brief Shared library loader
 * @author Alexander Saprykin
 *
 * All modern operating systems support dynamic loadable objects. Such objects
 * are compiled with special flags and can be loaded by other programs and
 * libraries later at the runtime. These loadable objects often called as the
 * shared libraries, though some platforms even allow to treat the program
 * binary as a loadable object, too.
 *
 * When the program is linked with a shared library its dependency would be
 * resolved by the operating system automatically before starting the program.
 * But in some circumstances you may need to load a shared library object
 * explicitly (i.e. implementing a plugin subsystem, checking for API
 * availability).
 *
 * All functions and variables which a shared library is exporting are called
 * symbols. Usually only the exported symbols are available from outside the
 * shared library. Actually all those symbols represent a library API.
 *
 * Use p_library_loader_new() to load a shared library and
 * p_library_loader_get_symbol() to retrieve a pointer to a symbol within it.
 * Close the library after usage with p_library_loader_free().
 *
 * Please note the following platform specific differences:
 *
 * - HP-UX doesn't support loading libraries containing TLS and built with
 * static TLS model. The same rule applies to any library used as dependency.
 * HP-UX on 32-bit PA-RISC systems doesn't support reference counting for loaded
 * libraries when using shl_* family of functions (always removes all library
 * references on unload).
 *
 * - On OpenVMS only shareable images (linked with /SHAREABLE) can be used for
 * dynamic symbol resolving. Usually they have .EXE extension.
 *
 * - BeOS supports dynamic loading for add-ons only. It is also possible to
 * load the same library several times independently (not like a traditional
 * shared library).
 */

#if !defined (PLIBSYS_H_INSIDE) && !defined (PLIBSYS_COMPILATION)
#  error "Header files shouldn't be included directly, consider using <plibsys.h> instead."
#endif

#ifndef PLIBSYS_HEADER_PLIBRARYLOADER_H
#define PLIBSYS_HEADER_PLIBRARYLOADER_H

#include <pmacros.h>
#include <ptypes.h>

P_BEGIN_DECLS

/** Opaque data structure to handle a shared library. */
typedef struct PLibraryLoader_ PLibraryLoader;

/** Pointer to a function address. */
typedef void (*PFuncAddr) (void);

/**
 * @brief Loads a shared library.
 * @param path Path to the shared library file.
 * @return Pointer to #PLibraryLoader in case of success, NULL otherwise.
 * @since 0.0.1
 *
 * If you are loading the already loaded shared library, an operating system
 * increments corresponding reference count and decrements it after freeing
 * #PLibraryLoader, thus the shared library would be unloaded from the address
 * space only when the counter becomes zero.
 */
P_LIB_API PLibraryLoader *	p_library_loader_new		(const pchar	*path);

/**
 * @brief Gets a pointer to a symbol in the loaded shared library.
 * @param loader Pointer to the loaded shared library handle.
 * @param sym Name of the symbol.
 * @return Pointer to the symbol in case of success, NULL otherwise.
 * @since 0.0.1
 *
 * Since the symbol may have a NULL value, the returned NULL value from this
 * call actually doesn't mean the failed result. You can additionally check the
 * error result using p_library_loader_get_last_error().
 */
P_LIB_API PFuncAddr		p_library_loader_get_symbol	(PLibraryLoader	*loader,
								 const pchar	*sym);

/**
 * @brief Frees memory and allocated resources of #PLibraryLoader.
 * @param loader #PLibraryLoader object to free.
 * @since 0.0.1
 */
P_LIB_API void			p_library_loader_free		(PLibraryLoader	*loader);

/**
 * @brief Gets the last occurred error.
 * @param loader #PLibraryLoader object to get error for.
 * @return Human readable error string in case of success, NULL otherwise.
 * @since 0.0.1
 * @version 0.0.3 @p loader parameter was added.
 * @note Caller takes ownership of the returned string.
 *
 * The NULL result may indicate that no error was occurred since the last call.
 *
 * Different operating systems have different behavior on error indicating.
 * Some systems reset an error status before the call, some do not. Some
 * systems write the successful call result (usually zero) to the error status,
 * thus resetting an error from the previous call.
 *
 * Some operating systems may return last error even if library handler was not
 * created. In that case try to pass NULL value as a parameter.
 */
P_LIB_API pchar *		p_library_loader_get_last_error	(PLibraryLoader	*loader);

/**
 * @brief Checks whether library loading subsystem uses reference counting.
 * @return TRUE in case of success, FALSE otherwise.
 * @since 0.0.3
 *
 * When reference counting is supported, the same shared library can be opened
 * several times, but it would be completely unloaded from the memory only when
 * the last reference to it is removed.
 *
 * @note For now, only HP-UX on 32-bit PA-RISC systems with shl_* model doesn't
 * support reference counting.
 */
P_LIB_API pboolean		p_library_loader_is_ref_counted (void);

P_END_DECLS

#endif /* PLIBSYS_HEADER_PLIBRARYLOADER_H */
