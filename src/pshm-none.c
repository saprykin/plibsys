/*
 * Copyright (C) 2010-2013 Alexander Saprykin <xelfium@gmail.com>
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

/* TODO: error report system */

#include "pmem.h"
#include "pshm.h"

#include <stdlib.h>

struct _PShm {
	pint	hdl;
};

P_LIB_API PShm *
p_shm_new (const pchar		*name,
	   psize		size,
	   PShmAccessPerms	perms)
{
	if (name == NULL)
		return NULL;

	/* To shut up compiler */
	P_UNUSED (size);
	P_UNUSED (perms);

	return NULL;
}

P_LIB_API void
p_shm_free (PShm *shm)
{
	P_UNUSED (shm);
}

P_LIB_API pboolean
p_shm_lock (PShm *shm)
{
	P_UNUSED (shm);

	return FALSE;
}

P_LIB_API pboolean
p_shm_unlock (PShm *shm)
{
	P_UNUSED (shm);

	return FALSE;
}

P_LIB_API ppointer
p_shm_get_address (PShm *shm)
{
	P_UNUSED (shm);

	return NULL;
}

P_LIB_API psize
p_shm_get_size (PShm *shm)
{
	P_UNUSED (shm);

	return 0;
}
