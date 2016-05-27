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
#include "pcondvariable.h"

#include <stdlib.h>

struct PCondVariable_ {
	pint	hdl;
};

P_LIB_API PCondVariable *
p_cond_variable_new (void)
{
	return NULL;
}

P_LIB_API void
p_cond_variable_free (PCondVariable *cond)
{
	P_UNUSED (cond);
}

P_LIB_API pboolean
p_cond_variable_wait (PCondVariable	*cond,
		      PMutex		*mutex)
{
	P_UNUSED (cond);
	P_UNUSED (mutex);
	return FALSE;
}

P_LIB_API pboolean
p_cond_variable_signal (PCondVariable *cond)
{
	P_UNUSED (cond);
	return FALSE;
}

P_LIB_API pboolean
p_cond_variable_broadcast (PCondVariable *cond)
{
	P_UNUSED (cond);
	return FALSE;
}
