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
