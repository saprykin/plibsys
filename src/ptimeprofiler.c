/*
 * The MIT License
 *
 * Copyright (C) 2016 Alexander Saprykin <saprykin.spb@gmail.com>
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

#include "pmem.h"
#include "ptimeprofiler.h"
#include "ptimeprofiler-private.h"

extern puint64 p_time_profiler_get_ticks_internal (void);
extern puint64 p_time_profiler_elapsed_usecs_internal (const PTimeProfiler *profiler);

P_LIB_API PTimeProfiler *
p_time_profiler_new ()
{
	PTimeProfiler *ret;

	if (P_UNLIKELY ((ret = p_malloc0 (sizeof (PTimeProfiler))) == NULL)) {
		P_ERROR ("PTimeProfiler: failed to allocate memory");
		return NULL;
	}

	ret->counter = p_time_profiler_get_ticks_internal ();

	return ret;
}

P_LIB_API void
p_time_profiler_reset (PTimeProfiler *profiler)
{
	if (P_UNLIKELY (profiler == NULL))
		return;

	profiler->counter = p_time_profiler_get_ticks_internal ();
}

P_LIB_API puint64
p_time_profiler_elapsed_usecs (const PTimeProfiler *profiler)
{
	if (P_UNLIKELY (profiler == NULL))
		return 0;

	return p_time_profiler_elapsed_usecs_internal (profiler);
}

P_LIB_API void
p_time_profiler_free (PTimeProfiler *profiler)
{
	p_free (profiler);
}
