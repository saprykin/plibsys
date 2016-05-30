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

#include "plibraryloader.h"
#include "pfile.h"
#include "pmem.h"
#include "pstring.h"
#include "perror-private.h"

#ifndef P_OS_WIN
#  include <dlfcn.h>
#endif

#ifdef P_OS_WIN
	typedef HINSTANCE	plibrary_handle;
#else
	typedef ppointer	plibrary_handle;
#endif

struct PLibraryLoader_ {
	plibrary_handle	handle;
};

static void pp_library_loader_clean_handle (plibrary_handle handle);

static void
pp_library_loader_clean_handle (plibrary_handle handle)
{
#ifdef P_OS_WIN
	if (P_UNLIKELY (!FreeLibrary (handle)))
		P_ERROR ("PLibraryLoader: failed to call FreeLibrary()");
#else
	if (P_UNLIKELY (dlclose (handle) != 0))
		P_ERROR ("PLibraryLoader: failed to call dlclose()");
#endif
}

P_LIB_API PLibraryLoader *
p_library_loader_new (const pchar *path)
{
	PLibraryLoader *	loader;
	plibrary_handle		handle;

	loader = NULL;

	if (!p_file_is_exists (path))
		return NULL;

#ifdef P_OS_WIN
	if (P_UNLIKELY ((handle = LoadLibraryA (path)) == NULL)) {
		P_ERROR ("PLibraryLoader: failed to call LoadLibraryA()");
		return NULL;
	}
#else
	if (P_UNLIKELY ((handle = dlopen (path, RTLD_NOW)) == NULL)) {
		P_ERROR ("PLibraryLoader: failed to call dlopen()");
		return NULL;
	}
#endif

	if (P_UNLIKELY ((loader = p_malloc0 (sizeof (PLibraryLoader))) == NULL)) {
		P_ERROR ("PLibraryLoader: failed to allocate memory");
		pp_library_loader_clean_handle (handle);
		return NULL;
	}

	loader->handle = handle;

	return loader;
}

P_LIB_API PFuncAddr
p_library_loader_get_symbol (PLibraryLoader *loader, const pchar *sym)
{
	PFuncAddr ret_sym = NULL;

	if (P_UNLIKELY (loader == NULL || sym == NULL || loader->handle == NULL))
		return NULL;

#ifdef P_OS_WIN
	ret_sym = (PFuncAddr) GetProcAddress (loader->handle, sym);
#else
	ret_sym = (PFuncAddr) dlsym (loader->handle, sym);
#endif

	return ret_sym;
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
p_library_loader_get_last_error (void)
{
	pchar *res = NULL;

#ifdef P_OS_WIN
	DWORD	err_code;
	LPVOID	msg_buf;

	err_code = p_error_get_last_error ();

	if (err_code == 0)
		return NULL;

	if (P_LIKELY (FormatMessageA (FORMAT_MESSAGE_ALLOCATE_BUFFER |
				      FORMAT_MESSAGE_FROM_SYSTEM |
				      FORMAT_MESSAGE_IGNORE_INSERTS,
				      NULL,
				      err_code,
				      MAKELANGID (LANG_NEUTRAL, SUBLANG_DEFAULT),
				      (LPSTR) &msg_buf,
				      0,
				      NULL) != 0)) {
		res = p_strdup ((pchar *) msg_buf);
		LocalFree (msg_buf);
	}
#else
	pchar *	msg;

	msg = dlerror ();

	if (msg != NULL)
		res = p_strdup (msg);
#endif

	return res;
}
