/*
 * The MIT License
 *
 * Copyright (C) 2013-2016 Alexander Saprykin <saprykin.spb@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * 'Software'), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED 'AS IS', WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
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
		P_ERROR ("PTimeProfiler::pp_time_profiler_get_ticks_clock: clock_gettime() failed");
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
		P_ERROR ("PTimeProfiler::pp_time_profiler_get_ticks_gtod: gettimeofday() failed");
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
#if defined (P_OS_IRIX) || (_POSIX_MONOTONIC_CLOCK > 0)
	pp_time_profiler_ticks_func = (PPOSIXTicksFunc) pp_time_profiler_get_ticks_clock;
#elif (_POSIX_MONOTONIC_CLOCK == 0) && defined (_SC_MONOTONIC_CLOCK)
	if (P_LIKELY (sysconf (_SC_MONOTONIC_CLOCK) > 0))
		pp_time_profiler_ticks_func = (PPOSIXTicksFunc) pp_time_profiler_get_ticks_clock;
	else
		pp_time_profiler_ticks_func = (PPOSIXTicksFunc) pp_time_profiler_get_ticks_gtod;
#else
	pp_time_profiler_ticks_func = (PPOSIXTicksFunc) pp_time_profiler_get_ticks_gtod;
#endif
}

void
p_time_profiler_shutdown (void)
{
	pp_time_profiler_ticks_func = NULL;
}
