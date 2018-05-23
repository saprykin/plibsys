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

#if !defined (PLIBSYS_H_INSIDE) && !defined (PLIBSYS_COMPILATION)
#  error "Header files shouldn't be included directly, consider using <plibsys.h> instead."
#endif

#ifndef PLIBSYS_HEADER_PERROR_PRIVATE_H
#define PLIBSYS_HEADER_PERROR_PRIVATE_H

#include "pmacros.h"
#include "ptypes.h"
#include "perrortypes.h"

P_BEGIN_DECLS

/**
 * @brief Gets an IO error code from a system error code.
 * @param err_code System error code.
 * @return IO error code.
 */
PErrorIO	p_error_get_io_from_system	(pint	err_code);

/**
 * @brief Gets an IO error code from the last call result.
 * @return IO error code.
 */
PErrorIO	p_error_get_last_io		(void);

/**
 * @brief Gets an IPC error code from a system error code
 * @param err_code System error code.
 * @return IPC error code.
 */
PErrorIPC	p_error_get_ipc_from_system	(pint	err_code);

/**
 * @brief Gets an IPC error code from the last call result.
 * @return IPC error code.
 */
PErrorIPC	p_error_get_last_ipc		(void);

P_END_DECLS

#endif /* PLIBSYS_HEADER_PERROR_PRIVATE_H */
