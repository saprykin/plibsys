/*
 * Copyright (C) 2013-2016 Alexander Saprykin <xelfium@gmail.com>
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

#include "ptimeprofiler.h"
#include "ptimeprofiler-private.h"

#include <mach/mach_time.h>

static puint64 pp_time_profiler_freq_num   = 0;
static puint64 pp_time_profiler_freq_denom = 0;

puint64
p_time_profiler_get_ticks_internal ()
{
	puint64 val = mach_absolute_time ();

	/* To prevent overflow */
	val /= 1000;

	val *= pp_time_profiler_freq_num;
	val /= pp_time_profiler_freq_denom;

	return val;
}

puint64
p_time_profiler_elapsed_usecs_internal (const PTimeProfiler *profiler)
{
	return p_time_profiler_get_ticks_internal () - profiler->counter;
}

void
p_time_profiler_init (void)
{
	mach_timebase_info_data_t tb;

	if (P_UNLIKELY (mach_timebase_info (&tb) != KERN_SUCCESS || tb.denom == 0)) {
		P_ERROR ("PTimeProfiler: failed to get clock frequency using mach_timebase_info()");
		return;
	}

	pp_time_profiler_freq_num   = (puint64) tb.numer;
	pp_time_profiler_freq_denom = (puint64) tb.denom;
}

void
p_time_profiler_shutdown (void)
{
	pp_time_profiler_freq_num   = 0;
	pp_time_profiler_freq_denom = 0;
}
