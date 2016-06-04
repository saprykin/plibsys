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

#include "ptimeprofiler.h"
#include "ptimeprofiler-private.h"

#include <unistd.h>
#include <time.h>
#include <sys/time.h>

#ifndef _POSIX_MONOTONIC_CLOCK
#  define _POSIX_MONOTONIC_CLOCK (-1)
#endif

typedef puint64 (* PPOSIXTicksFunc) (void);

static PPOSIXTicksFunc pp_time_profiler_ticks_func = NULL;

#if (_POSIX_MONOTONIC_CLOCK >= 0) || defined (P_OS_IRIX)
static puint64 pp_time_profiler_get_ticks_clock ();
#endif

static puint64 pp_time_profiler_get_ticks_gtod ();

#if (_POSIX_MONOTONIC_CLOCK >= 0) || defined (P_OS_IRIX)
static puint64
pp_time_profiler_get_ticks_clock ()
{
	struct timespec	ts;

#ifdef P_OS_IRIX
	if (P_UNLIKELY (clock_gettime (CLOCK_SGI_CYCLE, &ts) != 0)) {
#else
	if (P_UNLIKELY (clock_gettime (CLOCK_MONOTONIC, &ts) != 0)) {
#endif
		P_ERROR ("PTimeProfiler: failed to get time using clock_gettime()");
		return pp_time_profiler_get_ticks_gtod ();
	} else
		return (puint64) (ts.tv_sec * 1000000 + ts.tv_nsec / 1000);
}
#endif

static puint64
pp_time_profiler_get_ticks_gtod ()
{
	struct timeval tv;

	if (P_UNLIKELY (gettimeofday (&tv, NULL) != 0)) {
		P_ERROR ("PTimeProfiler: failed to get time using gettimeofday()");
		return 0;
	}

	return (puint64) (tv.tv_sec * 1000000 + tv.tv_usec);
}

puint64
p_time_profiler_get_ticks_internal ()
{
	return pp_time_profiler_ticks_func ();
}

puint64
p_time_profiler_elapsed_usecs_internal (const PTimeProfiler *profiler)
{
	return pp_time_profiler_ticks_func () - profiler->counter;
}

void
p_time_profiler_init (void)
{
#ifdef P_OS_IRIX
	pp_time_profiler_ticks_func = (PPOSIXTicksFunc) pp_time_profiler_get_ticks_clock;
#elif (_POSIX_MONOTONIC_CLOCK == 0) && defined (_SC_MONOTONIC_CLOCK)
	if (P_LIKELY (sysconf (_SC_MONOTONIC_CLOCK) > 0))
		pp_time_profiler_ticks_func = (PPOSIXTicksFunc) pp_time_profiler_get_ticks_clock;
	else
		pp_time_profiler_ticks_func = (PPOSIXTicksFunc) pp_time_profiler_get_ticks_gtod;
#elif _POSIX_MONOTONIC_CLOCK > 0
	pp_time_profiler_ticks_func = (PPOSIXTicksFunc) pp_time_profiler_get_ticks_clock;
#else
	pp_time_profiler_ticks_func = (PPOSIXTicksFunc) pp_time_profiler_get_ticks_gtod;
#endif
}

void
p_time_profiler_shutdown (void)
{
	pp_time_profiler_ticks_func = NULL;
}
