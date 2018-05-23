/*
 * The MIT License
 *
 * Copyright (C) 2017 Alexander Saprykin <saprykin.spb@gmail.com>
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

#include <proto/timer.h>

static puint64 pp_time_profiler_freq = 1;

puint64
p_time_profiler_get_ticks_internal ()
{
	struct EClockVal eclock;

	ITimer->ReadEClock (&eclock);

	return (((puint64) eclock.ev_hi) * pp_time_profiler_freq + (puint64) eclock.ev_lo);
}

puint64
p_time_profiler_elapsed_usecs_internal (const PTimeProfiler *profiler)
{
	puint64 value;

	value = p_time_profiler_get_ticks_internal ();

	/* Check for register overflow */

	if (P_UNLIKELY (value < profiler->counter))
		value += (((puint64) 1) << 32) * pp_time_profiler_freq;

	return (value - profiler->counter) * 1000000ULL / pp_time_profiler_freq;
}

void
p_time_profiler_init (void)
{
	struct EClockVal eclock;

	pp_time_profiler_freq = (puint64) ITimer->ReadEClock (&eclock);
}

void
p_time_profiler_shutdown (void)
{
	pp_time_profiler_freq = 1;
}
