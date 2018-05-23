/*
 * The MIT License
 *
 * Copyright (C) 2016-2017 Alexander Saprykin <saprykin.spb@gmail.com>
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

#include "plibraryloader.h"

P_LIB_API PLibraryLoader *
p_library_loader_new (const pchar *path)
{
	P_ERROR ("PLibraryLoader::p_library_loader_new: not implemented");
	return NULL;
}

P_LIB_API PFuncAddr
p_library_loader_get_symbol (PLibraryLoader *loader, const pchar *sym)
{
	P_UNUSED (loader);
	P_UNUSED (sym);

	P_ERROR ("PLibraryLoader::p_library_loader_get_symbol: not implemented");
	return NULL;
}

P_LIB_API void
p_library_loader_free (PLibraryLoader *loader)
{
	P_UNUSED (loader);
	P_ERROR ("PLibraryLoader::p_library_loader_free: not implemented");
}

P_LIB_API pchar *
p_library_loader_get_last_error (PLibraryLoader *loader)
{
	P_UNUSED (loader);

	P_ERROR ("PLibraryLoader::p_library_loader_get_last_error: not implemented");
	return NULL;
}

P_LIB_API pboolean
p_library_loader_is_ref_counted (void)
{
	return FALSE;
}

void
p_library_loader_init (void)
{
}

void
p_library_loader_shutdown (void)
{
}
