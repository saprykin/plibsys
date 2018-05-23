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
#include "prwlock.h"

#include <stdlib.h>
#include <pthread.h>

typedef pthread_rwlock_t rwlock_hdl;

struct PRWLock_ {
	rwlock_hdl hdl;
};

static pboolean pp_rwlock_unlock_any (PRWLock *lock);

static pboolean
pp_rwlock_unlock_any (PRWLock *lock)
{
	if (P_UNLIKELY (lock == NULL))
		return FALSE;

	if (P_LIKELY (pthread_rwlock_unlock (&lock->hdl) == 0))
		return TRUE;
	else {
		P_ERROR ("PRWLock::pp_rwlock_unlock_any: pthread_rwlock_unlock() failed");
		return FALSE;
	}
}

P_LIB_API PRWLock *
p_rwlock_new (void)
{
	PRWLock *ret;

	if (P_UNLIKELY ((ret = p_malloc0 (sizeof (PRWLock))) == NULL)) {
		P_ERROR ("PRWLock::p_rwlock_new: failed to allocate memory");
		return NULL;
	}

	if (P_UNLIKELY (pthread_rwlock_init (&ret->hdl, NULL) != 0)) {
		P_ERROR ("PRWLock::p_rwlock_new: pthread_rwlock_init() failed");
		p_free (ret);
		return NULL;
	}

	return ret;
}

P_LIB_API pboolean
p_rwlock_reader_lock (PRWLock *lock)
{
	if (P_UNLIKELY (lock == NULL))
		return FALSE;

	if (P_UNLIKELY (pthread_rwlock_rdlock (&lock->hdl) == 0))
		return TRUE;
	else {
		P_ERROR ("PRWLock::p_rwlock_reader_lock: pthread_rwlock_rdlock() failed");
		return FALSE;
	}
}

P_LIB_API pboolean
p_rwlock_reader_trylock (PRWLock *lock)
{
	if (P_UNLIKELY (lock == NULL))
		return FALSE;

	return (pthread_rwlock_tryrdlock (&lock->hdl) == 0) ? TRUE : FALSE;
}

P_LIB_API pboolean
p_rwlock_reader_unlock (PRWLock *lock)
{
	return pp_rwlock_unlock_any (lock);
}

P_LIB_API pboolean
p_rwlock_writer_lock (PRWLock *lock)
{
	if (P_UNLIKELY (lock == NULL))
		return FALSE;

	if (P_UNLIKELY (pthread_rwlock_wrlock (&lock->hdl) == 0))
		return TRUE;
	else {
		P_ERROR ("PRWLock::p_rwlock_writer_lock: pthread_rwlock_wrlock() failed");
		return FALSE;
	}
}

P_LIB_API pboolean
p_rwlock_writer_trylock (PRWLock *lock)
{
	if (P_UNLIKELY (lock == NULL))
		return FALSE;

	return (pthread_rwlock_trywrlock (&lock->hdl) == 0) ? TRUE : FALSE;
}

P_LIB_API pboolean
p_rwlock_writer_unlock (PRWLock *lock)
{
	return pp_rwlock_unlock_any (lock);
}

P_LIB_API void
p_rwlock_free (PRWLock *lock)
{
	if (P_UNLIKELY (lock == NULL))
		return;

	if (P_UNLIKELY (pthread_rwlock_destroy (&lock->hdl) != 0))
		P_ERROR ("PRWLock::p_rwlock_free: pthread_rwlock_destroy() failed");

	p_free (lock);
}

void
p_rwlock_init (void)
{
}

void
p_rwlock_shutdown (void)
{
}
