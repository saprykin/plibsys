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

#define INCL_DOSPROFILE
#define INCL_DOSERRORS
#include <os2.h>

#if PLIBSYS_HAS_LLDIV
#  ifdef P_CC_GNU
#    define __USE_ISOC99
#  endif
#  include <stdlib.h>
#endif

static puint64 pp_time_profiler_freq = 1;

puint64
p_time_profiler_get_ticks_internal ()
{
	union {
		puint64	ticks;
		QWORD	tcounter;
	} tick_time;

	if (P_UNLIKELY (DosTmrQueryTime (&tick_time.tcounter) != NO_ERROR)) {
		P_ERROR ("PTimeProfiler::p_time_profiler_get_ticks_internal: DosTmrQueryTime() failed");
		return 0;
	}

	return tick_time.ticks;
}

puint64
p_time_profiler_elapsed_usecs_internal (const PTimeProfiler *profiler)
{
	puint64	ticks;
#if PLIBSYS_HAS_LLDIV
	lldiv_t	ldres;
#endif
	puint64	quot;
	puint64	rem;

	ticks = p_time_profiler_get_ticks_internal ();

	if (ticks < profiler->counter) {
		P_WARNING ("PTimeProfiler::p_time_profiler_elapsed_usecs_internal: negative jitter");
		return 1;
	}

	ticks -= profiler->counter;

#if PLIBSYS_HAS_LLDIV
	ldres = lldiv ((long long) ticks, (long long) pp_time_profiler_freq);

	quot = ldres.quot;
	rem  = ldres.rem;
#else
	quot = ticks / pp_time_profiler_freq;
	rem  = ticks % pp_time_profiler_freq;
#endif

	return (puint64) (quot * 1000000LL + (rem * 1000000LL) / pp_time_profiler_freq);
}

void
p_time_profiler_init (void)
{
	ULONG freq;

	if (P_UNLIKELY (DosTmrQueryFreq (&freq) != NO_ERROR)) {
		P_ERROR ("PTimeProfiler::p_time_profiler_init: DosTmrQueryFreq() failed");
		return;
	}

	pp_time_profiler_freq = (puint64) freq;
}

void
p_time_profiler_shutdown (void)
{
	pp_time_profiler_freq = 1;
}
