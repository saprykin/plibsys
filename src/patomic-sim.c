/*
 * Copyright (C) 2011 Ryan Lortie <desrt@desrt.ca>
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

#include "patomic.h"
#include "pmutex.h"

/* We have to use the slow, but safe locking method. */
static PMutex *p_atomic_mutex = NULL;

P_LIB_API pint
p_atomic_int_get (const volatile pint *atomic)
{
	pint value;

	p_mutex_lock (p_atomic_mutex);
	value = *atomic;
	p_mutex_unlock (p_atomic_mutex);

	return value;
}

P_LIB_API void
p_atomic_int_set (volatile pint	*atomic,
		  pint		val)
{
	p_mutex_lock (p_atomic_mutex);
	*atomic = val;
	p_mutex_unlock (p_atomic_mutex);
}

P_LIB_API void
p_atomic_int_inc (volatile pint *atomic)
{
	p_mutex_lock (p_atomic_mutex);
	(*atomic)++;
	p_mutex_unlock (p_atomic_mutex);
}

P_LIB_API pboolean
p_atomic_int_dec_and_test (volatile pint *atomic)
{
	pboolean is_zero;

	p_mutex_lock (p_atomic_mutex);
	is_zero = --(*atomic) == 0;
	p_mutex_unlock (p_atomic_mutex);

	return is_zero;
}

P_LIB_API pboolean
p_atomic_int_compare_and_exchange (volatile pint	*atomic,
				   pint			oldval,
				   pint			newval)
{
	pboolean success;

	p_mutex_lock (p_atomic_mutex);

	if ((success = (*atomic == oldval)))
		*atomic = newval;

	p_mutex_unlock (p_atomic_mutex);

	return success;
}

P_LIB_API pint
p_atomic_int_add (volatile pint	*atomic,
		  pint		val)
{
	pint oldval;

	p_mutex_lock (p_atomic_mutex);
	oldval = *atomic;
	*atomic = oldval + val;
	p_mutex_unlock (p_atomic_mutex);

	return oldval;
}

P_LIB_API puint
p_atomic_int_and (volatile puint	*atomic,
		  puint			val)
{
	puint oldval;

	p_mutex_lock (p_atomic_mutex);
	oldval = *atomic;
	*atomic = oldval & val;
	p_mutex_unlock (p_atomic_mutex);

	return oldval;
}

P_LIB_API puint
p_atomic_int_or (volatile puint	*atomic,
		 puint		val)
{
	puint oldval;

	p_mutex_lock (p_atomic_mutex);
	oldval = *atomic;
	*atomic = oldval | val;
	p_mutex_unlock (p_atomic_mutex);

	return oldval;
}

P_LIB_API puint
p_atomic_int_xor (volatile puint	*atomic,
		  puint			val)
{
	puint oldval;

	p_mutex_lock (p_atomic_mutex);
	oldval = *atomic;
	*atomic = oldval ^ val;
	p_mutex_unlock (p_atomic_mutex);

	return oldval;
}

P_LIB_API ppointer
p_atomic_pointer_get (const volatile void *atomic)
{
	const volatile ppointer *ptr = atomic;
	ppointer value;

	p_mutex_lock (p_atomic_mutex);
	value = *ptr;
	p_mutex_unlock (p_atomic_mutex);

	return value;
}

P_LIB_API void
p_atomic_pointer_set (volatile void	*atomic,
		      ppointer		val)
{
	volatile ppointer *ptr = atomic;

	p_mutex_lock (p_atomic_mutex);
	*ptr = val;
	p_mutex_unlock (p_atomic_mutex);
}

P_LIB_API pboolean
p_atomic_pointer_compare_and_exchange (volatile void	*atomic,
				       ppointer		oldval,
				       ppointer		newval)
{
	volatile ppointer *ptr = atomic;
	pboolean success;

	p_mutex_lock (p_atomic_mutex);

	if ((success = (*ptr == oldval)))
		*ptr = newval;

	p_mutex_unlock (p_atomic_mutex);

	return success;
}

P_LIB_API pssize
p_atomic_pointer_add (volatile void	*atomic,
		      pssize		val)
{
	volatile pssize *ptr = atomic;
	pssize oldval;

	p_mutex_lock (p_atomic_mutex);
	oldval = *ptr;
	*ptr = oldval + val;
	p_mutex_unlock (p_atomic_mutex);

	return oldval;
}

P_LIB_API psize
p_atomic_pointer_and (volatile void	*atomic,
		      psize		val)
{
	volatile psize *ptr = atomic;
	psize oldval;

	p_mutex_lock (p_atomic_mutex);
	oldval = *ptr;
	*ptr = oldval & val;
	p_mutex_unlock (p_atomic_mutex);

	return oldval;
}

P_LIB_API psize
p_atomic_pointer_or (volatile void	*atomic,
		     psize		val)
{
	volatile psize *ptr = atomic;
	psize oldval;

	p_mutex_lock (p_atomic_mutex);
	oldval = *ptr;
	*ptr = oldval | val;
	p_mutex_unlock (p_atomic_mutex);

	return oldval;
}

P_LIB_API psize
p_atomic_pointer_xor (volatile void	*atomic,
		      psize		val)
{
	volatile psize *ptr = atomic;
	psize oldval;

	p_mutex_lock (p_atomic_mutex);
	oldval = *ptr;
	*ptr = oldval ^ val;
	p_mutex_unlock (p_atomic_mutex);

	return oldval;
}

P_LIB_API pboolean
p_atomic_is_lock_free (void)
{
	return FALSE;
}

void
__p_atomic_thread_init (void)
{
	if (p_atomic_mutex == NULL)
		p_atomic_mutex = p_mutex_new ();
}

void
__p_atomic_thread_shutdown (void)
{
	if (p_atomic_mutex != NULL) {
		p_mutex_free (p_atomic_mutex);
		p_atomic_mutex = NULL;
	}
}
