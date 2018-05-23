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

#include "perror.h"
#include "pfile.h"
#include "plibraryloader.h"
#include "pmem.h"
#include "pstring.h"

typedef HINSTANCE	plibrary_handle;

struct PLibraryLoader_ {
	plibrary_handle	handle;
};

static void pp_library_loader_clean_handle (plibrary_handle handle);

static void
pp_library_loader_clean_handle (plibrary_handle handle)
{
	if (P_UNLIKELY (!FreeLibrary (handle)))
		P_ERROR ("PLibraryLoader::pp_library_loader_clean_handle: FreeLibrary() failed");
}

P_LIB_API PLibraryLoader *
p_library_loader_new (const pchar *path)
{
	PLibraryLoader	*loader = NULL;
	plibrary_handle	handle;

	if (!p_file_is_exists (path))
		return NULL;

	if (P_UNLIKELY ((handle = LoadLibraryA (path)) == NULL)) {
		P_ERROR ("PLibraryLoader::p_library_loader_new: LoadLibraryA() failed");
		return NULL;
	}

	if (P_UNLIKELY ((loader = p_malloc0 (sizeof (PLibraryLoader))) == NULL)) {
		P_ERROR ("PLibraryLoader::p_library_loader_new: failed to allocate memory");
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

	ret_sym = (PFuncAddr) GetProcAddress (loader->handle, sym);

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
p_library_loader_get_last_error (PLibraryLoader *loader)
{
	pchar	*res = NULL;
	DWORD	err_code;
	LPVOID	msg_buf;

	P_UNUSED (loader);

	err_code = p_error_get_last_system ();

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

	return res;
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
