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

#include "pmem.h"
#include "psemaphore.h"

#include <stdlib.h>

struct _PSemaphore {
	pint	hdl;
};

P_LIB_API PSemaphore *
p_semaphore_new (const pchar *name,  pint init_val, PSemaphoreAccessMode mode)
{
	if (name == NULL || init_val < 0)
		return NULL;
		
	P_INT_TO_POINTER (mode);

	return NULL;
}

P_LIB_API void
p_semaphore_take_ownership (PSemaphore *sem)
{
	P_UNUSED (sem);
}

P_LIB_API pboolean
p_semaphore_acquire (PSemaphore *sem)
{
	if (sem == NULL)
		return FALSE;

	return FALSE;
}

P_LIB_API pboolean
p_semaphore_release (PSemaphore *sem)
{
	if (sem == NULL)
		return FALSE;

	return FALSE;
}

P_LIB_API void
p_semaphore_free (PSemaphore *sem)
{
	if (sem == NULL)
		return;
}

