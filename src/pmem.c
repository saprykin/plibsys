/* 
 * 09.12.2010
 * Copyright (C) 2010 Alexander Saprykin <xelfium@gmail.com>
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

pboolean		p_mem_table_inited = FALSE;
static PMemVTable	p_mem_table;

static void		init_mem_table (void);

static void
init_mem_table (void)
{
	p_mem_table.malloc	= malloc;
	p_mem_table.realloc	= realloc;
	p_mem_table.free	= free;
}

P_LIB_API ppointer
p_malloc (psize n_bytes)
{
	if (!p_mem_table_inited) {
		init_mem_table ();
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
		init_mem_table ();
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
		init_mem_table ();
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
		init_mem_table ();
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
