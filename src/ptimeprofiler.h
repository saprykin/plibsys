/*
 * Copyright (C) 2013-2016 Alexander Saprykin <xelfium@gmail.com>
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

/**
 * @file ptimeprofiler.h
 * @brief Calculates elapsed time since fixed moment of time
 * @author Alexander Saprykin
 *
 * Use #PTimeProfiler to calculate elapsed time for time consuming
 * operations. This can help to leverage bottle-necks code parts.
 */

#if !defined (__PLIBSYS_H_INSIDE__) && !defined (PLIBSYS_COMPILATION)
#  error "Header files shouldn't be included directly, consider using <plibsys.h> instead."
#endif

#ifndef __PTIMEPROFILER_H__
#define __PTIMEPROFILER_H__

#include <pmacros.h>
#include <ptypes.h>

/** Time profiler opaque data structure */
typedef struct _PTimeProfiler PTimeProfiler;

P_BEGIN_DECLS

/**
 * @brief Creates new #PTimeProfiler object.
 * @return Pointer to newly created #PTimeProfiler object.
 * @since 0.0.1
 */
P_LIB_API PTimeProfiler *	p_time_profiler_new		(void);

/**
 * @brief Resets #PTimeProfiler internal counter to zero.
 * @param profiler Time profiler to reset.
 * @since 0.0.1
 *
 * After reset time profiler begins to count elapsed time from
 * that moment of time.
 */
P_LIB_API void			p_time_profiler_reset		(PTimeProfiler *	profiler);

/**
 * @brief Calculates elapsed time since last reset or creation.
 * @param profiler Time profiler to calculate elapsed time from.
 * @return Microseconds elapsed since last reset or creation.
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

#endif /* __PTIMEPROFILER_H__ */
