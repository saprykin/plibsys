/*
 * Copyright (C) 2010-2016 Alexander Saprykin <xelfium@gmail.com>
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

#include <string.h>
#include <stdlib.h>

#include "pmem.h"
#include "plib-private.h"

#ifdef P_OS_WIN
#include <windows.h>
#else
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#endif

pboolean		p_mem_table_inited = FALSE;
static PMemVTable	p_mem_table;

static void		__p_mem_init_mem_table (void);

static void
__p_mem_init_mem_table (void)
{
	p_mem_table.malloc	= (ppointer (*)(psize)) malloc;
	p_mem_table.realloc	= (ppointer (*)(ppointer, psize)) realloc;
	p_mem_table.free	= (void (*)(ppointer)) free;
}

P_LIB_API ppointer
p_malloc (psize n_bytes)
{
	if (!p_mem_table_inited) {
		__p_mem_init_mem_table ();
		p_mem_table_inited = TRUE;
	}

	if (n_bytes)
		return p_mem_table.malloc (n_bytes);
	else
		return NULL;
}

P_LIB_API ppointer
p_malloc0 (psize n_bytes)
{
	ppointer ret;

	if (!p_mem_table_inited) {
		__p_mem_init_mem_table ();
		p_mem_table_inited = TRUE;
	}

	if (n_bytes) {
		if ((ret = p_mem_table.malloc (n_bytes)) == NULL)
			return NULL;

		memset (ret, 0, n_bytes);
		return ret;
	}
	else
		return NULL;
}

P_LIB_API ppointer
p_realloc (ppointer mem, psize n_bytes)
{
	if (!n_bytes)
		return NULL;

	if (!p_mem_table_inited) {
		__p_mem_init_mem_table ();
		p_mem_table_inited = TRUE;
	}

	if (mem == NULL)
		return p_mem_table.malloc (n_bytes);
	else
		return p_mem_table.realloc (mem, n_bytes);
}

P_LIB_API void
p_free (ppointer mem)
{
	if (!p_mem_table_inited) {
		__p_mem_init_mem_table ();
		p_mem_table_inited = TRUE;
	}

	if (mem != NULL)
		p_mem_table.free (mem);
}

P_LIB_API pboolean
p_mem_set_vtable (PMemVTable *table)
{
	if (table == NULL)
		return FALSE;

	if (table->free == NULL || table->malloc == NULL || table->realloc == NULL)
		return FALSE;

	p_mem_table.malloc	= table->malloc;
	p_mem_table.realloc	= table->realloc;
	p_mem_table.free	= table->free;

	return TRUE;
}

P_LIB_API ppointer
p_mem_mmap (psize	n_bytes,
	    PError	**error)
{
	ppointer	addr;
#ifdef P_OS_WIN
	HANDLE		hdl;
#else
	int		fd;
	int		map_flags = MAP_PRIVATE;
#endif

	if (n_bytes == 0) {
		p_error_set_error_p (error,
				     (pint) P_ERROR_IO_INVALID_ARGUMENT,
				     0,
				     "Invalid input argument");
		return NULL;
	}

#ifdef P_OS_WIN
	if ((hdl = CreateFileMapping (INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, (DWORD) n_bytes, NULL)) == NULL) {
		p_error_set_error_p (error,
				     (pint) __p_error_get_last_io (),
				     (pint) GetLastError (),
				     "Failed to call CreateFileMapping() to create file mapping");
		return NULL;
	}

	if ((addr = MapViewOfFile (hdl, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, n_bytes)) == NULL) {
		p_error_set_error_p (error,
				     (pint) __p_error_get_last_io (),
				     (pint) GetLastError (),
				     "Failed to call MapViewOfFile() to map file view");
		CloseHandle (hdl);
		return NULL;
	}

	if (!CloseHandle (hdl)) {
		p_error_set_error_p (error,
				     (pint) __p_error_get_last_io (),
				     (pint) GetLastError (),
				     "Failed to call CloseHandle() to close file mapping");
		UnmapViewOfFile (addr);
		return NULL;
	}
#else
#  if !defined (PLIB_MMAP_HAS_MAP_ANONYMOUS) && !defined (PLIB_MMAP_HAS_MAP_ANON)
	if ((fd = open ("/dev/zero", O_RDWR | O_EXCL, 0754)) == -1) {
		p_error_set_error_p (error,
				     (pint) __p_error_get_last_io (),
				     (pint) errno,
				     "Failed to open /dev/zero for file mapping");
		return NULL;
	}
#  else
	fd = -1;
#  endif

#  ifdef PLIB_MMAP_HAS_MAP_ANONYMOUS
	map_flags |= MAP_ANONYMOUS;
#  elif defined (PLIB_MMAP_HAS_MAP_ANON)
	map_flags |= MAP_ANON;
#  endif

	if ((addr = mmap (NULL, n_bytes, PROT_READ | PROT_WRITE, map_flags, fd, 0)) == (void *) -1) {
		p_error_set_error_p (error,
				     (pint) __p_error_get_last_io (),
				     (pint) errno,
				     "Failed to call mmap() to create file mapping");
#  if !defined (PLIB_MMAP_HAS_MAP_ANONYMOUS) && !defined (PLIB_MMAP_HAS_MAP_ANON)
		close (fd);
#  endif
		return NULL;
	}

#  if !defined (PLIB_MMAP_HAS_MAP_ANONYMOUS) && !defined (PLIB_MMAP_HAS_MAP_ANON)
	if (close (fd) == -1) {
		p_error_set_error_p (error,
				     (pint) __p_error_get_last_io (),
				     (pint) errno,
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
	if (mem == NULL || n_bytes == 0) {
		p_error_set_error_p (error,
				     (pint) P_ERROR_IO_INVALID_ARGUMENT,
				     0,
				     "Invalid input argument");
		return FALSE;
	}

#ifdef P_OS_WIN
	if (UnmapViewOfFile (mem) == 0) {
		p_error_set_error_p (error,
				     (pint) __p_error_get_last_io (),
				     (pint) GetLastError (),
				     "Failed to call UnmapViewOfFile() to remove file mapping");
#else
	if (munmap (mem, n_bytes) != 0) {
		p_error_set_error_p (error,
				     (pint) __p_error_get_last_io (),
				     (pint) errno,
				     "Failed to call munmap() to remove file mapping");
#endif
		return FALSE;
	} else
		return TRUE;
}
