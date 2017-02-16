/*
 * Copyright (C) 2017 Alexander Saprykin <xelfium@gmail.com>
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

#define INCL_DOSPROFILE
#define INCL_DOSERRORS
#include <os2.h>

static puint64 pp_time_profiler_freq = 1;

puint64
p_time_profiler_get_ticks_internal ()
{
	QWORD	tcounter;
	puint64	time_low;
	puint64	time_high;

	if (P_UNLIKELY (DosTmrQueryTime (&tcounter) != NO_ERROR)) {
		P_ERROR ("PTimeProfiler::p_time_profiler_get_ticks_internal: DosTmrQueryTime() failed");
		return 0;
	}

	time_low  = (puint64) tcounter.ulLo;
	time_high = (puint64) tcounter.ulHi;

	return (time_high << 32) | time_low;
}

puint64
p_time_profiler_elapsed_usecs_internal (const PTimeProfiler *profiler)
{
	return (p_time_profiler_get_ticks_internal () - profiler->counter) / pp_time_profiler_freq;
}

void
p_time_profiler_init (void)
{
	ULONG freq;

	if (P_UNLIKELY (DosTmrQueryFreq (&freq) != NO_ERROR)) {
		P_ERROR ("PTimeProfiler::p_time_profiler_init: DosTmrQueryFreq() failed");
		return;
	}

	pp_time_profiler_freq = (puint64) (freq / 1000000.0F);
}

void
p_time_profiler_shutdown (void)
{
	pp_time_profiler_freq = 1;
}
