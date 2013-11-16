/* 
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

#include "pmem.h"
#include "pshmbuffer.h"
#include "ptypes.h"
#include "pshm.h"

#define SHM_BUFFER_READ_OFFSET	0
#define SHM_BUFFER_WRITE_OFFSET	sizeof (puint32)
#define SHM_BUFFER_DATA_OFFSET	sizeof (puint32) * 2

struct _PShmBuffer {
	PShm *shm;
	psize size;
};

static psize get_free_space (PShmBuffer *buf);
static pssize get_used_space (PShmBuffer *buf);

/* Warning: this function is not thread-safe, only for internal usage */
static psize
get_free_space (PShmBuffer *buf)
{
	puint32		read_pos, write_pos;
	ppointer	addr;

	if (buf == NULL)
		return 0;

	if ((addr = p_shm_get_address (buf->shm)) == NULL) {
		P_ERROR ("PShmBuffer: failed to get memory address");
		return 0;
	}

	memcpy (&read_pos, (pchar *) addr + SHM_BUFFER_READ_OFFSET, sizeof (read_pos));
	memcpy (&write_pos, (pchar *) addr + SHM_BUFFER_WRITE_OFFSET, sizeof (write_pos));

	if (write_pos < read_pos)
		return read_pos - write_pos;
	else if (write_pos > read_pos)
		return buf->size - (write_pos - read_pos) - 1;
	else
		return buf->size - 1;
}

static pssize
get_used_space (PShmBuffer *buf)
{
	puint32		read_pos, write_pos;
	ppointer	addr;

	if (buf == NULL)
		return -1;

	if ((addr = p_shm_get_address (buf->shm)) == NULL) {
		P_ERROR ("PShmBuffer: failed to get memory address");
		return -1;
	}

	memcpy (&read_pos, (pchar *) addr + SHM_BUFFER_READ_OFFSET, sizeof (read_pos));
	memcpy (&write_pos, (pchar *) addr + SHM_BUFFER_WRITE_OFFSET, sizeof (write_pos));

	if (write_pos > read_pos)
		return write_pos - read_pos;
	else if (write_pos < read_pos)
		return (buf->size - (read_pos - write_pos));
	else
		return 0;
}

P_LIB_API PShmBuffer *
p_shm_buffer_new (const pchar *name, psize size)
{
	PShmBuffer	*ret;
	PShm		*shm;
	
	if (name == NULL)
		return NULL;

	if ((shm = p_shm_new (name, (size != 0) ? size + SHM_BUFFER_DATA_OFFSET + 1 : 0, P_SHM_ACCESS_READWRITE)) == NULL) {
		P_ERROR ("PShmBuffer: failed to allocate memory");
		return NULL;
	}

	if (p_shm_get_size (shm) - SHM_BUFFER_DATA_OFFSET - 1 <= 0) {
		P_ERROR ("PShmBuffer: too small memory segment opened");
		p_shm_free (shm);
		return NULL;
	}

	if ((ret = p_malloc0 (sizeof (PShmBuffer))) == NULL) {
		P_ERROR ("PShmBuffer: failed to allocate memory");
		p_shm_free (shm);
		return NULL;
	}

	ret->shm = shm;
	ret->size = p_shm_get_size (shm) - SHM_BUFFER_DATA_OFFSET;

	return ret;
}

P_LIB_API void
p_shm_buffer_free (PShmBuffer *buf)
{
	if (buf == NULL)
		return;
		
	p_shm_free (buf->shm);
	p_free (buf);
}

P_LIB_API pint
p_shm_buffer_read (PShmBuffer *buf, ppointer storage, psize len)
{
	psize		read_pos, write_pos;
	psize		data_aval, to_copy;
	puint		i;
	ppointer	addr;

	if (buf == NULL || storage == NULL || len == 0)
		return -1;

	if ((addr = p_shm_get_address (buf->shm)) == NULL) {
		P_ERROR ("PShmBuffer: failed to get memory address");
		return -1;
	}

	if (!p_shm_lock (buf->shm)) {
		P_ERROR ("PShmBuffer: failed to lock memory for reading");
		return -1;
	}

	memcpy (&read_pos, (pchar *) addr + SHM_BUFFER_READ_OFFSET, sizeof (read_pos));
	memcpy (&write_pos, (pchar *) addr + SHM_BUFFER_WRITE_OFFSET, sizeof (write_pos));

	if (read_pos == write_pos) {
		if (!p_shm_unlock (buf->shm))
			P_ERROR ("PShmBuffer: failed to unlock memory after reading");
		
		return 0;
	}

	data_aval = get_used_space (buf);
	to_copy = (data_aval <= len) ? data_aval : len;

	/* TODO: Handle exceptions on Windows */
	for (i = 0; i < to_copy; ++i)
		memcpy ((pchar *) storage + i, (pchar *) addr + SHM_BUFFER_DATA_OFFSET + ((read_pos + i) % buf->size), 1);

	read_pos = (read_pos + to_copy) % buf->size;
	memcpy ((pchar *) addr + SHM_BUFFER_READ_OFFSET, &read_pos, sizeof (read_pos));

	if (!p_shm_unlock (buf->shm))
		P_ERROR ("PShmBuffer: failed to unlock memory after reading");

	return (pint) to_copy;
}

P_LIB_API pssize
p_shm_buffer_write (PShmBuffer *buf, ppointer data, psize len)
{
	psize		read_pos, write_pos;
	puint		i;
	ppointer	addr;

	if (buf == NULL || data == NULL || len == 0)
		return -1;

	if ((addr = p_shm_get_address (buf->shm)) == NULL) {
		P_ERROR ("PShmBuffer: failed to get memory address");
		return -1;
	}

	if (!p_shm_lock (buf->shm)) {
		P_ERROR ("PShmBuffer: failed to lock memory for writing");
		return -1;
	}

	memcpy (&read_pos, (pchar *) addr + SHM_BUFFER_READ_OFFSET, sizeof (read_pos));
	memcpy (&write_pos, (pchar *) addr + SHM_BUFFER_WRITE_OFFSET, sizeof (write_pos));

	if (get_free_space (buf) < len) {
		if (!p_shm_unlock (buf->shm))
			P_ERROR ("PShmBuffer: failed to unlock memory after writing");

		return -1;
	}

	/* TODO: Handle exceptions on Windows */
	for (i = 0; i < len; ++i)
		memcpy ((pchar *) addr + SHM_BUFFER_DATA_OFFSET + ((write_pos + i) % buf->size), (char *) data + i, 1);

	write_pos = (write_pos + len) % buf->size;
	memcpy ((pchar *) addr + SHM_BUFFER_WRITE_OFFSET, &write_pos, sizeof (write_pos));

	if (!p_shm_unlock (buf->shm))
		P_ERROR ("PShmBuffer: failed to unlock memory after writing");

	return len;
}

P_LIB_API psize
p_shm_buffer_get_free_space (PShmBuffer *buf)
{
	psize space;

	if (buf == NULL)
		return 0;

	if (!p_shm_lock (buf->shm)) {
		P_ERROR ("PShmBuffer: failed to lock memory to get free space");
		return 0;
	}

	space = get_free_space (buf);

	if (!p_shm_unlock (buf->shm))
		P_ERROR ("PShmBuffer: failed to unlock memory after getting free space");

	return space;
}

P_LIB_API pssize
p_shm_buffer_get_used_space (PShmBuffer *buf)
{
	pssize space;

	if (buf == NULL)
		return -1;

	if (!p_shm_lock (buf->shm)) {
		P_ERROR ("PShmBuffer: failed to lock memory to get used space");
		return -1;
	}

	space = get_used_space (buf);

	if (!p_shm_unlock (buf->shm))
		P_ERROR ("PShmBuffer: failed to unlock memory after getting used space");

	return space;
}

P_LIB_API void
p_shm_buffer_clear (PShmBuffer *buf)
{
	ppointer	addr;
	psize		size;

	if (buf == NULL)
		return;

	if ((addr = p_shm_get_address (buf->shm)) == NULL) {
		P_ERROR ("PShmBuffer: failed to get memory address");
		return;
	}

	size = p_shm_get_size (buf->shm);

	if (!p_shm_lock (buf->shm)) {
		P_ERROR ("PShmBuffer: failed to lock memory for clearance");
		return;
	}

	memset (addr, 0, size);

	if (!p_shm_unlock (buf->shm))
		P_ERROR ("PShmBuffer: failed to unlock memory after clearance");
}
