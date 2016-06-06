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

#include "pmem.h"
#include "pmutex.h"
#include "pcondvariable.h"
#include "prwlock.h"

#include <stdlib.h>

#define P_RWLOCK_SET_READERS(lock, readers) (((lock) & (~0x00007FFF)) | (readers))
#define P_RWLOCK_READER_COUNT(lock) ((lock) & 0x00007FFF)
#define P_RWLOCK_SET_WRITERS(lock, writers) (((lock) & (~0x3FFF8000)) | ((writers) << 15))
#define P_RWLOCK_WRITER_COUNT(lock) (((lock) & 0x3FFF8000) >> 15)

struct PRWLock_ {
	PMutex		*mutex;
	PCondVariable	*read_cv;
	PCondVariable	*write_cv;
	puint32		active_threads;
	puint32		waiting_threads;
};

P_LIB_API PRWLock *
p_rwlock_new (void)
{
	PRWLock *ret;

	if (P_UNLIKELY ((ret = p_malloc0 (sizeof (PRWLock))) == NULL)) {
		P_ERROR ("PRWLock::p_rwlock_new: failed to allocate memory");
		return NULL;
	}

	if (P_UNLIKELY ((ret->mutex = p_mutex_new ()) == NULL)) {
		P_ERROR ("PRWLock::p_rwlock_new: failed to allocate mutex");
		p_free (ret);
	}

	if (P_UNLIKELY ((ret->read_cv = p_cond_variable_new ()) == NULL)) {
		P_ERROR ("PRWLock::p_rwlock_new: failed to allocate condition variable for read");
		p_mutex_free (ret->mutex);
		p_free (ret);
	}

	if (P_UNLIKELY ((ret->write_cv = p_cond_variable_new ()) == NULL)) {
		P_ERROR ("PRWLock::p_rwlock_new: failed to allocate condition variable for write");
		p_cond_variable_free (ret->read_cv);
		p_mutex_free (ret->mutex);
		p_free (ret);
	}

	return ret;
}

P_LIB_API pboolean
p_rwlock_reader_lock (PRWLock *lock)
{
	pboolean wait_ok;

	if (P_UNLIKELY (lock == NULL))
		return FALSE;

	if (P_UNLIKELY (p_mutex_lock (lock->mutex) == FALSE)) {
		P_ERROR ("PRWLock::p_rwlock_reader_lock: failed to lock mutex");
		return FALSE;
	}

	wait_ok = TRUE;

	if (P_RWLOCK_WRITER_COUNT (lock->active_threads)) {
		lock->waiting_threads = P_RWLOCK_SET_READERS (lock->waiting_threads,
							      P_RWLOCK_READER_COUNT (lock->waiting_threads) + 1);

		while (P_RWLOCK_WRITER_COUNT (lock->active_threads)) {
			wait_ok = p_cond_variable_wait (lock->read_cv, lock->mutex);

			if (P_UNLIKELY (wait_ok == FALSE)) {
				P_ERROR ("PRWLock::p_rwlock_reader_lock: failed to wait for read condition");
				break;
			}
		}

		lock->waiting_threads = P_RWLOCK_SET_READERS (lock->waiting_threads,
							      P_RWLOCK_READER_COUNT (lock->waiting_threads) - 1);
	}

	if (P_LIKELY (wait_ok == TRUE))
		lock->active_threads = P_RWLOCK_SET_READERS (lock->active_threads,
							     P_RWLOCK_READER_COUNT (lock->active_threads) + 1);

	if (P_UNLIKELY (p_mutex_unlock (lock->mutex) == FALSE)) {
		P_ERROR ("PRWLock:;p_rwlock_reader_lock: failed to unlock mutex");
		return FALSE;
	}

	return wait_ok;
}

P_LIB_API pboolean
p_rwlock_reader_trylock (PRWLock *lock)
{
	if (P_UNLIKELY (lock == NULL))
		return FALSE;

	if (P_UNLIKELY (p_mutex_lock (lock->mutex) == FALSE)) {
		P_ERROR ("PRWLock::p_rwlock_reader_trylock: failed to lock mutex");
		return FALSE;
	}

	if (P_RWLOCK_WRITER_COUNT (lock->active_threads)) {
		if (P_UNLIKELY (p_mutex_unlock (lock->mutex) == FALSE))
			P_ERROR ("PRWLock::p_rwlock_reader_trylock(1): failed to unlock mutex");

		return FALSE;
	}

	lock->active_threads = P_RWLOCK_SET_READERS (lock->active_threads,
						     P_RWLOCK_READER_COUNT (lock->active_threads) + 1);

	if (P_UNLIKELY (p_mutex_unlock (lock->mutex) == FALSE)) {
		P_ERROR ("PRWLock::p_rwlock_reader_trylock(2): failed to unlock mutex");
		return FALSE;
	}

	return TRUE;
}

P_LIB_API pboolean
p_rwlock_reader_unlock (PRWLock *lock)
{
	puint32		reader_count;
	pboolean	signal_ok;

	if (P_UNLIKELY (lock == NULL))
		return FALSE;

	if (P_UNLIKELY (p_mutex_lock (lock->mutex) == FALSE)) {
		P_ERROR ("PRWLock::p_rwlock_reader_unlock: failed to lock mutex");
		return FALSE;
	}

	reader_count = P_RWLOCK_READER_COUNT (lock->active_threads);

	if (P_UNLIKELY (reader_count == 0)) {
		if (P_UNLIKELY (p_mutex_unlock (lock->mutex) == FALSE))
			P_ERROR ("PRWLock::p_rwlock_reader_unlock(1): failed to unlock mutex");

		return TRUE;
	}

	lock->active_threads = P_RWLOCK_SET_READERS (lock->active_threads, reader_count - 1);

	signal_ok = TRUE;

	if (reader_count == 1 && P_RWLOCK_WRITER_COUNT (lock->waiting_threads))
		signal_ok = p_cond_variable_signal (lock->write_cv);

	if (P_UNLIKELY (signal_ok == FALSE))
		P_ERROR ("PRWLock::p_rwlock_reader_unlock: failed to signal write condition");

	if (P_UNLIKELY (p_mutex_unlock (lock->mutex) == FALSE)) {
		P_ERROR ("PRWLock::p_rwlock_reader_unlock(2): failed to unlock mutex");
		return FALSE;
	}

	return signal_ok;
}

P_LIB_API pboolean
p_rwlock_writer_lock (PRWLock *lock)
{
	pboolean wait_ok;

	if (P_UNLIKELY (lock == NULL))
		return FALSE;

	if (P_UNLIKELY (p_mutex_lock (lock->mutex) == FALSE)) {
		P_ERROR ("PRWLock::p_rwlock_writer_lock: failed to lock mutex");
		return FALSE;
	}

	wait_ok = TRUE;

	if (lock->active_threads) {
		lock->waiting_threads = P_RWLOCK_SET_WRITERS (lock->waiting_threads,
							      P_RWLOCK_WRITER_COUNT (lock->waiting_threads) + 1);

		while (lock->active_threads) {
			wait_ok = p_cond_variable_wait (lock->write_cv, lock->mutex);

			if (P_UNLIKELY (wait_ok == FALSE)) {
				P_ERROR ("PRWLock::p_rwlock_writer_lock: failed to wait for write condition");
				break;
			}
		}

		lock->waiting_threads = P_RWLOCK_SET_WRITERS (lock->waiting_threads,
							      P_RWLOCK_WRITER_COUNT (lock->waiting_threads) - 1);
	}

	if (P_LIKELY (wait_ok == TRUE))
		lock->active_threads = P_RWLOCK_SET_WRITERS (lock->active_threads, 1);

	if (P_UNLIKELY (p_mutex_unlock (lock->mutex) == FALSE)) {
		P_ERROR ("PRWLock::p_rwlock_writer_lock: failed to unlock mutex");
		return FALSE;
	}

	return wait_ok;
}

P_LIB_API pboolean
p_rwlock_writer_trylock (PRWLock *lock)
{
	if (P_UNLIKELY (lock == NULL))
		return FALSE;

	if (P_UNLIKELY (p_mutex_lock (lock->mutex) == FALSE)) {
		P_ERROR ("PRWLock::p_rwlock_writer_trylock: failed to lock mutex");
		return FALSE;
	}

	if (lock->active_threads) {
		if (P_UNLIKELY (p_mutex_unlock (lock->mutex) == FALSE))
			P_ERROR ("PRWLock::p_rwlock_writer_trylock(1): failed to unlock mutex");

		return FALSE;
	}

	lock->active_threads = P_RWLOCK_SET_WRITERS (lock->active_threads, 1);

	if (P_UNLIKELY (p_mutex_unlock (lock->mutex) == FALSE)) {
		P_ERROR ("PRWLock:;p_rwlock_writer_trylock(2): failed to unlock mutex");
		return FALSE;
	}

	return TRUE;
}

P_LIB_API pboolean
p_rwlock_writer_unlock (PRWLock *lock)
{
	pboolean signal_ok;

	if (P_UNLIKELY (lock == NULL))
		return FALSE;

	if (P_UNLIKELY (p_mutex_lock (lock->mutex) == FALSE)) {
		P_ERROR ("PRWLock::p_rwlock_writer_unlock: failed to lock mutex");
		return FALSE;
	}

	lock->active_threads = P_RWLOCK_SET_WRITERS (lock->active_threads, 0);

	signal_ok = TRUE;

	if (P_RWLOCK_WRITER_COUNT (lock->waiting_threads)) {
		if (P_UNLIKELY (p_cond_variable_signal (lock->write_cv) == FALSE)) {
			P_ERROR ("PRWLock::p_rwlock_writer_unlock: failed to signal write condition");
			signal_ok = FALSE;
		}
	} else if (P_RWLOCK_READER_COUNT (lock->waiting_threads)) {
		if (P_UNLIKELY (p_cond_variable_broadcast (lock->read_cv) == FALSE)) {
			P_ERROR ("PRWLock::p_rwlock_writer_unlock: failed to broadcast read condition");
			signal_ok = FALSE;
		}
	}

	if (P_UNLIKELY (p_mutex_unlock (lock->mutex) == FALSE)) {
		P_ERROR ("PRWLock::p_rwlock_writer_unlock: failed to unlock mutex");
		return FALSE;
	}

	return signal_ok;
}

P_LIB_API void
p_rwlock_free (PRWLock *lock)
{
	if (P_UNLIKELY (lock == NULL))
		return;

	if (P_UNLIKELY (lock->active_threads))
		P_WARNING ("PRWLock::p_rwlock_free: destroying while active threads are present");

	if (P_UNLIKELY (lock->waiting_threads))
		P_WARNING ("PRWLock::p_rwlock_free: destroying while waiting threads are present");

	p_mutex_free (lock->mutex);
	p_cond_variable_free (lock->read_cv);
	p_cond_variable_free (lock->write_cv);

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
