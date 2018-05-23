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
		P_ERROR ("PTimeProfiler::p_time_profiler_init: mach_timebase_info() failed");
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
