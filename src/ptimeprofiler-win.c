/*
 * The MIT License
 *
 * Copyright (C) 2015-2017 Alexander Saprykin <saprykin.spb@gmail.com>
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

/* https://msdn.microsoft.com/ru-ru/library/windows/desktop/dn553408(v=vs.85).aspx */

#include "ptimeprofiler.h"
#include "ptimeprofiler-private.h"

#include <time.h>

#if PLIBSYS_HAS_LLDIV
#  include <stdlib.h>
#endif

typedef puint64 (WINAPI * PWin32TicksFunc) (void);
typedef puint64 (* PWin32ElapsedFunc) (puint64 last_counter);

static PWin32TicksFunc   pp_time_profiler_ticks_func   = NULL;
static PWin32ElapsedFunc pp_time_profiler_elapsed_func = NULL;
static puint64           pp_time_profiler_freq         = 1;

static puint64 WINAPI pp_time_profiler_get_hr_ticks (void);
static puint64 pp_time_profiler_elapsed_hr (puint64 last_counter);
static puint64 pp_time_profiler_elapsed_tick64 (puint64 last_counter);
static puint64 pp_time_profiler_elapsed_tick (puint64 last_counter);

static puint64 WINAPI
pp_time_profiler_get_hr_ticks (void)
{
	LARGE_INTEGER tcounter;

	if (P_UNLIKELY (QueryPerformanceCounter (&tcounter) == FALSE)) {
		P_ERROR ("PTimeProfiler::pp_time_profiler_get_hr_ticks: QueryPerformanceCounter() failed");
		tcounter.QuadPart = 0;
	}

	return (puint64) tcounter.QuadPart;
}

static puint64
pp_time_profiler_elapsed_hr (puint64 last_counter)
{
	puint64	ticks;
#ifdef PLIBSYS_HAS_LLDIV
	lldiv_t	ldres;
#endif
	puint64	quot;
	puint64	rem;

	ticks = pp_time_profiler_ticks_func () - last_counter;

#ifdef PLIBSYS_HAS_LLDIV
	ldres = lldiv ((long long) ticks, (long long) pp_time_profiler_freq);

	quot = ldres.quot;
	rem  = ldres.rem;
#else
	quot = ticks / pp_time_profiler_freq;
	rem  = ticks % pp_time_profiler_freq;
#endif

	return (puint64) (quot * 1000000 + (rem * 1000000) / pp_time_profiler_freq);
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

	return ((val | (high_bit << 32)) - last_counter) * 1000;
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
			P_ERROR ("PTimeProfiler::p_time_profiler_init: QueryPerformanceFrequency() failed");
			has_qpc = FALSE;
		} else {
			pp_time_profiler_freq         = (puint64) (tcounter.QuadPart);
			pp_time_profiler_ticks_func   = (PWin32TicksFunc) pp_time_profiler_get_hr_ticks;
			pp_time_profiler_elapsed_func = (PWin32ElapsedFunc) pp_time_profiler_elapsed_hr;
		}
	}

	if (P_UNLIKELY (has_qpc == FALSE)) {
		hmodule = GetModuleHandleA ("kernel32.dll");

		if (P_UNLIKELY (hmodule == NULL)) {
			P_ERROR ("PTimeProfiler::p_time_profiler_init: failed to load kernel32.dll module");
			return;
		}

		pp_time_profiler_ticks_func   = (PWin32TicksFunc) GetProcAddress (hmodule, "GetTickCount64");
		pp_time_profiler_elapsed_func = (PWin32ElapsedFunc) pp_time_profiler_elapsed_tick64;

		if (P_UNLIKELY (pp_time_profiler_ticks_func == NULL)) {
			pp_time_profiler_ticks_func   = (PWin32TicksFunc) GetProcAddress (hmodule, "GetTickCount");
			pp_time_profiler_elapsed_func = (PWin32ElapsedFunc) pp_time_profiler_elapsed_tick;
		}

		if (P_UNLIKELY (pp_time_profiler_ticks_func == NULL)) {
			P_ERROR ("PTimeProfiler::p_time_profiler_init: no reliable tick counter");
			pp_time_profiler_elapsed_func = NULL;
		}
	}
}

void
p_time_profiler_shutdown (void)
{
	pp_time_profiler_freq         = 1;
	pp_time_profiler_ticks_func   = NULL;
	pp_time_profiler_elapsed_func = NULL;
}
