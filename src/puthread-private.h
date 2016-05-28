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

#if !defined (PLIBSYS_H_INSIDE) && !defined (PLIBSYS_COMPILATION)
#  error "Header files shouldn't be included directly, consider using <plibsys.h> instead."
#endif

#ifndef PLIBSYS_HEADER_PUTHREAD_PRIVATE_H
#define PLIBSYS_HEADER_PUTHREAD_PRIVATE_H

#include "pmacros.h"
#include "ptypes.h"
#include "puthread.h"

P_BEGIN_DECLS

/** Base thread structure */
typedef struct PUThreadBase_ {
	pint			ref_count;	/**< Reference counter.	*/
	pint			ret_code;	/**< Return code.	*/
	pboolean		ours;		/**< Our thread flag.	*/
	pboolean		joinable;	/**< Joinable flag.	*/
	PUThreadFunc		func;		/**< Thread routine.	*/
	ppointer		data;		/**< Thread input data.	*/
	PUThreadPriority	prio;		/**< Thread priority.	*/
} PUThreadBase;

P_END_DECLS

#endif /* PLIBSYS_HEADER_PUTHREAD_PRIVATE_H */
