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
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

#include "ptimeprofiler.h"
#include "ptimeprofiler-private.h"

#include <time.h>

#include <windows.h>

typedef puint64 (* PWin32TicksFunc) (void);
typedef puint64 (* PWin32ElapsedFunc) (puint64 last_counter);

static PWin32TicksFunc   pp_time_profiler_ticks_func   = NULL;
static PWin32ElapsedFunc pp_time_profiler_elapsed_func = NULL;
static puint64           pp_time_profiler_freq         = 0;

static puint64 pp_time_profiler_get_hr_ticks (void);
static puint64 pp_time_profiler_elapsed_hr (puint64 last_counter);
static puint64 pp_time_profiler_elapsed_tick64 (puint64 last_counter);
static puint64 pp_time_profiler_elapsed_tick (puint64 last_counter);

static puint64
pp_time_profiler_get_hr_ticks (void)
{
	LARGE_INTEGER tcounter;

	if (P_UNLIKELY (QueryPerformanceCounter (&tcounter) == FALSE)) {
		P_ERROR ("PTimeProfiler: failed to get current HR ticks count");
		tcounter.QuadPart = 0;
	}

	return (puint64) tcounter.QuadPart;
}

static puint64
pp_time_profiler_elapsed_hr (puint64 last_counter)
{
	return (pp_time_profiler_ticks_func () - last_counter) / pp_time_profiler_freq;
}

static puint64
pp_time_profiler_elapsed_tick64 (puint64 last_counter)
{
	return (pp_time_profiler_ticks_func () - last_counter) * 1000;
}

static puint64
pp_time_profiler_elapsed_tick (puint64 last_counter)
{
	puint64 val;
	puint64 high_bit;

	high_bit = 0;
	val      = pp_time_profiler_ticks_func ();
		
	if (P_UNLIKELY (val < last_counter))
		high_bit = 1;

	return (val | (high_bit << 32)) - last_counter;
}

puint64
p_time_profiler_get_ticks_internal ()
{
	return pp_time_profiler_ticks_func ();
}

puint64
p_time_profiler_elapsed_usecs_internal (const PTimeProfiler *profiler)
{
	return pp_time_profiler_elapsed_func (profiler->counter);
}

void
p_time_profiler_init (void)
{
	LARGE_INTEGER	tcounter;
	HMODULE		hmodule;
	pboolean	has_qpc;

	has_qpc = (QueryPerformanceCounter (&tcounter) != 0 && tcounter.QuadPart != 0) ? TRUE : FALSE;

	if (has_qpc == TRUE) {
		if (P_UNLIKELY (QueryPerformanceFrequency (&tcounter) == 0)) {
			P_ERROR ("PTimeProfiler: failed to call QueryPerformanceFrequency()");
			has_qpc = FALSE;
		} else {
			pp_time_profiler_freq         = (puint64) (tcounter.QuadPart / 1000000.0F);
			pp_time_profiler_ticks_func   = (PWin32TicksFunc) pp_time_profiler_get_hr_ticks;
			pp_time_profiler_elapsed_func = (PWin32ElapsedFunc) pp_time_profiler_elapsed_hr;
		}
	}

	if (P_UNLIKELY (has_qpc == FALSE)) {
		hmodule = GetModuleHandleA ("kernel32.dll");

		if (P_UNLIKELY (hmodule == NULL)) {
			P_ERROR ("PTimeProfiler: failed to load kernel32.dll module");
			return;
		}

		pp_time_profiler_ticks_func   = (PWin32TicksFunc) GetProcAddress (hmodule, "GetTickCount64");
		pp_time_profiler_elapsed_func = (PWin32ElapsedFunc) pp_time_profiler_elapsed_tick64;

		if (P_UNLIKELY (pp_time_profiler_ticks_func == NULL)) {
			pp_time_profiler_ticks_func   = (PWin32TicksFunc) GetProcAddress (hmodule, "GetTickCount");
			pp_time_profiler_elapsed_func = (PWin32ElapsedFunc) pp_time_profiler_elapsed_tick;
		}
		
		if (P_UNLIKELY (pp_time_profiler_ticks_func == NULL)) {
			P_ERROR ("PTimeProfiler: failed to load any reliable tick counter");
			pp_time_profiler_elapsed_func = NULL;
		}
	}
}

void
p_time_profiler_shutdown (void)
{
	pp_time_profiler_freq         = 0;
	pp_time_profiler_ticks_func   = NULL;
	pp_time_profiler_elapsed_func = NULL;
}
