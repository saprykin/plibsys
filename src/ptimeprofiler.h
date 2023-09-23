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

/**
 * @file ptimeprofiler.h
 * @brief Time profiler
 * @author Alexander Saprykin
 *
 * #PTimeProfiler acts like a time chronometer: in any moment of time you can
 * make a time slice to see how much time elapsed since the last slice or timer
 * start.
 *
 * This profiler is useful to gather information about execution time for calls
 * or parts of the code. It can help to leverage bottle-necks in your code.
 *
 * To start using a profiler create a new one with p_time_profiler_new() call
 * and p_time_profiler_elapsed_usecs() to get elapsed time since the creation.
 * If you need to reset a profiler use p_time_profiler_reset(). Remove a
 * profiler with p_time_profiler_free().
 */

#if !defined (PLIBSYS_H_INSIDE) && !defined (PLIBSYS_COMPILATION)
#  error "Header files shouldn't be included directly, consider using <plibsys.h> instead."
#endif

#ifndef PLIBSYS_HEADER_PTIMEPROFILER_H
#define PLIBSYS_HEADER_PTIMEPROFILER_H

#include <pmacros.h>
#include <ptypes.h>

P_BEGIN_DECLS

/** Time profiler opaque data structure. */
typedef struct PTimeProfiler_ PTimeProfiler;

/**
 * @brief Creates a new #PTimeProfiler object.
 * @return Pointer to a newly created #PTimeProfiler object.
 * @since 0.0.1
 */
P_LIB_API PTimeProfiler *	p_time_profiler_new		(void);

/**
 * @brief Resets the #PTimeProfiler's internal counter to zero.
 * @param profiler Time profiler to reset.
 * @since 0.0.1
 *
 * After a reset the time profiler begins to count elapsed time from that moment
 * of time.
 */
P_LIB_API void			p_time_profiler_reset		(PTimeProfiler *	profiler);

/**
 * @brief Calculates elapsed time since the last reset or creation.
 * @param profiler Time profiler to calculate elapsed time for.
 * @return Microseconds elapsed since the last reset or creation.
 * @since 0.0.1
 */
P_LIB_API puint64		p_time_profiler_elapsed_usecs	(const PTimeProfiler *	profiler);

/**
 * @brief Frees #PTimeProfiler object.
 * @param profiler #PTimeProfiler to free.
 * @since 0.0.1
 */
P_LIB_API void			p_time_profiler_free		(PTimeProfiler *	profiler);

P_END_DECLS

#endif /* PLIBSYS_HEADER_PTIMEPROFILER_H */
