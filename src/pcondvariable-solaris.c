/* 
 * 07.04.2011
 * Copyright (C) 2010-2011 Alexander Saprykin <xelfium@gmail.com>
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
#include "pcondvariable.h"

#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <thread.h>
#include <synch.h>

struct _PCondVariable {
	cond_t	hdl;
};

P_LIB_API PCondVariable *
p_cond_variable_new (void)
{
	PCondVariable *ret;

	if ((ret = p_malloc0 (sizeof (PCondVariable))) == NULL) {
		P_ERROR ("PCondVariable: failed to allocate memory");
		return NULL;
	}

	if (cond_init (&ret->hdl, NULL, NULL) != 0) {
		P_ERROR ("PCondVariable: failed to initialize conditional variable");
		p_free (ret);
		return NULL;
	}

	return ret;
}

P_LIB_API void
p_cond_variable_free (PCondVariable *cond)
{
	if (cond == NULL)
		return;
	
	if (cond_destroy (&cond->hdl) != 0)
		P_WARNING ("PCondVariable: failed to destroy handler");

	p_free (cond);
}

P_LIB_API pint
p_cond_variable_wait (PCondVariable	*cond,
		      PMutex		*mutex)
{
	if (cond == NULL || mutex == NULL)
		return -1;
	
	/* Cast is eligible since there is only one filed in PMutex structure */
	if (cond_wait (&cond->hdl, (mutex_t *) mutex) != 0) {
		P_ERROR ("PCondVariable: failed to wait");
		return -1;
	}

	return 0;
}

P_LIB_API pint
p_cond_variable_signal (PCondVariable *cond)
{
	if (cond == NULL)
		return -1;

	if (cond_signal (&cond->hdl) != 0) {
		P_ERROR ("PCondVariable: failed to signal");
		return -1;
	}

	return 0;
}

P_LIB_API pint
p_cond_variable_broadcast (PCondVariable *cond)
{
	if (cond == NULL)
		return -1;

	if (cond_broadcast (&cond->hdl) != 0) {
		P_ERROR ("PCondVariable: failed to broadcast");
		return -1;
	}

	return 0;
}

