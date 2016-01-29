/*
 * Copyright (C) 2015-2016 Alexander Saprykin <xelfium@gmail.com>
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

#include <windows.h>

struct _PTimeProfiler {
	puint64		counter;
	pboolean	hasPerformanceCounter;
};

static puint64 __p_time_profiler_current_ticks (const PTimeProfiler *profiler);

static puint64
__p_time_profiler_current_ticks (const PTimeProfiler *profiler)
{
	LARGE_INTEGER	tcounter;

	if (profiler == NULL)
		return 0;

	if (profiler->hasPerformanceCounter) {
		if (QueryPerformanceCounter (&tcounter) == FALSE) {
			P_ERROR ("PTimeProfiler: Failed to get current ticks count");
			tcounter.QuadPart = 0;
		}

		return (puint64) tcounter.QuadPart;
	} else {
#ifdef PLIB_HAS_GETTICKCOUNT_64
	return (puint64) GetTickCount64 ();
#else
	return (puint64) GetTickCount ();
#endif
	}
}

P_LIB_API PTimeProfiler *
p_time_profiler_new ()
{
	PTimeProfiler	*ret;
	LARGE_INTEGER	tcounter;

	if ((ret = p_malloc0 (sizeof (PTimeProfiler))) == NULL)
		return NULL;

	if (QueryPerformanceCounter (&tcounter) == FALSE || tcounter.QuadPart == 0)
		ret->hasPerformanceCounter = FALSE;
	else
		ret->hasPerformanceCounter = TRUE;

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
	LARGE_INTEGER	frequency;
#ifndef PLIB_HAS_GETTICKCOUNT_64
	puint64		val;
	puint64		high_bit;
#endif

	if (profiler == NULL)
		return 0;

	if (profiler->hasPerformanceCounter) {
		if (QueryPerformanceFrequency (&frequency) == FALSE) {
			P_ERROR ("PTimeProfiler: Failed to get performance frequency, fallback to 1000000 Hz");
			frequency.QuadPart = 1000000;
		}

		return (puint64) ((double)(__p_time_profiler_current_ticks (profiler) - profiler->counter) /
				  (frequency.QuadPart / 1000000.0F));
	} else {
#ifdef PLIB_HAS_GETTICKCOUNT_64
	return (puint64) (GetTickCount64 () - profiler->counter) * 1000;
#else
	high_bit = 0;
	val = (puint64) GetTickCount ();
	if (val < profiler->counter)
		high_bit = 1;

	return (val | (high_bit << 32)) - profiler->counter;
#endif
	}
}

P_LIB_API void
p_time_profiler_free (PTimeProfiler *profiler)
{
	if (profiler == NULL)
		return;

	p_free (profiler);
}
