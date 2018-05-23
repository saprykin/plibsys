/*
 * The MIT License
 *
 * Copyright (C) 2010-2017 Alexander Saprykin <saprykin.spb@gmail.com>
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

#include <string.h>
#include <stdlib.h>

#include "perror.h"
#include "pmem.h"
#include "perror-private.h"
#include "psysclose-private.h"

#ifndef P_OS_WIN
#  if defined (P_OS_BEOS)
#    include <be/kernel/OS.h>
#  elif defined (P_OS_OS2)
#    define INCL_DOSMEMMGR
#    define INCL_DOSERRORS
#    include <os2.h>
#  elif !defined (P_OS_AMIGA)
#    include <unistd.h>
#    include <sys/types.h>
#    include <sys/mman.h>
#    include <sys/stat.h>
#    include <fcntl.h>
#  endif
#endif

static pboolean		p_mem_table_inited = FALSE;
static PMemVTable	p_mem_table;

void
p_mem_init (void)
{
	if (P_UNLIKELY (p_mem_table_inited == TRUE))
		return;

	p_mem_restore_vtable ();
}

void
p_mem_shutdown (void)
{
	if (P_UNLIKELY (!p_mem_table_inited))
		return;

	p_mem_table.malloc  = NULL;
	p_mem_table.realloc = NULL;
	p_mem_table.free    = NULL;

	p_mem_table_inited = FALSE;
}

P_LIB_API ppointer
p_malloc (psize n_bytes)
{
	if (P_LIKELY (n_bytes > 0))
		return p_mem_table.malloc (n_bytes);
	else
		return NULL;
}

P_LIB_API ppointer
p_malloc0 (psize n_bytes)
{
	ppointer ret;

	if (P_LIKELY (n_bytes > 0)) {
		if (P_UNLIKELY ((ret = p_mem_table.malloc (n_bytes)) == NULL))
			return NULL;

		memset (ret, 0, n_bytes);
		return ret;
	} else
		return NULL;
}

P_LIB_API ppointer
p_realloc (ppointer mem, psize n_bytes)
{
	if (P_UNLIKELY (n_bytes == 0))
		return NULL;

	if (P_UNLIKELY (mem == NULL))
		return p_mem_table.malloc (n_bytes);
	else
		return p_mem_table.realloc (mem, n_bytes);
}

P_LIB_API void
p_free (ppointer mem)
{
	if (P_LIKELY (mem != NULL))
		p_mem_table.free (mem);
}

P_LIB_API pboolean
p_mem_set_vtable (const PMemVTable *table)
{
	if (P_UNLIKELY (table == NULL))
		return FALSE;

	if (P_UNLIKELY (table->free == NULL || table->malloc == NULL || table->realloc == NULL))
		return FALSE;

	p_mem_table.malloc  = table->malloc;
	p_mem_table.realloc = table->realloc;
	p_mem_table.free    = table->free;

	p_mem_table_inited = TRUE;

	return TRUE;
}

P_LIB_API void
p_mem_restore_vtable (void)
{
	p_mem_table.malloc  = (ppointer (*)(psize)) malloc;
	p_mem_table.realloc = (ppointer (*)(ppointer, psize)) realloc;
	p_mem_table.free    = (void (*)(ppointer)) free;

	p_mem_table_inited = TRUE;
}

P_LIB_API ppointer
p_mem_mmap (psize	n_bytes,
	    PError	**error)
{
	ppointer	addr;
#if defined (P_OS_WIN)
	HANDLE		hdl;
#elif defined (P_OS_BEOS)
	area_id		area;
#elif defined (P_OS_OS2)
	APIRET		ulrc;
#elif !defined (P_OS_AMIGA)
	int		fd;
	int		map_flags = MAP_PRIVATE;
#endif

	if (P_UNLIKELY (n_bytes == 0)) {
		p_error_set_error_p (error,
				     (pint) P_ERROR_IO_INVALID_ARGUMENT,
				     0,
				     "Invalid input argument");
		return NULL;
	}

#if defined (P_OS_WIN)
	if (P_UNLIKELY ((hdl = CreateFileMappingA (INVALID_HANDLE_VALUE,
						   NULL,
						   PAGE_READWRITE,
						   0,
						   (DWORD) n_bytes,
						   NULL)) == NULL)) {
		p_error_set_error_p (error,
				     (pint) p_error_get_last_io (),
				     p_error_get_last_system (),
				     "Failed to call CreateFileMapping() to create file mapping");
		return NULL;
	}

	if (P_UNLIKELY ((addr = MapViewOfFile (hdl,
					       FILE_MAP_READ | FILE_MAP_WRITE,
					       0,
					       0,
					       n_bytes)) == NULL)) {
		p_error_set_error_p (error,
				     (pint) p_error_get_last_io (),
				     p_error_get_last_system (),
				     "Failed to call MapViewOfFile() to map file view");
		CloseHandle (hdl);
		return NULL;
	}

	if (P_UNLIKELY (!CloseHandle (hdl))) {
		p_error_set_error_p (error,
				     (pint) p_error_get_last_io (),
				     p_error_get_last_system (),
				     "Failed to call CloseHandle() to close file mapping");
		UnmapViewOfFile (addr);
		return NULL;
	}
#elif defined (P_OS_BEOS)
	if (P_LIKELY ((n_bytes % B_PAGE_SIZE)) != 0)
		n_bytes = (n_bytes / B_PAGE_SIZE + 1) * B_PAGE_SIZE;

	area = create_area ("", &addr, B_ANY_ADDRESS, n_bytes, B_NO_LOCK, B_READ_AREA | B_WRITE_AREA);

	if (P_UNLIKELY (area < B_NO_ERROR)) {
		p_error_set_error_p (error,
				     (pint) p_error_get_last_io (),
				     p_error_get_last_system (),
				     "Failed to call create_area() to create memory area");
		return NULL;
	}
#elif defined (P_OS_OS2)
	if (P_UNLIKELY ((ulrc = DosAllocMem ((PPVOID) &addr,
					     (ULONG) n_bytes,
					     PAG_READ | PAG_WRITE | PAG_COMMIT |
					     OBJ_ANY)) != NO_ERROR)) {
		/* Try to remove OBJ_ANY */
		if (P_UNLIKELY ((ulrc = DosAllocMem ((PPVOID) &addr,
						     (ULONG) n_bytes,
						     PAG_READ | PAG_WRITE)) != NO_ERROR)) {
			p_error_set_error_p (error,
					     (pint) p_error_get_io_from_system ((pint) ulrc),
					     ulrc,
					     "Failed to call DosAllocMemory() to alocate memory");
			return NULL;
		}
	}
#elif defined (P_OS_AMIGA)
	addr = malloc (n_bytes);

	if (P_UNLIKELY (addr == NULL)) {
		p_error_set_error_p (error,
				     (pint) p_error_get_last_io (),
				     p_error_get_last_system (),
				     "Failed to allocate system memory");
		return NULL;
	}
#else
#  if !defined (PLIBSYS_MMAP_HAS_MAP_ANONYMOUS) && !defined (PLIBSYS_MMAP_HAS_MAP_ANON)
	if (P_UNLIKELY ((fd = open ("/dev/zero", O_RDWR | O_EXCL, 0754)) == -1)) {
		p_error_set_error_p (error,
				     (pint) p_error_get_last_io (),
				     p_error_get_last_system (),
				     "Failed to open /dev/zero for file mapping");
		return NULL;
	}
#  else
	fd = -1;
#  endif

#  ifdef PLIBSYS_MMAP_HAS_MAP_ANONYMOUS
	map_flags |= MAP_ANONYMOUS;
#  elif defined (PLIBSYS_MMAP_HAS_MAP_ANON)
	map_flags |= MAP_ANON;
#  endif

	if (P_UNLIKELY ((addr = mmap (NULL,
				      n_bytes,
				      PROT_READ | PROT_WRITE,
				      map_flags,
				      fd,
				      0)) == (void *) -1)) {
		p_error_set_error_p (error,
				     (pint) p_error_get_last_io (),
				     p_error_get_last_system (),
				     "Failed to call mmap() to create file mapping");
#  if !defined (PLIBSYS_MMAP_HAS_MAP_ANONYMOUS) && !defined (PLIBSYS_MMAP_HAS_MAP_ANON)
		if (P_UNLIKELY (p_sys_close (fd) != 0))
			P_WARNING ("PMem::p_mem_mmap: failed to close file descriptor to /dev/zero");
#  endif
		return NULL;
	}

#  if !defined (PLIBSYS_MMAP_HAS_MAP_ANONYMOUS) && !defined (PLIBSYS_MMAP_HAS_MAP_ANON)
	if (P_UNLIKELY (p_sys_close (fd) != 0)) {
		p_error_set_error_p (error,
				     (pint) p_error_get_last_io (),
				     p_error_get_last_system (),
				     "Failed to close /dev/zero handle");
		munmap (addr, n_bytes);
		return NULL;
	}
#  endif
#endif

	return addr;
}

P_LIB_API pboolean
p_mem_munmap (ppointer	mem,
	      psize	n_bytes,
	      PError	**error)
{
#if defined (P_OS_BEOS)
	area_id	area;
#elif defined (P_OS_OS2)
	APIRET	ulrc;
#elif defined (P_OS_AMIGA)
	P_UNUSED (n_bytes);
	P_UNUSED (error);
#endif

	if (P_UNLIKELY (mem == NULL || n_bytes == 0)) {
		p_error_set_error_p (error,
				     (pint) P_ERROR_IO_INVALID_ARGUMENT,
				     0,
				     "Invalid input argument");
		return FALSE;
	}

#if defined (P_OS_WIN)
	if (P_UNLIKELY (UnmapViewOfFile (mem) == 0)) {
		p_error_set_error_p (error,
				     (pint) p_error_get_last_io (),
				     p_error_get_last_system (),
				     "Failed to call UnmapViewOfFile() to remove file mapping");
#elif defined (P_OS_BEOS)
	if (P_UNLIKELY ((area = area_for (mem)) == B_ERROR)) {
		p_error_set_error_p (error,
				     (pint) p_error_get_last_io (),
				     p_error_get_last_system (),
				     "Failed to call area_for() to find allocated memory area");
		return FALSE;
	}

	if (P_UNLIKELY ((delete_area (area)) != B_OK)) {
		p_error_set_error_p (error,
				     (pint) p_error_get_last_io (),
				     p_error_get_last_system (),
				     "Failed to call delete_area() to remove memory area");
#elif defined (P_OS_OS2)
	if (P_UNLIKELY ((ulrc = DosFreeMem ((PVOID) mem)) != NO_ERROR)) {
		p_error_set_error_p (error,
				     (pint) p_error_get_io_from_system ((pint) ulrc),
				     ulrc,
				     "Failed to call DosFreeMem() to free memory");
#elif defined (P_OS_AMIGA)
	free (mem);

	if (P_UNLIKELY (FALSE)) {
#else
	if (P_UNLIKELY (munmap (mem, n_bytes) != 0)) {
		p_error_set_error_p (error,
				     (pint) p_error_get_last_io (),
				     p_error_get_last_system (),
				     "Failed to call munmap() to remove file mapping");
#endif
		return FALSE;
	} else
		return TRUE;
}
