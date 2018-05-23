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

#include "pshm.h"

#include <stdlib.h>

struct PShm_ {
	pint	hdl;
};

P_LIB_API PShm *
p_shm_new (const pchar		*name,
	   psize		size,
	   PShmAccessPerms	perms,
	   PError		**error)
{
	P_UNUSED (name);
	P_UNUSED (size);
	P_UNUSED (perms);
	P_UNUSED (error);

	return NULL;
}

P_LIB_API void
p_shm_take_ownership (PShm *shm)
{
	P_UNUSED (shm);
}

P_LIB_API void
p_shm_free (PShm *shm)
{
	P_UNUSED (shm);
}

P_LIB_API pboolean
p_shm_lock (PShm	*shm,
	    PError	**error)
{
	P_UNUSED (shm);
	P_UNUSED (error);

	return FALSE;
}

P_LIB_API pboolean
p_shm_unlock (PShm	*shm,
	      PError	**error)
{
	P_UNUSED (shm);
	P_UNUSED (error);

	return FALSE;
}

P_LIB_API ppointer
p_shm_get_address (const PShm *shm)
{
	P_UNUSED (shm);

	return NULL;
}

P_LIB_API psize
p_shm_get_size (const PShm *shm)
{
	P_UNUSED (shm);

	return 0;
}
