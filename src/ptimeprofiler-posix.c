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

#include <unistd.h>
#include <time.h>
#include <sys/time.h>

#ifndef _POSIX_MONOTONIC_CLOCK
#  define _POSIX_MONOTONIC_CLOCK (-1)
#endif

struct PTimeProfiler_ {
	puint64		counter;
	pboolean	hasMonotonicClock;
};

static puint64 pp_time_profiler_current_ticks (const PTimeProfiler *profiler);
static puint64 pp_time_profiler_current_tick_gtod ();

static puint64
pp_time_profiler_current_ticks (const PTimeProfiler *profiler)
{
#if _POSIX_MONOTONIC_CLOCK >= 0
	struct timespec	ts;
#endif

#if _POSIX_MONOTONIC_CLOCK >= 0
	if (P_LIKELY (profiler->hasMonotonicClock == TRUE)) {
		if (P_UNLIKELY (clock_gettime (CLOCK_MONOTONIC, &ts) != 0)) {
			P_ERROR ("PTimeProfiler: Failed to get time using clock_gettime()");
			return pp_time_profiler_current_tick_gtod ();
		} else
			return (puint64) (ts.tv_sec * 1000000 + ts.tv_nsec / 1000);
	} else
		return pp_time_profiler_current_tick_gtod ();
#else
	return pp_time_profiler_current_tick_gtod ();
#endif
}

static puint64
pp_time_profiler_current_tick_gtod ()
{
	struct timeval tv;

	if (P_UNLIKELY (gettimeofday (&tv, NULL) != 0)) {
		P_ERROR ("PTimeProfiler: Failed to get time using gettimeofday()");
		return 0;
	}

	return (puint64) (tv.tv_sec * 1000000 + tv.tv_usec);
}

P_LIB_API PTimeProfiler *
p_time_profiler_new ()
{
	PTimeProfiler	*ret;

	if (P_UNLIKELY ((ret = p_malloc0 (sizeof (PTimeProfiler))) == NULL))
		return NULL;

#if _POSIX_MONOTONIC_CLOCK == 0
	if (P_LIKELY (sysconf (_SC_MONOTONIC_CLOCK) > 0))
		ret->hasMonotonicClock = TRUE;
	else
		ret->hasMonotonicClock = FALSE;
#elif _POSIX_MONOTONIC_CLOCK > 0
	ret->hasMonotonicClock = TRUE;
#else
	ret->hasMonotonicClock = FALSE;
#endif

	ret->counter = pp_time_profiler_current_ticks (ret);

	return ret;
}

P_LIB_API void
p_time_profiler_reset (PTimeProfiler *profiler)
{
	if (P_UNLIKELY (profiler == NULL))
		return;

	profiler->counter = pp_time_profiler_current_ticks (profiler);
}

P_LIB_API puint64
p_time_profiler_elapsed_usecs (const PTimeProfiler *profiler)
{
	if (P_UNLIKELY (profiler == NULL))
		return 0;

	return pp_time_profiler_current_ticks (profiler) - profiler->counter;
}

P_LIB_API void
p_time_profiler_free (PTimeProfiler *profiler)
{
	if (P_UNLIKELY (profiler == NULL))
		return;

	p_free (profiler);
}
