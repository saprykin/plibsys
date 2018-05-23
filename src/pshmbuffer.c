/*
 * The MIT License
 *
 * Copyright (C) 2010-2016 Alexander Saprykin <saprykin.spb@gmail.com>
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

#include "pmem.h"
#include "pshm.h"
#include "pshmbuffer.h"

#include <stdlib.h>
#include <string.h>

#define P_SHM_BUFFER_READ_OFFSET	0
#define P_SHM_BUFFER_WRITE_OFFSET	sizeof (psize)
#define P_SHM_BUFFER_DATA_OFFSET	sizeof (psize) * 2

struct PShmBuffer_ {
	PShm *shm;
	psize size;
};

static psize pp_shm_buffer_get_free_space (PShmBuffer *buf);
static psize pp_shm_buffer_get_used_space (PShmBuffer *buf);

/* Warning: this function is not thread-safe, only for internal usage */
static psize
pp_shm_buffer_get_free_space (PShmBuffer *buf)
{
	psize		read_pos, write_pos;
	ppointer	addr;

	addr = p_shm_get_address (buf->shm);

	memcpy (&read_pos, (pchar *) addr + P_SHM_BUFFER_READ_OFFSET, sizeof (read_pos));
	memcpy (&write_pos, (pchar *) addr + P_SHM_BUFFER_WRITE_OFFSET, sizeof (write_pos));

	if (write_pos < read_pos)
		return read_pos - write_pos;
	else if (write_pos > read_pos)
		return buf->size - (write_pos - read_pos) - 1;
	else
		return buf->size - 1;
}

static psize
pp_shm_buffer_get_used_space (PShmBuffer *buf)
{
	psize		read_pos, write_pos;
	ppointer	addr;

	addr = p_shm_get_address (buf->shm);

	memcpy (&read_pos, (pchar *) addr + P_SHM_BUFFER_READ_OFFSET, sizeof (read_pos));
	memcpy (&write_pos, (pchar *) addr + P_SHM_BUFFER_WRITE_OFFSET, sizeof (write_pos));

	if (write_pos > read_pos)
		return write_pos - read_pos;
	else if (write_pos < read_pos)
		return (buf->size - (read_pos - write_pos));
	else
		return 0;
}

P_LIB_API PShmBuffer *
p_shm_buffer_new (const pchar	*name,
		  psize		size,
		  PError	**error)
{
	PShmBuffer	*ret;
	PShm		*shm;

	if (P_UNLIKELY (name == NULL)) {
		p_error_set_error_p (error,
				     (pint) P_ERROR_IPC_INVALID_ARGUMENT,
				     0,
				     "Invalid input argument");
		return NULL;
	}

	if (P_UNLIKELY ((shm = p_shm_new (name,
					  (size != 0) ? size + P_SHM_BUFFER_DATA_OFFSET + 1 : 0,
					  P_SHM_ACCESS_READWRITE,
					  error)) == NULL))
		return NULL;

	if (P_UNLIKELY (p_shm_get_size (shm) <= P_SHM_BUFFER_DATA_OFFSET + 1)) {
		p_error_set_error_p (error,
				     (pint) P_ERROR_IPC_INVALID_ARGUMENT,
				     0,
				     "Too small memory segment to hold required data");
		p_shm_free (shm);
		return NULL;
	}

	if (P_UNLIKELY ((ret = p_malloc0 (sizeof (PShmBuffer))) == NULL)) {
		p_error_set_error_p (error,
				     (pint) P_ERROR_IPC_NO_RESOURCES,
				     0,
				     "Failed to allocate memory for shared buffer");
		p_shm_free (shm);
		return NULL;
	}

	ret->shm  = shm;
	ret->size = p_shm_get_size (shm) - P_SHM_BUFFER_DATA_OFFSET;

	return ret;
}

P_LIB_API void
p_shm_buffer_free (PShmBuffer *buf)
{
	if (P_UNLIKELY (buf == NULL))
		return;

	p_shm_free (buf->shm);
	p_free (buf);
}

P_LIB_API void
p_shm_buffer_take_ownership (PShmBuffer *buf)
{
	if (P_UNLIKELY (buf == NULL))
		return;

	p_shm_take_ownership (buf->shm);
}

P_LIB_API pint
p_shm_buffer_read (PShmBuffer	*buf,
		   ppointer	storage,
		   psize	len,
		   PError	**error)
{
	psize		read_pos, write_pos;
	psize		data_aval, to_copy;
	puint		i;
	ppointer	addr;

	if (P_UNLIKELY (buf == NULL || storage == NULL || len == 0)) {
		p_error_set_error_p (error,
				     (pint) P_ERROR_IPC_INVALID_ARGUMENT,
				     0,
				     "Invalid input argument");
		return -1;
	}

	if (P_UNLIKELY ((addr = p_shm_get_address (buf->shm)) == NULL)) {
		p_error_set_error_p (error,
				     (pint) P_ERROR_IPC_INVALID_ARGUMENT,
				     0,
				     "Unable to get shared memory address");
		return -1;
	}

	if (P_UNLIKELY (p_shm_lock (buf->shm, error) == FALSE))
		return -1;

	memcpy (&read_pos, (pchar *) addr + P_SHM_BUFFER_READ_OFFSET, sizeof (read_pos));
	memcpy (&write_pos, (pchar *) addr + P_SHM_BUFFER_WRITE_OFFSET, sizeof (write_pos));

	if (read_pos == write_pos) {
		if (P_UNLIKELY (p_shm_unlock (buf->shm, error) == FALSE))
			return -1;

		return 0;
	}

	data_aval = pp_shm_buffer_get_used_space (buf);
	to_copy   = (data_aval <= len) ? data_aval : len;

	for (i = 0; i < to_copy; ++i)
		memcpy ((pchar *) storage + i,
			(pchar *) addr + P_SHM_BUFFER_DATA_OFFSET + ((read_pos + i) % buf->size),
			1);

	read_pos = (read_pos + to_copy) % buf->size;
	memcpy ((pchar *) addr + P_SHM_BUFFER_READ_OFFSET, &read_pos, sizeof (read_pos));

	if (P_UNLIKELY (p_shm_unlock (buf->shm, error) == FALSE))
		return -1;

	return (pint) to_copy;
}

P_LIB_API pssize
p_shm_buffer_write (PShmBuffer	*buf,
		    ppointer	data,
		    psize	len,
		    PError	**error)
{
	psize		read_pos, write_pos;
	puint		i;
	ppointer	addr;

	if (P_UNLIKELY (buf == NULL || data == NULL || len == 0)) {
		p_error_set_error_p (error,
				     (pint) P_ERROR_IPC_INVALID_ARGUMENT,
				     0,
				     "Invalid input argument");
		return -1;
	}

	if (P_UNLIKELY ((addr = p_shm_get_address (buf->shm)) == NULL)) {
		p_error_set_error_p (error,
				     (pint) P_ERROR_IPC_INVALID_ARGUMENT,
				     0,
				     "Unable to get shared memory address");
		return -1;
	}

	if (P_UNLIKELY (p_shm_lock (buf->shm, error) == FALSE))
		return -1;

	memcpy (&read_pos, (pchar *) addr + P_SHM_BUFFER_READ_OFFSET, sizeof (read_pos));
	memcpy (&write_pos, (pchar *) addr + P_SHM_BUFFER_WRITE_OFFSET, sizeof (write_pos));

	if (pp_shm_buffer_get_free_space (buf) < len) {
		if (P_UNLIKELY (p_shm_unlock (buf->shm, error) == FALSE))
			return -1;

		return 0;
	}

	for (i = 0; i < len; ++i)
		memcpy ((pchar *) addr + P_SHM_BUFFER_DATA_OFFSET + ((write_pos + i) % buf->size),
			(pchar *) data + i,
			1);

	write_pos = (write_pos + len) % buf->size;
	memcpy ((pchar *) addr + P_SHM_BUFFER_WRITE_OFFSET, &write_pos, sizeof (write_pos));

	if (P_UNLIKELY (p_shm_unlock (buf->shm, error) == FALSE))
		return -1;

	return (pssize) len;
}

P_LIB_API pssize
p_shm_buffer_get_free_space (PShmBuffer	*buf,
			     PError	**error)
{
	psize space;

	if (P_UNLIKELY (buf == NULL)) {
		p_error_set_error_p (error,
				     (pint) P_ERROR_IPC_INVALID_ARGUMENT,
				     0,
				     "Invalid input argument");
		return -1;
	}

	if (P_UNLIKELY (p_shm_lock (buf->shm, error) == FALSE))
		return -1;

	space = pp_shm_buffer_get_free_space (buf);

	if (P_UNLIKELY (p_shm_unlock (buf->shm, error) == FALSE))
		return -1;

	return (pssize) space;
}

P_LIB_API pssize
p_shm_buffer_get_used_space (PShmBuffer	*buf,
			     PError	**error)
{
	psize space;

	if (P_UNLIKELY (buf == NULL)) {
		p_error_set_error_p (error,
				     (pint) P_ERROR_IPC_INVALID_ARGUMENT,
				     0,
				     "Invalid input argument");
		return -1;
	}

	if (P_UNLIKELY (p_shm_lock (buf->shm, error) == FALSE))
		return -1;

	space = pp_shm_buffer_get_used_space (buf);

	if (P_UNLIKELY (p_shm_unlock (buf->shm, error) == FALSE))
		return -1;

	return (pssize) space;
}

P_LIB_API void
p_shm_buffer_clear (PShmBuffer *buf)
{
	ppointer	addr;
	psize		size;

	if (P_UNLIKELY (buf == NULL))
		return;

	if (P_UNLIKELY ((addr = p_shm_get_address (buf->shm)) == NULL)) {
		P_ERROR ("PShmBuffer::p_shm_buffer_clear: p_shm_get_address() failed");
		return;
	}

	size = p_shm_get_size (buf->shm);

	if (P_UNLIKELY (p_shm_lock (buf->shm, NULL) == FALSE)) {
		P_ERROR ("PShmBuffer::p_shm_buffer_clear: p_shm_lock() failed");
		return;
	}

	memset (addr, 0, size);

	if (P_UNLIKELY (p_shm_unlock (buf->shm, NULL) == FALSE))
		P_ERROR ("PShmBuffer::p_shm_buffer_clear: p_shm_unlock() failed");
}
