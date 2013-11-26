/*
 * Copyright (C) 2013 Alexander Saprykin <xelfium@gmail.com>
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

#include "ptimeprofiler.h"
#include "pmem.h"

#include <time.h>
#include <sys/time.h>

struct _PTimeProfiler {
	puint64		counter;
	pboolean	hasMonotonicClock;
};

static puint64 __p_time_profiler_current_ticks (const PTimeProfiler *profiler);
static puint64 __p_time_profiler_current_tick_gtod ();

static puint64
__p_time_profiler_current_ticks (const PTimeProfiler *profiler)
{
#if _POSIX_MONOTONIC_CLOCK >= 0
	struct timespec	ts;
#endif

	if (profiler == NULL)
		return 0;

#if _POSIX_MONOTONIC_CLOCK >= 0
	if (profiler->hasMonotonicClock) {
		if (clock_gettime (CLOCK_MONOTONIC, &ts) != 0) {
			P_ERROR ("PTimeProfiler: Failed to get time using clock_gettime()");
			return __p_time_profiler_current_tick_gtod ();
		} else
			return (puint64) (ts.tv_sec * 1000000 + ts.tv_nsec / 1000);
	} else
		return __p_time_profiler_current_tick_gtod ();
#else
	return __p_time_profiler_current_tick_gtod ();
#endif
}

static puint64
__p_time_profiler_current_tick_gtod ()
{
	struct timeval tv;

	if (gettimeofday (&tv, NULL) != 0) {
		P_ERROR ("PTimeProfiler: Failed to get time using gettimeofday()");
		return 0;
	}

	return (puint64) (tv.tv_sec * 1000000 + tv.tv_usec);
}

P_LIB_API PTimeProfiler *
p_time_profiler_new ()
{
	PTimeProfiler	*ret;


	if ((ret = p_malloc0 (sizeof (PTimeProfiler))) == NULL)
		return NULL;

#if _POSIX_MONOTONIC_CLOCK == 0
	if (sysconf (_SC_MONOTONIC_CLOCK) > 0)
		ret->hasMonotonicClock = TRUE;
	else
		ret->hasMonotonicClock = FALSE;
#elif _POSIX_MONOTONIC_CLOCK > 0
	ret->hasMonotonicClock = TRUE;
#else
	ret->hasMonotonicClock = FALSE;
#endif

	ret->counter = __p_time_profiler_current_ticks (ret);

	return ret;
}

P_LIB_API void
p_time_profiler_reset (PTimeProfiler *profiler)
{
	if (profiler == NULL)
		return;

	profiler->counter = __p_time_profiler_current_ticks (profiler);
}

P_LIB_API puint64
p_time_profiler_elapsed_usecs (const PTimeProfiler *profiler)
{
	if (profiler == NULL)
		return 0;

	return __p_time_profiler_current_ticks (profiler) - profiler->counter;
}

P_LIB_API void
p_time_profiler_free (PTimeProfiler *profiler)
{
	if (profiler == NULL)
		return;

	p_free (profiler);
}
