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

#include "pmem.h"
#include "pspinlock.h"

#ifdef P_OS_VMS
#  include <builtins.h>
#else
#  include <machine/builtins.h>
#endif

struct PSpinLock_ {
	volatile pint spin;
};

P_LIB_API PSpinLock *
p_spinlock_new (void)
{
	PSpinLock *ret;

	if (P_UNLIKELY ((ret = p_malloc0 (sizeof (PSpinLock))) == NULL)) {
		P_ERROR ("PSpinLock::p_spinlock_new: failed to allocate memory");
		return NULL;
	}

	return ret;
}

P_LIB_API pboolean
p_spinlock_lock (PSpinLock *spinlock)
{
	if (P_UNLIKELY (spinlock == NULL))
		return FALSE;

	(void) __LOCK_LONG ((volatile void *) &(spinlock->spin));

	return TRUE;
}

P_LIB_API pboolean
p_spinlock_trylock (PSpinLock *spinlock)
{
	if (P_UNLIKELY (spinlock == NULL))
		return FALSE;

	return __LOCK_LONG_RETRY ((volatile void *) &(spinlock->spin), 1) == 1 ? TRUE : FALSE;
}

P_LIB_API pboolean
p_spinlock_unlock (PSpinLock *spinlock)
{
	if (P_UNLIKELY (spinlock == NULL))
		return FALSE;

	(void) __UNLOCK_LONG ((volatile void *) &(spinlock->spin));

	return TRUE;
}

P_LIB_API void
p_spinlock_free (PSpinLock *spinlock)
{
	p_free (spinlock);
}
