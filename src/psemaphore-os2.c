/*
 * The MIT License
 *
 * Copyright (C) 2017-2018 Alexander Saprykin <saprykin.spb@gmail.com>
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

#include "psemaphore.h"

#include <stdlib.h>

struct PSemaphore_ {
	pint	hdl;
};

P_LIB_API PSemaphore *
p_semaphore_new (const pchar		*name,
		 pint			init_val,
		 PSemaphoreAccessMode	mode,
		 PError			**error)
{
	P_UNUSED (name);
	P_UNUSED (init_val);
	P_UNUSED (mode);

	p_error_set_error_p (error,
			     (pint) P_ERROR_IPC_NOT_IMPLEMENTED,
			     0,
			     "No semaphore implementation");

	return NULL;
}

P_LIB_API void
p_semaphore_take_ownership (PSemaphore *sem)
{
	P_UNUSED (sem);
}

P_LIB_API pboolean
p_semaphore_acquire (PSemaphore	*sem,
		     PError	**error)
{
	P_UNUSED (sem);

	p_error_set_error_p (error,
			     (pint) P_ERROR_IPC_NOT_IMPLEMENTED,
			     0,
			     "No semaphore implementation");

	return FALSE;
}

P_LIB_API pboolean
p_semaphore_release (PSemaphore	*sem,
		     PError	**error)
{
	P_UNUSED (sem);

	p_error_set_error_p (error,
			     (pint) P_ERROR_IPC_NOT_IMPLEMENTED,
			     0,
			     "No semaphore implementation");

	return FALSE;
}

P_LIB_API void
p_semaphore_free (PSemaphore *sem)
{
	P_UNUSED (sem);
}

