/*
 * Copyright (C) 2010-2016 Alexander Saprykin <xelfium@gmail.com>
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
 * @file pmain.h
 * @brief PLibSYS initialization/shutdown
 * @author Alexander Saprykin
 *
 * In order to use PLibSYS correctly you must initialize it once per
 * application using p_libsys_init() routine. It will initialize required
 * resources, mutexes and other stuff. On exit you should free used
 * resources by calling p_libsys_shutdown() routine.
 */

#if !defined (__PLIBSYS_H_INSIDE__) && !defined (PLIBSYS_COMPILATION)
#  error "Header files shouldn't be included directly, consider using <plibsys.h> instead."
#endif

#ifndef __PMAIN_H__
#define __PMAIN_H__

#include <pmacros.h>

P_BEGIN_DECLS

/**
 * @brief Initializes PLibSYS resources.
 * @since 0.0.1
 */
P_LIB_API void	p_libsys_init		(void);

/**
 * @brief Frees PLibSYS resources. You shouldn't use PLibSYS routines
 * after calling this one.
 * @since 0.0.1
 */
P_LIB_API void	p_libsys_shutdown	(void);

P_END_DECLS

#endif /* __PMAIN_H__ */
