/*
 * Copyright (C) 2017 Alexander Saprykin <xelfium@gmail.com>
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
p_library_loader_is_ref_counted ()
{
	return TRUE;
}
