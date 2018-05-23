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
