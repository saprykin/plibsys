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

#include "pmem.h"
#include "pspinlock.h"

struct PSpinLock_ {
	volatile pint spin;
};

P_LIB_API PSpinLock *
p_spinlock_new (void)
{
	PSpinLock *ret;

	if (P_UNLIKELY ((ret = p_malloc0 (sizeof (PSpinLock))) == NULL)) {
		P_ERROR ("PSpinLock: failed to allocate memory");
		return NULL;
	}

	return ret;
}

P_LIB_API pboolean
p_spinlock_lock (PSpinLock *spinlock)
{
	pint tmp_int;

	if (P_UNLIKELY (spinlock == NULL))
		return FALSE;

	do {
		tmp_int = 0;
	} while ((pboolean) __atomic_compare_exchange_n (&(spinlock->spin),
							 &tmp_int,
							 1,
							 0,
							 __ATOMIC_ACQUIRE,
							 __ATOMIC_RELAXED) == FALSE);

	return TRUE;
}

P_LIB_API pboolean
p_spinlock_trylock (PSpinLock *spinlock)
{
	pint tmp_int = 0;

	if (P_UNLIKELY (spinlock == NULL))
		return FALSE;

	return (pboolean) __atomic_compare_exchange_n (&(spinlock->spin),
						       &tmp_int,
						       1,
						       0,
						       __ATOMIC_ACQUIRE,
						       __ATOMIC_RELAXED);
}

P_LIB_API pboolean
p_spinlock_unlock (PSpinLock *spinlock)
{
	if (P_UNLIKELY (spinlock == NULL))
		return FALSE;

	__atomic_store_4 (&(spinlock->spin), 0, __ATOMIC_RELEASE);

	return TRUE;
}

P_LIB_API void
p_spinlock_free (PSpinLock *spinlock)
{
	p_free (spinlock);
}
