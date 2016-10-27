/*
 * Copyright (C) 2016 Alexander Saprykin <xelfium@gmail.com>
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, see <http://www.gnu.org/licenses/>.
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
