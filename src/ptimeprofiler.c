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

#ifdef P_OS_WIN
#  include <windows.h>
#else
#  include <unistd.h>
#endif

struct _PTimeProfiler {
	pint64 counter;
};

static pint64 __p_time_profiler_current_ticks ();

static pint64
__p_time_profiler_current_ticks ()
{
#ifdef P_OS_WIN
	LARGE_INTEGER	tcounter;

	if (QueryPerformanceCounter (&tcounter) == FALSE)
		tcounter.QuadPart = 0;

	return (pint64) tcounter.QuadPart;
#else
#  if defined (_POSIX_TIMERS) && (_POSIX_TIMERS > 0)
	/* So, we have POSIX timers, not bad */
	struct timespec	ts;

#    ifdef _POSIX_MONOTONIC_CLOCK
	clock_gettime (CLOCK_MONOTONIC, &ts);
#    else
	clock_gettime (CLOCK_REALTIME, &ts);
#    endif

	return (pint64) ts.tv_sec * 1000000 + ts.tv_nsec / 1000;
#  else
	/* Bad case, we have no viable clock source */
	return 0;
#  endif
#endif
}

P_LIB_API PTimeProfiler *
p_time_profiler_new ()
{
	PTimeProfiler *ret;

	if ((ret = p_malloc0 (sizeof (PTimeProfiler))) == NULL)
		return NULL;

	ret->counter = __p_time_profiler_current_ticks ();

	return ret;
}

P_LIB_API void
p_time_profiler_reset (PTimeProfiler *profiler)
{
	if (profiler == NULL)
		return;

	profiler->counter = __p_time_profiler_current_ticks ();
}

P_LIB_API pint64
p_time_profiler_elapsed_usecs (PTimeProfiler *profiler)
{
#ifdef P_OS_WIN
	LARGE_INTEGER frequency;
#endif

	if (profiler == NULL)
		return 0;

#ifdef P_OS_WIN
	if (QueryPerformanceFrequency (&frequency) == FALSE)
		frequency.QuadPart = 1000000;

	return (pint64) ((double)(__p_time_profiler_current_ticks () - profiler->counter) /
			 (frequency.QuadPart / 1000000.0F));
#else
	return __p_time_profiler_current_ticks () - profiler->counter;
#endif
}

P_LIB_API void
p_time_profiler_free (PTimeProfiler *profiler)
{
	if (profiler == NULL)
		return;

	p_free (profiler);
}
