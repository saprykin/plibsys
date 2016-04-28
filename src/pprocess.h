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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA.
 */

/**
 * @file pprocess.h
 * @brief Process related routines
 * @author Alexander Saprykin
 */

#if !defined (__PLIBSYS_H_INSIDE__) && !defined (PLIBSYS_COMPILATION)
#  error "Header files shouldn't be included directly, consider using <plibsys.h> instead."
#endif

#ifndef __PPROCESS_H__
#define __PPROCESS_H__

#include <pmacros.h>
#include <ptypes.h>

P_BEGIN_DECLS

/**
 * @brief Gets PID of calling process.
 * @return PID of the currently calling process.
 * @since 0.0.1
 */
P_LIB_API puint32	p_process_get_current_pid	(void);

/**
 * @brief Checks wheather process with given PID is running or not.
 * @param pid PID to check for.
 * @return TRUE if process with given PID exists and is running, FALSE otherwise.
 * @since 0.0.1
 */
P_LIB_API pboolean	p_process_is_running		(puint32 pid);

P_END_DECLS

#endif /* __PPROCESS_H__ */

