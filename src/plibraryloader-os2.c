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

#define INCL_DOSMODULEMGR
#define INCL_DOSERRORS
#include <os2.h>

typedef HMODULE plibrary_handle;

struct PLibraryLoader_ {
	plibrary_handle	handle;
	APIRET		last_error;
};

static void pp_library_loader_clean_handle (plibrary_handle handle);

static void
pp_library_loader_clean_handle (plibrary_handle handle)
{
	APIRET ulrc;

	while ((ulrc = DosFreeModule (handle)) == ERROR_INTERRUPT)
		;

	if (P_UNLIKELY (ulrc != NO_ERROR))
		P_ERROR ("PLibraryLoader::pp_library_loader_clean_handle: DosFreeModule() failed");
}

P_LIB_API PLibraryLoader *
p_library_loader_new (const pchar *path)
{
	PLibraryLoader	*loader = NULL;
	plibrary_handle	handle  = NULLHANDLE;
	UCHAR		load_err[256];
	APIRET		ulrc;


	if (!p_file_is_exists (path))
		return NULL;

	while ((ulrc = DosLoadModule ((PSZ) load_err,
				      sizeof (load_err),
				      (PSZ) path,
				      (PHMODULE) &handle)) == ERROR_INTERRUPT)
		;

	if (P_UNLIKELY (ulrc != NO_ERROR)) {
		P_ERROR ("PLibraryLoader::p_library_loader_new: DosLoadModule() failed");
		return NULL;
	}

	if (P_UNLIKELY ((loader = p_malloc0 (sizeof (PLibraryLoader))) == NULL)) {
		P_ERROR ("PLibraryLoader::p_library_loader_new: failed to allocate memory");
		pp_library_loader_clean_handle (handle);
		return NULL;
	}

	loader->handle     = handle;
	loader->last_error = NO_ERROR;

	return loader;
}

P_LIB_API PFuncAddr
p_library_loader_get_symbol (PLibraryLoader *loader, const pchar *sym)
{
	PFN	func_addr = NULL;
	APIRET	ulrc;

	if (P_UNLIKELY (loader == NULL || sym == NULL || loader->handle == NULL))
		return NULL;

	if (P_UNLIKELY ((ulrc = DosQueryProcAddr (loader->handle, 0, (PSZ) sym, &func_addr)) != NO_ERROR)) {
		P_ERROR ("PLibraryLoader::p_library_loader_get_symbol: DosQueryProcAddr() failed");
		loader->last_error = ulrc;
		return NULL;
	}

	loader->last_error = NO_ERROR;

	return (PFuncAddr) func_addr;
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

	switch (loader->last_error) {
		case NO_ERROR:
			return NULL;
		case ERROR_INVALID_HANDLE:
			return p_strdup ("Invalid resource handler");
		case ERROR_INVALID_NAME:
			return p_strdup ("Invalid procedure name");
		default:
			return p_strdup ("Unknown error");
	}
}

P_LIB_API pboolean
p_library_loader_is_ref_counted (void)
{
	return TRUE;
}

void
p_library_loader_init (void)
{
}

void
p_library_loader_shutdown (void)
{
}
