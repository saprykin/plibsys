/*
 * The MIT License
 *
 * Copyright (C) 2010-2016 Alexander Saprykin <saprykin.spb@gmail.com>
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
 * @file pprocess.h
 * @brief Process information
 * @author Alexander Saprykin
 *
 * A process is an executing unit in an operating system with its own address
 * space. Every process can be identified with a unique identifier called PID.
 * To get a PID of the currently running process call
 * p_process_get_current_pid(). To check whether a process with a given PID is
 * running up use p_process_is_running().
 */

#if !defined (PLIBSYS_H_INSIDE) && !defined (PLIBSYS_COMPILATION)
#  error "Header files shouldn't be included directly, consider using <plibsys.h> instead."
#endif

#ifndef PLIBSYS_HEADER_PPROCESS_H
#define PLIBSYS_HEADER_PPROCESS_H

#include <pmacros.h>
#include <ptypes.h>

P_BEGIN_DECLS

/**
 * @brief Gets a PID of the calling process.
 * @return PID of the calling process.
 * @since 0.0.1
 */
P_LIB_API puint32	p_process_get_current_pid	(void);

/**
 * @brief Checks whether a process with a given PID is running or not.
 * @param pid PID to check for.
 * @return TRUE if the process with the given PID exists and is running up,
 * FALSE otherwise.
 * @since 0.0.1
 */
P_LIB_API pboolean	p_process_is_running		(puint32 pid);

P_END_DECLS

#endif /* PLIBSYS_HEADER_PPROCESS_H */

