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

#include "psemaphore.h"

#include <stdlib.h>

struct PSemaphore_ {
	pint	hdl;
};

P_LIB_API PSemaphore *
p_semaphore_new (const pchar		*name,
		 pint			init_val,
		 PSemaphoreAccessMode	mode,
		 PError			**error)
{
	P_UNUSED (name);
	P_UNUSED (init_val);
	P_UNUSED (mode);

	p_error_set_error_p (error,
			     (pint) P_ERROR_IPC_NOT_IMPLEMENTED,
			     0,
			     "No semaphore implementation");

	return NULL;
}

P_LIB_API void
p_semaphore_take_ownership (PSemaphore *sem)
{
	P_UNUSED (sem);
}

P_LIB_API pboolean
p_semaphore_acquire (PSemaphore *sem,
		     PError	**error)
{
	P_UNUSED (sem);

	p_error_set_error_p (error,
			     (pint) P_ERROR_IPC_NOT_IMPLEMENTED,
			     0,
			     "No semaphore implementation");

	return FALSE;
}

P_LIB_API pboolean
p_semaphore_release (PSemaphore *sem,
		     PError	**error)
{
	P_UNUSED (sem);

	p_error_set_error_p (error,
			     (pint) P_ERROR_IPC_NOT_IMPLEMENTED,
			     0,
			     "No semaphore implementation");

	return FALSE;
}

P_LIB_API void
p_semaphore_free (PSemaphore *sem)
{
	P_UNUSED (sem);
}

