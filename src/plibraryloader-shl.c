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
p_library_loader_is_ref_counted ()
{
#if defined (P_OS_HPUX) && defined (P_CPU_HPPA) && (PLIBSYS_SIZEOF_VOID_P == 4)
	return FALSE;
#else
	return TRUE;
#endif
}
