/*
 * The MIT License
 *
 * Copyright (C) 2017-2023 Alexander Saprykin <saprykin.spb@gmail.com>
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

#include <exec/types.h>
#include <exec/memory.h>
#include <devices/timer.h>
#include <interfaces/timer.h>

#include <proto/dos.h>
#include <proto/exec.h>

static struct MsgPort		*pp_port              = NULL;
static struct TimeRequest	*pp_timer_io          = NULL;
static struct TimerIFace	*pp_ITimer            = NULL;
static puint64 			pp_time_profiler_freq = 1;

static void
pp_time_profiler_close_device (void)
{
	if (P_LIKELY (pp_ITimer != NULL)) {
		IExec->DropInterface ((struct Interface *) pp_ITimer);
		pp_ITimer = NULL;
	}

	if (P_LIKELY (pp_timer_io != NULL)) {
		IExec->CloseDevice ((struct IORequest *) pp_timer_io);
		IExec->FreeSysObject (ASOT_IOREQUEST, pp_timer_io);
		IExec->FreeSysObject (ASOT_PORT, pp_port);

		pp_timer_io = NULL;
		pp_port     = NULL;
	}

	pp_time_profiler_freq = 1;
}

puint64
p_time_profiler_get_ticks_internal ()
{
	struct EClockVal eclock;

	if (P_UNLIKELY (pp_ITimer == NULL)) {
		P_ERROR ("PTimeProfiler::p_time_profiler_get_ticks_internal: timer subsystem is not initialized");
		return 0;
	}

	pp_ITimer->ReadEClock (&eclock);

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

	pp_port = IExec->AllocSysObjectTags (ASOT_PORT, TAG_END);
	pp_timer_io = IExec->AllocSysObjectTags (ASOT_IOREQUEST,
						 ASOIOR_Size, sizeof (struct TimeRequest),
						 ASOIOR_ReplyPort, pp_port,
						 TAG_END);

	if (P_UNLIKELY (IExec->OpenDevice (TIMERNAME, UNIT_ECLOCK, (struct IORequest *) pp_timer_io, 0L) != 0)) {
		P_ERROR ("PTimeProfiler::p_time_profiler_init: failed to open timer device");
		IExec->FreeSysObject (ASOT_IOREQUEST, pp_timer_io);
		IExec->FreeSysObject (ASOT_PORT, pp_port);
		return;
	}

	pp_ITimer = (struct TimerIFace *) IExec->GetInterface ((struct Library *) pp_timer_io->Request.io_Device, "main", 1, NULL);

	if (P_UNLIKELY (pp_ITimer == NULL)) {
		P_ERROR ("PTimeProfiler::p_time_profiler_init: failed to get timer interface");
		pp_time_profiler_close_device ();	
		return;
	}

	pp_time_profiler_freq = (puint64) pp_ITimer->ReadEClock (&eclock);
}

void
p_time_profiler_shutdown (void)
{
	pp_time_profiler_close_device ();
}
