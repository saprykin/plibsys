/*
 * Copyright (C) 2016-2017 Alexander Saprykin <xelfium@gmail.com>
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
