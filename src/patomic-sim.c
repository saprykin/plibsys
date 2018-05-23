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

#include "patomic.h"
#include "pmutex.h"

/* We have to use the slow, but safe locking method. */
static PMutex *pp_atomic_mutex = NULL;

P_LIB_API pint
p_atomic_int_get (const volatile pint *atomic)
{
	pint value;

	p_mutex_lock (pp_atomic_mutex);
	value = *atomic;
	p_mutex_unlock (pp_atomic_mutex);

	return value;
}

P_LIB_API void
p_atomic_int_set (volatile pint	*atomic,
		  pint		val)
{
	p_mutex_lock (pp_atomic_mutex);
	*atomic = val;
	p_mutex_unlock (pp_atomic_mutex);
}

P_LIB_API void
p_atomic_int_inc (volatile pint *atomic)
{
	p_mutex_lock (pp_atomic_mutex);
	(*atomic)++;
	p_mutex_unlock (pp_atomic_mutex);
}

P_LIB_API pboolean
p_atomic_int_dec_and_test (volatile pint *atomic)
{
	pboolean is_zero;

	p_mutex_lock (pp_atomic_mutex);
	is_zero = --(*atomic) == 0;
	p_mutex_unlock (pp_atomic_mutex);

	return is_zero;
}

P_LIB_API pboolean
p_atomic_int_compare_and_exchange (volatile pint	*atomic,
				   pint			oldval,
				   pint			newval)
{
	pboolean success;

	p_mutex_lock (pp_atomic_mutex);

	if ((success = (*atomic == oldval)))
		*atomic = newval;

	p_mutex_unlock (pp_atomic_mutex);

	return success;
}

P_LIB_API pint
p_atomic_int_add (volatile pint	*atomic,
		  pint		val)
{
	pint oldval;

	p_mutex_lock (pp_atomic_mutex);
	oldval = *atomic;
	*atomic = oldval + val;
	p_mutex_unlock (pp_atomic_mutex);

	return oldval;
}

P_LIB_API puint
p_atomic_int_and (volatile puint	*atomic,
		  puint			val)
{
	puint oldval;

	p_mutex_lock (pp_atomic_mutex);
	oldval = *atomic;
	*atomic = oldval & val;
	p_mutex_unlock (pp_atomic_mutex);

	return oldval;
}

P_LIB_API puint
p_atomic_int_or (volatile puint	*atomic,
		 puint		val)
{
	puint oldval;

	p_mutex_lock (pp_atomic_mutex);
	oldval = *atomic;
	*atomic = oldval | val;
	p_mutex_unlock (pp_atomic_mutex);

	return oldval;
}

P_LIB_API puint
p_atomic_int_xor (volatile puint	*atomic,
		  puint			val)
{
	puint oldval;

	p_mutex_lock (pp_atomic_mutex);
	oldval = *atomic;
	*atomic = oldval ^ val;
	p_mutex_unlock (pp_atomic_mutex);

	return oldval;
}

P_LIB_API ppointer
p_atomic_pointer_get (const volatile void *atomic)
{
	const volatile ppointer *ptr = atomic;
	ppointer value;

	p_mutex_lock (pp_atomic_mutex);
	value = *ptr;
	p_mutex_unlock (pp_atomic_mutex);

	return value;
}

P_LIB_API void
p_atomic_pointer_set (volatile void	*atomic,
		      ppointer		val)
{
	volatile ppointer *ptr = atomic;

	p_mutex_lock (pp_atomic_mutex);
	*ptr = val;
	p_mutex_unlock (pp_atomic_mutex);
}

P_LIB_API pboolean
p_atomic_pointer_compare_and_exchange (volatile void	*atomic,
				       ppointer		oldval,
				       ppointer		newval)
{
	volatile ppointer *ptr = atomic;
	pboolean success;

	p_mutex_lock (pp_atomic_mutex);

	if ((success = (*ptr == oldval)))
		*ptr = newval;

	p_mutex_unlock (pp_atomic_mutex);

	return success;
}

P_LIB_API pssize
p_atomic_pointer_add (volatile void	*atomic,
		      pssize		val)
{
	volatile pssize *ptr = atomic;
	pssize oldval;

	p_mutex_lock (pp_atomic_mutex);
	oldval = *ptr;
	*ptr = oldval + val;
	p_mutex_unlock (pp_atomic_mutex);

	return oldval;
}

P_LIB_API psize
p_atomic_pointer_and (volatile void	*atomic,
		      psize		val)
{
	volatile psize *ptr = atomic;
	psize oldval;

	p_mutex_lock (pp_atomic_mutex);
	oldval = *ptr;
	*ptr = oldval & val;
	p_mutex_unlock (pp_atomic_mutex);

	return oldval;
}

P_LIB_API psize
p_atomic_pointer_or (volatile void	*atomic,
		     psize		val)
{
	volatile psize *ptr = atomic;
	psize oldval;

	p_mutex_lock (pp_atomic_mutex);
	oldval = *ptr;
	*ptr = oldval | val;
	p_mutex_unlock (pp_atomic_mutex);

	return oldval;
}

P_LIB_API psize
p_atomic_pointer_xor (volatile void	*atomic,
		      psize		val)
{
	volatile psize *ptr = atomic;
	psize oldval;

	p_mutex_lock (pp_atomic_mutex);
	oldval = *ptr;
	*ptr = oldval ^ val;
	p_mutex_unlock (pp_atomic_mutex);

	return oldval;
}

P_LIB_API pboolean
p_atomic_is_lock_free (void)
{
	return FALSE;
}

void
p_atomic_thread_init (void)
{
	if (P_LIKELY (pp_atomic_mutex == NULL))
		pp_atomic_mutex = p_mutex_new ();
}

void
p_atomic_thread_shutdown (void)
{
	if (P_LIKELY (pp_atomic_mutex != NULL)) {
		p_mutex_free (pp_atomic_mutex);
		pp_atomic_mutex = NULL;
	}
}
