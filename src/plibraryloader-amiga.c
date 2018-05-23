/*
 * The MIT License
 *
 * Copyright (C) 2017-2018 Alexander Saprykin <saprykin.spb@gmail.com>
 * Copyright (C) 2002-2015 by Olaf Barthel <obarthel (at) gmx.net>
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
 *
 * Path conversion code was taken and adopted from clib2 project:
 * https://github.com/adtools/clib2
 *
 * This part of code is distributed under the BSD-3-Clause license:
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *   - Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *
 *   - Neither the name of Olaf Barthel nor the names of contributors
 *     may be used to endorse or promote products derived from this
 *     software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include "perror.h"
#include "pfile.h"
#include "plibraryloader.h"
#include "pmem.h"
#include "pstring.h"

#include <string.h>

#include <proto/dos.h>
#include <proto/elf.h>

#if defined (__CLIB2__)
#  include <dos.h>
#elif defined (__NEWLIB__)
#  include <amiga_platform.h>
#endif

typedef APTR plibrary_handle;

struct PLibraryLoader_ {
	plibrary_handle	handle;
	Elf32_Error	last_error;
};

static Elf32_Handle pp_library_loader_elf_root = NULL;

static void pp_library_loader_clean_handle (plibrary_handle handle);
static pint pp_library_loader_translate_path (const pchar *in, pchar *out, psize out_len);

/*
 * The following patterns must translate properly:
 *
 *		foo/
 *		///
 *		foo//bar
 *		foo//bar//baz
 *		./foo
 *		././foo
 *		foo/./baz
 *		foo/./bar/./baz
 *		foo/./././bar
 *		foo/.
 *		/.
 *		/tmp
 *		/tmp/foo
 *		/dev/null
 *		/dev/null/foo
 *		/dev/nullX
 *		/foo
 *		/foo/
 *		/foo/bar
 *		/foo/bar/baz
 *		foo/../bar
 *		foo/bar/../../baz
 *		foo/bar/..
 *		../foo
 *		../../foo
 *		.
 *		..
 */

static pint
pp_library_loader_translate_path (const pchar *in, pchar *out, psize out_len)
{
	pchar volume_name[MAXPATHLEN];
	psize len, volume_name_len;
	pint i, j;

	len = strlen (in);

	if (out_len < MAXPATHLEN || len > MAXPATHLEN)
		return -1;

	strcpy (out, in);

	/* Just copy is path is already an Amiga one */

	if (strchr (in, ':') != NULL)
		return 0;

	in = out;

	/* Strip neighbouring slashes: ('foo//bar' -> 'foo/bar').
	 * The "//" pattern in a Unix file name is apparently harmless,
	 * but on AmigaDOS it has a very definite meaning. */

	if (len > 2) {
		pboolean have_double_slash = FALSE;

		for (i = 0; i < len - 1; ++i) {
			if (in[i] == '/' && in[i + 1] == '/') {
				have_double_slash = TRUE;
				break;
			}
		}

		if (have_double_slash) {
			pboolean have_slash;
			pchar c;

			have_slash = FALSE;

			for (i = j = 0; i < len; ++i) {
				c = in[i];

				if (c == '/') {
					if (!have_slash)
						out[j++] = c;

					have_slash = TRUE;
				} else {
					out[j++] = c;
					have_slash = FALSE;
				}
			}

			len = j;
			out[len] = '\0';
		}
	}

	/* Strip trailing slashes ('foo/' -> 'foo'). A leading '/' must
	 * be preserved, though ('///' -> '/'). */

	if(len > 1) {
		psize num_trailing_slashes = 0;

		while ((num_trailing_slashes < len - 1) && (in[len - (num_trailing_slashes + 1)] == '/'))
			num_trailing_slashes++;

		if (num_trailing_slashes > 0) {
			len -= num_trailing_slashes;
			out[len] = '\0';
		}
	}

	/* Ditch all leading './' ('./foo' -> 'foo'). */

	while (len > 2 && out[0] == '.' && out[1] == '/') {
		len -= 2;
		memmove (out, &out[2], len);
		out[len] = '\0';
	}

	/* Ditch all embedded '/./' ('foo/./bar' -> 'foo/bar', 'foo/././bar' -> 'foo/bar'). */

	if (len > 2) {
		pboolean have_slash_dot_slash = FALSE;

		for (i = j = 0; i < len - 2; ++i) {
			if(in[i] == '/' && in[i + 1] == '.' && in[i + 2] == '/') {
				have_slash_dot_slash = TRUE;
				break;
			}
		}

		if (have_slash_dot_slash) {
			for (i = j = 0; i < len; ++i) {
				while (i < len - 2 && in[i] == '/' && in[i + 1] == '.' && in[i + 2] == '/')
					i += 2;

				if (i < len)
					out[j++] = in[i];
			}
	
			len = j;
			out[len] = '\0';
		}
	}

	/* Special case: the path name may end with "/." signifying that the
	 * directory itself is requested ('foo/.' -> 'foo'). */

	if (len >= 2 && strncmp (&in[len - 2], "/.", 2) == 0) {
		/* If it's just '/.' then it's really '/'. */
		if (len == 2) {
			strcpy (out, "/");
			len = 1;
		} else {
			len -= 2;
			out[len] = '\0';
		}
	}

	/* Check for absolute path. */

	if (in[0] == '/') {
		/* OK, so this is an absolute path. We begin by checking
		 * for a few special cases, the first being a reference
		 * to "/tmp". */
		if ((strncmp (in, "/tmp", 4) == 0) && (in[4] == '/' || len == 4)) {
			if (in[4] == '/') {
				/* Convert "/tmp/foo" to "T:foo". */
				memmove (&out[2], &in[5], len - 5);
				memmove (out, "T:", 2);

				len -= 3;
			} else {
				/* Convert "/tmp" to "T:". */
				strcpy (out, "T:");

				len = 2;
			}

			out[len] = '\0';
		} else if ((strncmp (in, "/dev/null", 9)) == 0 && (len == 9 || in[9] == '/')) {
			strcpy (out, "NIL:");
			len = 4;
		} else {
			psize path_name_start = 0;
			volume_name_len       = 0;

			/* Find out how long the first component of the absolute path is. */
			for (i = 1; i <= len; ++i) {
				if (i == len || in[i] == '/') {
					volume_name_len = i - 1;

					/* Is there anything following the path name? */
					if (i < len)
						path_name_start = i + 1;

					break;
				}
			}

			/* Copy the first component and attach a colon. "/foo" becomes "foo:". */
			memmove (out, &in[1], volume_name_len);
			out[volume_name_len++] = ':';

			/* Now add the finishing touches. "/foo/bar" finally
			 * becomes "foo:bar" and "/foo" becomes "foo:". */
			if (path_name_start > 0) {
				memmove (&out[volume_name_len], &in[path_name_start], len - path_name_start);

				len--;
			}

			out[len] = '\0';
		}
	}

	/* Extract and remove the volume name from the path. We
	 * are going to need it later. */

	volume_name_len = 0;

	for (i = 0; i < len; ++i) {
		if (in[i] == ':') {
			/* Look for extra colon characters embedded in the name
		         * (as in "foo/bar:baz") which really don't belong here. */
			for (j = 0 ; j < i ; j++) {
				if(in[j] == '/')
					return -1;
			}

			volume_name_len = i + 1;
			len -= volume_name_len;

			memmove (volume_name, in, volume_name_len);
			memmove (out, &out[volume_name_len], len);
			
			out[len] = '\0';

			break;
		}
	}

	/* Look for extra colon characters embedded in the name
         * (as in "foo:bar:baz") which really don't belong here. */

	for (i = 0; i < len; ++i) {
		if (in[i] == ':')
			return -1;
	}	

	/* Now parse the path name and replace all embedded '..' with
	 * the AmigaDOS counterparts ('foo/../bar' -> 'foo//bar'). */

	if (len > 3) {
		pboolean have_slash_dot_dot_slash = FALSE;

		for (i = j = 0; i < len - 3; ++i) {
			if (in[i] == '/' && in[i + 1] == '.' && in[i + 2] == '.' && in[i + 3] == '/') {
				have_slash_dot_dot_slash = TRUE;
				break;
			}
		}

		if (have_slash_dot_dot_slash) {
			pboolean have_before = FALSE;

			for (i = j = 0; i < len; ++i) {
				if(i < len - 3 && in[i] == '/' && in[i + 1] == '.' && in[i + 2] == '.' && in[i + 3] == '/') {
					out[j++] = in[i];

					if (have_before)
						out[j++] = '/';

					i += 2;

					have_before = TRUE;
				} else {
					have_before = FALSE;
					out[j++] = in[i];
				}
			}

			len = j;
			out[len] = '\0';
		}
	}

	/* Translate a trailing '/..' to '//' */

	if (len >= 3 && strncmp (&in[len - 3], "/..", 3) == 0) {
		len -= 2;
		out[len++] = '/';
		out[len] = '\0';
	}

	/* Translate a leading '../' ('../foo' -> '/foo') */

	if (len >= 3 && strncmp (in, "../", 3) == 0) {
		memmove (out, &in[2], len - 2);
		
		len -= 2;
		out[len] = '\0';
	}

	/* Translate the '..' ('..' -> '/') */

	if (len == 2 && strncmp (in, "..", 2) == 0) {
		strcpy (out, "/");

		len = 1;
	}

	/* Translate the '.' ('.' -> '') */

	if (len == 1 && in[0] == '.') {
		strcpy (out, "");

		len = 0;
	}

	/* Now put it all together again. */

	if(volume_name_len > 0) {
		memmove (&out[volume_name_len], in, len);
		memmove (out, volume_name, volume_name_len);

		len += volume_name_len;
		out[len] = '\0';
	}

	return 0;
}

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
	pchar		path_buffer[MAXPATHLEN];
	
	if (!p_file_is_exists (path))
		return NULL;

	if (pp_library_loader_elf_root == NULL) {
		P_ERROR ("PLibraryLoader::p_library_loader_new: shared library subsystem is not initialized");
		return NULL;
	}

	if (strlen (path) >= MAXPATHLEN) {
		P_ERROR ("PLibraryLoader::p_library_loader_new: too long file path or name");
		return NULL;
	}

	if (pp_library_loader_translate_path (path, path_buffer, MAXPATHLEN) != 0) {
		P_ERROR ("PLibraryLoader::p_library_loader_new: failed to convert to UNIX path");
		return NULL;
	}

	path = path_buffer;

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
