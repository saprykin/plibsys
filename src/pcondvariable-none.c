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

#include "pcondvariable.h"

#include <stdlib.h>

struct PCondVariable_ {
	pint hdl;
};

P_LIB_API PCondVariable *
p_cond_variable_new (void)
{
	return NULL;
}

P_LIB_API void
p_cond_variable_free (PCondVariable *cond)
{
	P_UNUSED (cond);
}

P_LIB_API pboolean
p_cond_variable_wait (PCondVariable	*cond,
		      PMutex		*mutex)
{
	P_UNUSED (cond);
	P_UNUSED (mutex);

	return FALSE;
}

P_LIB_API pboolean
p_cond_variable_signal (PCondVariable *cond)
{
	P_UNUSED (cond);

	return FALSE;
}

P_LIB_API pboolean
p_cond_variable_broadcast (PCondVariable *cond)
{
	P_UNUSED (cond);

	return FALSE;
}

void
p_cond_variable_init (void)
{
}

void
p_cond_variable_shutdown (void)
{
}
