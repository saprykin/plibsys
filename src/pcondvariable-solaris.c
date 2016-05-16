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

	if (P_UNLIKELY ((ret = p_malloc0 (sizeof (PCondVariable))) == NULL)) {
		P_ERROR ("PCondVariable: failed to allocate memory");
		return NULL;
	}

	if (P_UNLIKELY (cond_init (&ret->hdl, NULL, NULL) != 0)) {
		P_ERROR ("PCondVariable: failed to initialize conditional variable");
		p_free (ret);
		return NULL;
	}

	return ret;
}

P_LIB_API void
p_cond_variable_free (PCondVariable *cond)
{
	if (P_UNLIKELY (cond == NULL))
		return;

	if (P_UNLIKELY (cond_destroy (&cond->hdl) != 0))
		P_WARNING ("PCondVariable: failed to destroy handler");

	p_free (cond);
}

P_LIB_API pboolean
p_cond_variable_wait (PCondVariable	*cond,
		      PMutex		*mutex)
{
	if (P_UNLIKELY (cond == NULL || mutex == NULL))
		return FALSE;

	/* Cast is eligible since there is only one field in the PMutex structure */
	if (P_UNLIKELY (cond_wait (&cond->hdl, (mutex_t *) mutex) != 0)) {
		P_ERROR ("PCondVariable: failed to wait");
		return FALSE;
	}

	return TRUE;
}

P_LIB_API pboolean
p_cond_variable_signal (PCondVariable *cond)
{
	if (P_UNLIKELY (cond == NULL))
		return FALSE;

	if (P_UNLIKELY (cond_signal (&cond->hdl) != 0)) {
		P_ERROR ("PCondVariable: failed to signal");
		return FALSE;
	}

	return TRUE;
}

P_LIB_API pboolean
p_cond_variable_broadcast (PCondVariable *cond)
{
	if (P_UNLIKELY (cond == NULL))
		return FALSE;

	if (P_UNLIKELY (cond_broadcast (&cond->hdl) != 0)) {
		P_ERROR ("PCondVariable: failed to broadcast");
		return FALSE;
	}

	return TRUE;
}
