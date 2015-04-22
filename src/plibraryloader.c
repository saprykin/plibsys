/*
 * Copyright (C) 2015 Alexander Saprykin <xelfium@gmail.com>
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

#include "plibraryloader.h"
#include "pfile.h"
#include "pmem.h"
#include "pstring.h"

#ifdef P_OS_WIN
#include <winsock2.h>
#include <windows.h>
#else
#include <dlfcn.h>
#endif

struct _PLibraryLoader {
#ifdef P_OS_WIN
	HINSTANCE	handle;
#else
	ppointer	handle;
#endif
};

P_LIB_API PLibraryLoader *
p_library_loader_new (const pchar *path)
{
	PLibraryLoader *	loader;
#ifdef P_OS_WIN
	HINSTANCE 		handle;
#else
	ppointer		handle;
#endif

	loader = NULL;

	if (!p_file_is_exists (path))
		return NULL;

#ifdef P_OS_WIN
	if ((handle = LoadLibraryA (path)) == NULL) {
		P_ERROR ("PLibraryLoader: failed to call LoadLibraryA()");
		return NULL;
	}
#else
	if ((handle = dlopen (path, RTLD_NOW)) == NULL) {
		P_ERROR ("PLibraryLoader: failed to call dlopen()");
		return NULL;
	}
#endif

	if ((loader = p_malloc0 (sizeof (PLibraryLoader))) == NULL) {
		P_ERROR ("PLibraryLoader: failed to allocate memory");
		return NULL;
	}

	loader->handle = handle;

	return loader;
}

P_LIB_API ppointer
p_library_loader_get_symbol (PLibraryLoader *loader, const pchar *sym)
{
	ppointer ret_sym = NULL;

	if (loader == NULL || sym == NULL || loader->handle == NULL)
		return NULL;

#ifdef P_OS_WIN
	ret_sym = GetProcAddress (loader->handle, sym);
#else
	ret_sym = dlsym (loader->handle, sym);
#endif

	return ret_sym;
}

P_LIB_API void
p_library_loader_free (PLibraryLoader *loader)
{
	if (loader == NULL)
		return;

	if (loader->handle != NULL) {
#ifdef P_OS_WIN
		if (!FreeLibrary (loader->handle))
			P_ERROR ("PLibraryLoader: failed to call FreeLibrary()");
#else
		if (dlclose (loader->handle) != 0)
			P_ERROR ("PLibraryLoader: failed to call dlclose()");
#endif
	}

	p_free (loader);
}

P_LIB_API pchar *
p_library_loader_get_last_error (void)
{
	pchar *res = NULL;

#ifdef P_OS_WIN
	DWORD	err_code;
	LPVOID	msg_buf;

	err_code = GetLastError ();

	if (err_code == 0)
		return NULL;

	if (FormatMessageA (FORMAT_MESSAGE_ALLOCATE_BUFFER |
			    FORMAT_MESSAGE_FROM_SYSTEM |
			    FORMAT_MESSAGE_IGNORE_INSERTS,
			    NULL,
			    err_code,
			    MAKELANGID (LANG_NEUTRAL, SUBLANG_DEFAULT),
			    (LPTSTR) &msg_buf,
			    0,
			    NULL) != 0) {
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
