/*
 * Copyright (C) 2016 Alexander Saprykin <xelfium@gmail.com>
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
#include "pmem.h"

#include <unistd.h>
#include <time.h>
#include <sys/time.h>

struct _PTimeProfiler {
	puint64	counter;
};

P_LIB_API PTimeProfiler *
p_time_profiler_new ()
{
	PTimeProfiler	*ret;

	if ((ret = p_malloc0 (sizeof (PTimeProfiler))) == NULL)
		return NULL;

	ret->counter = (puint64) gethrtime ();

	return ret;
}

P_LIB_API void
p_time_profiler_reset (PTimeProfiler *profiler)
{
	if (profiler == NULL)
		return;

	profiler->counter = (puint64) gethrtime ();
}

P_LIB_API puint64
p_time_profiler_elapsed_usecs (const PTimeProfiler *profiler)
{
	if (profiler == NULL)
		return 0;

	return (((puint64) gethrtime ()) - profiler->counter) / 1000;
}

P_LIB_API void
p_time_profiler_free (PTimeProfiler *profiler)
{
	if (profiler == NULL)
		return;

	p_free (profiler);
}
