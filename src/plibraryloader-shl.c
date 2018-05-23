/*
 * The MIT License
 *
 * Copyright (C) 2017 Alexander Saprykin <saprykin.spb@gmail.com>
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

#include "perror.h"
#include "pfile.h"
#include "plibraryloader.h"
#include "pmem.h"
#include "pstring.h"

#include <dl.h>
#include <errno.h>
#include <string.h>

typedef shl_t plibrary_handle;

struct PLibraryLoader_ {
	plibrary_handle	handle;
	int		last_error;
};

static void pp_library_loader_clean_handle (plibrary_handle handle);

static void
pp_library_loader_clean_handle (plibrary_handle handle)
{
	if (P_UNLIKELY (shl_unload (handle) != 0))
		P_ERROR ("PLibraryLoader::pp_library_loader_clean_handle: shl_unload() failed");
}

P_LIB_API PLibraryLoader *
p_library_loader_new (const pchar *path)
{
	PLibraryLoader	*loader = NULL;
	plibrary_handle	handle;

	if (!p_file_is_exists (path))
		return NULL;

	if (P_UNLIKELY ((handle = shl_load (path, BIND_IMMEDIATE | BIND_NONFATAL | DYNAMIC_PATH, 0)) == NULL)) {
		P_ERROR ("PLibraryLoader::p_library_loader_new: shl_load() failed");
		return NULL;
	}

	if (P_UNLIKELY ((loader = p_malloc0 (sizeof (PLibraryLoader))) == NULL)) {
		P_ERROR ("PLibraryLoader::p_library_loader_new: failed to allocate memory");
		pp_library_loader_clean_handle (handle);
		return NULL;
	}

	loader->handle     = handle;
	loader->last_error = 0;

	return loader;
}

P_LIB_API PFuncAddr
p_library_loader_get_symbol (PLibraryLoader *loader, const pchar *sym)
{
	PFuncAddr func_addr = NULL;

	if (P_UNLIKELY (loader == NULL || sym == NULL || loader->handle == NULL))
		return NULL;

	if (P_UNLIKELY (shl_findsym (&loader->handle, sym, TYPE_UNDEFINED, (ppointer) &func_addr) != 0)) {
		P_ERROR ("PLibraryLoader::p_library_loader_get_symbol: shl_findsym() failed");
		loader->last_error = (errno == 0 ? -1 : errno);
		return NULL;
	}

	loader->last_error = 0;

	return func_addr;
}

P_LIB_API void
p_library_loader_free (PLibraryLoader *loader)
{
	if (P_UNLIKELY (loader == NULL))
		return;

	pp_library_loader_clean_handle (loader->handle);

	p_free (loader);
}

P_LIB_API pchar *
p_library_loader_get_last_error (PLibraryLoader *loader)
{
	if (loader == NULL)
		return NULL;

	if (loader->last_error == 0)
		return NULL;
	else if (loader->last_error == -1)
		return p_strdup ("Failed to find a symbol");
	else
		return p_strdup (strerror (loader->last_error));
}

P_LIB_API pboolean
p_library_loader_is_ref_counted (void)
{
#if defined (P_OS_HPUX) && defined (P_CPU_HPPA) && (PLIBSYS_SIZEOF_VOID_P == 4)
	return FALSE;
#else
	return TRUE;
#endif
}

void
p_library_loader_init (void)
{
}

void
p_library_loader_shutdown (void)
{
}
