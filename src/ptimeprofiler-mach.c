/*
 * Copyright (C) 2013-2016 Alexander Saprykin <xelfium@gmail.com>
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
#include "ptimeprofiler.h"

#include <mach/mach_time.h>

struct PTimeProfiler_ {
	puint64 counter;
};

static puint64 pp_time_profiler_freq_num   = 0;
static puint64 pp_time_profiler_freq_denom = 0;

static puint64 pp_time_profiler_current_ticks ();

static puint64
pp_time_profiler_current_ticks ()
{
	puint64 val = mach_absolute_time ();

	val /= 1000;
	val *= pp_time_profiler_freq_num;
	val /= pp_time_profiler_freq_denom;

	return val;
}

P_LIB_API PTimeProfiler *
p_time_profiler_new ()
{
	PTimeProfiler *ret;

	if (P_UNLIKELY (pp_time_profiler_freq_denom == 0)) {
		P_ERROR ("PTimeProfiler: tick counter is not properly initialized");
		return NULL;
	}

	if (P_UNLIKELY ((ret = p_malloc0 (sizeof (PTimeProfiler))) == NULL)) {
		P_ERROR ("PTimeProfiler: failed to allocate memory");
		return NULL;
	}

	ret->counter = pp_time_profiler_current_ticks ();

	return ret;
}

P_LIB_API void
p_time_profiler_reset (PTimeProfiler *profiler)
{
	if (P_UNLIKELY (profiler == NULL))
		return;

	profiler->counter = pp_time_profiler_current_ticks ();
}

P_LIB_API puint64
p_time_profiler_elapsed_usecs (const PTimeProfiler *profiler)
{
	if (P_UNLIKELY (profiler == NULL))
		return 0;

	return pp_time_profiler_current_ticks () - profiler->counter;
}

P_LIB_API void
p_time_profiler_free (PTimeProfiler *profiler)
{
	p_free (profiler);
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
