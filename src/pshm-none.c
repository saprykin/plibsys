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
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

#include "pmem.h"
#include "pshm.h"

#include <stdlib.h>

struct _PShm {
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
