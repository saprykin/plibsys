/*
 * Copyright (C) 2016 Alexander Saprykin <xelfium@gmail.com>
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, see <http://www.gnu.org/licenses/>.
 */

#include "pmem.h"
#include "ptimeprofiler.h"
#include "ptimeprofiler-private.h"

extern puint64 p_time_profiler_get_ticks_internal (void);
extern puint64 p_time_profiler_elapsed_usecs_internal (const PTimeProfiler *profiler);

P_LIB_API PTimeProfiler *
p_time_profiler_new ()
{
	PTimeProfiler *ret;

	if (P_UNLIKELY ((ret = p_malloc0 (sizeof (PTimeProfiler))) == NULL)) {
		P_ERROR ("PTimeProfiler: failed to allocate memory");
		return NULL;
	}

	ret->counter = p_time_profiler_get_ticks_internal ();

	return ret;
}

P_LIB_API void
p_time_profiler_reset (PTimeProfiler *profiler)
{
	if (P_UNLIKELY (profiler == NULL))
		return;

	profiler->counter = p_time_profiler_get_ticks_internal ();
}

P_LIB_API puint64
p_time_profiler_elapsed_usecs (const PTimeProfiler *profiler)
{
	if (P_UNLIKELY (profiler == NULL))
		return 0;

	return p_time_profiler_elapsed_usecs_internal (profiler);
}

P_LIB_API void
p_time_profiler_free (PTimeProfiler *profiler)
{
	p_free (profiler);
}
