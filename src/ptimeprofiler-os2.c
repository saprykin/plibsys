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

#include <stdlib.h>

static puint64 pp_time_profiler_freq = 1;

puint64
p_time_profiler_get_ticks_internal ()
{
	union {
		puint64	ticks;
		QWORD	tcounter;
	} tick_time;

	if (P_UNLIKELY (DosTmrQueryTime (&tick_time.tcounter) != NO_ERROR)) {
		P_ERROR ("PTimeProfiler::p_time_profiler_get_ticks_internal: DosTmrQueryTime() failed");
		return 0;
	}

	return tick_time.ticks;
}

puint64
p_time_profiler_elapsed_usecs_internal (const PTimeProfiler *profiler)
{
	puint64	ticks;
	lldiv_t	ldres;

	ticks = p_time_profiler_get_ticks_internal ();

	if (ticks < profiler->counter) {
		P_WARNING ("PTimeProfiler::p_time_profiler_elapsed_usecs_internal: negative jitter");
		return 1;
	}

	ticks -= profiler->counter;
	ldres  = lldiv ((pint64) ticks, (pint64) pp_time_profiler_freq);

	return (puint64) (ldres.quot * 1000000LL + (ldres.rem * 1000000LL) / pp_time_profiler_freq);
}

void
p_time_profiler_init (void)
{
	ULONG freq;

	if (P_UNLIKELY (DosTmrQueryFreq (&freq) != NO_ERROR)) {
		P_ERROR ("PTimeProfiler::p_time_profiler_init: DosTmrQueryFreq() failed");
		return;
	}

	pp_time_profiler_freq = (puint64) freq;
}

void
p_time_profiler_shutdown (void)
{
	pp_time_profiler_freq = 1;
}
