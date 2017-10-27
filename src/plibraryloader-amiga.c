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

#include <proto/dos.h>
#include <proto/elf.h>

typedef APTR plibrary_handle;

struct PLibraryLoader_ {
	plibrary_handle	handle;
	Elf32_Error	last_error;
};

static Elf32_Handle pp_library_loader_elf_root = NULL;

static void pp_library_loader_clean_handle (plibrary_handle handle);

static void
pp_library_loader_clean_handle (plibrary_handle handle)
{
	IElf->DLClose (pp_library_loader_elf_root, handle);
}

P_LIB_API PLibraryLoader *
p_library_loader_new (const pchar *path)
{
	PLibraryLoader	*loader = NULL;
	plibrary_handle	handle  = NULL;

	if (!p_file_is_exists (path))
		return NULL;

	if (pp_library_loader_elf_root == NULL) {
		P_ERROR ("PLibraryLoader::p_library_loader_new: shared library subsystem is not initialized");
		return NULL;
	}

	handle = IElf->DLOpen (pp_library_loader_elf_root, (CONST_STRPTR) path, ELF32_RTLD_LOCAL);

	if (P_UNLIKELY (handle == NULL)) {
		P_ERROR ("PLibraryLoader::p_library_loader_new: DLOpen() failed");
		return NULL;
	}

	if (P_UNLIKELY ((loader = p_malloc0 (sizeof (PLibraryLoader))) == NULL)) {
		P_ERROR ("PLibraryLoader::p_library_loader_new: failed to allocate memory");
		pp_library_loader_clean_handle (handle);
		return NULL;
	}

	loader->handle     = handle;
	loader->last_error = ELF32_NO_ERROR;

	return loader;
}

P_LIB_API PFuncAddr
p_library_loader_get_symbol (PLibraryLoader *loader, const pchar *sym)
{
	APTR func_addr = NULL;

	if (P_UNLIKELY (loader == NULL || sym == NULL || loader->handle == NULL))
		return NULL;

	if (pp_library_loader_elf_root == NULL) {
		P_ERROR ("PLibraryLoader::p_library_loader_new: shared library subsystem is not initialized");
		return NULL;
	}

	loader->last_error = IElf->DLSym (pp_library_loader_elf_root,
					  loader->handle,
					  (CONST_STRPTR) sym,
					  &func_addr);

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
	if (P_UNLIKELY (loader == NULL))
		return NULL;

	switch (loader->last_error) {
		case ELF32_NO_ERROR:
			return NULL;
		case ELF32_OUT_OF_MEMORY:
			return p_strdup ("Out of memory");
		case ELF32_INVALID_HANDLE:
			return p_strdup ("Invalid resource handler");
		case ELF32_NO_MORE_RELOCS:
			return p_strdup ("No more relocations left");
		case ELF32_SECTION_NOT_LOADED:
			return p_strdup ("Section not loaded");
		case ELF32_UNKNOWN_RELOC:
			return p_strdup ("Unknown relocation");
		case ELF32_READ_ERROR:
			return p_strdup ("Read error");
		case ELF32_INVALID_SDA_BASE:
			return p_strdup ("Invalid SDA base");
		case ELF32_SYMBOL_NOT_FOUND:
			return p_strdup ("Symbol not found");
		case ELF32_INVALID_NAME:
			return p_strdup ("Invalid procedure name");
		case ELF32_REQUIRED_OBJECT_MISSING:
			return p_strdup ("Required object is missing");
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
	BPTR		segment_list;
	Elf32_Handle	elf_handle;

	if (pp_library_loader_elf_root != NULL)
		return;

	segment_list = IDOS->GetProcSegList (NULL, GPSLF_RUN);

	if (P_UNLIKELY (segment_list == 0)) {
		P_ERROR ("PLibraryLoader::p_library_loader_init: GetProcSegList() failed");
		return;
	}

	if (P_UNLIKELY (IDOS->GetSegListInfoTags (segment_list,
						  GSLI_ElfHandle,
						  &elf_handle,
						  TAG_DONE) != 1)) {
		P_ERROR ("PLibraryLoader::p_library_loader_init: GetSegListInfoTags() failed");
		return;
	}

	if (P_UNLIKELY (elf_handle == NULL)) {
		P_ERROR ("PLibraryLoader::p_library_loader_init: failed to finf proper GSLI_ElfHandle");
		return;
	}

	pp_library_loader_elf_root = IElf->OpenElfTags (OET_ElfHandle, elf_handle, TAG_DONE);

	if (P_UNLIKELY (pp_library_loader_elf_root == NULL)) {
		P_ERROR ("PLibraryLoader::p_library_loader_init: OpenElfTags() failed");
		return;
	}
}

void
p_library_loader_shutdown (void)
{
	if (pp_library_loader_elf_root == NULL)
		return;

	IElf->CloseElfTags (pp_library_loader_elf_root, CET_ReClose, TRUE, TAG_DONE);

	pp_library_loader_elf_root = NULL;
}
