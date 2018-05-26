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

#include <dlfcn.h>

/* FreeBSD may cause a segfault: https://reviews.freebsd.org/D5112,
 * DragonFlyBSD as well, so we need to check a file size before calling dlopen()
 */
#if defined (P_OS_FREEBSD) || defined (P_OS_DRAGONFLY)
#  include <unistd.h>
#  include <sys/types.h>
#  include <sys/stat.h>
#endif

typedef ppointer plibrary_handle;

struct PLibraryLoader_ {
	plibrary_handle	handle;
};

static void pp_library_loader_clean_handle (plibrary_handle handle);

static void
pp_library_loader_clean_handle (plibrary_handle handle)
{
	if (P_UNLIKELY (dlclose (handle) != 0))
		P_ERROR ("PLibraryLoader::pp_library_loader_clean_handle: dlclose() failed");
}

P_LIB_API PLibraryLoader *
p_library_loader_new (const pchar *path)
{
	PLibraryLoader	*loader = NULL;
	plibrary_handle	handle;
#if defined (P_OS_FREEBSD) || defined (P_OS_DRAGONFLY)
	struct stat	stat_buf;
#endif

	if (!p_file_is_exists (path))
		return NULL;

#if defined (P_OS_FREEBSD) || defined (P_OS_DRAGONFLY)
	if (P_UNLIKELY (stat (path, &stat_buf) != 0)) {
		P_ERROR ("PLibraryLoader::p_library_loader_new: stat() failed");
		return NULL;
	}

	if (P_UNLIKELY (stat_buf.st_size == 0)) {
		P_ERROR ("PLibraryLoader::p_library_loader_new: unable to handle zero-size file");
		return NULL;
	}
#endif

	if (P_UNLIKELY ((handle = dlopen (path, RTLD_NOW)) == NULL)) {
		P_ERROR ("PLibraryLoader::p_library_loader_new: dlopen() failed");
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
	if (P_UNLIKELY (loader == NULL || sym == NULL || loader->handle == NULL))
		return NULL;

	return (PFuncAddr) dlsym (loader->handle, sym);
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
	pchar *res = NULL;
	pchar *msg;

	P_UNUSED (loader);

	msg = dlerror ();

	if (msg != NULL)
		res = p_strdup (msg);

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
