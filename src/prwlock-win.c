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

/* More emulation variants: https://github.com/neosmart/RWLock */

#include "pmem.h"
#include "patomic.h"
#include "puthread.h"
#include "prwlock.h"

#include <stdlib.h>

#define P_RWLOCK_XP_MAX_SPIN 4000
#define P_RWLOCK_XP_IS_CLEAR(lock) (((lock) & 0x40007FFF) == 0)
#define P_RWLOCK_XP_IS_WRITER(lock) (((lock) & 0x40000000) != 0)
#define P_RWLOCK_XP_SET_WRITER(lock) ((lock) | 0x40000000)
#define P_RWLOCK_XP_UNSET_WRITER(lock) ((lock) & (~0x40000000))
#define P_RWLOCK_XP_SET_READERS(lock, readers) (((lock) & (~0x00007FFF)) | (readers))
#define P_RWLOCK_XP_READER_COUNT(lock) ((lock) & 0x00007FFF)
#define P_RWLOCK_XP_SET_WAITING(lock, waiting) (((lock) & (~0x3FFF8000)) | ((waiting) << 15))
#define P_RWLOCK_XP_WAITING_COUNT(lock) (((lock) & 0x3FFF8000) >> 15)

typedef VOID    (WINAPI *InitializeSRWLockFunc)          (ppointer lock);
typedef VOID    (WINAPI *AcquireSRWLockExclusiveFunc)    (ppointer lock);
typedef BOOLEAN (WINAPI *TryAcquireSRWLockExclusiveFunc) (ppointer lock);
typedef VOID    (WINAPI *ReleaseSRWLockExclusiveFunc)    (ppointer lock);
typedef VOID    (WINAPI *AcquireSRWLockSharedFunc)       (ppointer lock);
typedef BOOLEAN (WINAPI *TryAcquireSRWLockSharedFunc)    (ppointer lock);
typedef VOID    (WINAPI *ReleaseSRWLockSharedFunc)       (ppointer lock);

typedef pboolean (* PWin32LockInit)          (PRWLock *lock);
typedef void     (* PWin32LockClose)         (PRWLock *lock);
typedef pboolean (* PWin32LockStartRead)     (PRWLock *lock);
typedef pboolean (* PWin32LockStartReadTry)  (PRWLock *lock);
typedef pboolean (* PWin32LockEndRead)       (PRWLock *lock);
typedef pboolean (* PWin32LockStartWrite)    (PRWLock *lock);
typedef pboolean (* PWin32LockStartWriteTry) (PRWLock *lock);
typedef pboolean (* PWin32LockEndWrite)      (PRWLock *lock);

static PWin32LockInit          pp_rwlock_init_func            = NULL;
static PWin32LockClose         pp_rwlock_close_func           = NULL;
static PWin32LockStartRead     pp_rwlock_start_read_func      = NULL;
static PWin32LockStartReadTry  pp_rwlock_start_read_try_func  = NULL;
static PWin32LockEndRead       pp_rwlock_end_read_func        = NULL;
static PWin32LockStartWrite    pp_rwlock_start_write_func     = NULL;
static PWin32LockStartWriteTry pp_rwlock_start_write_try_func = NULL;
static PWin32LockEndWrite      pp_rwlock_end_write_func       = NULL;

typedef struct PRWLockVistaTable_ {
	InitializeSRWLockFunc		rwl_init;
	AcquireSRWLockExclusiveFunc	rwl_excl_lock;
	TryAcquireSRWLockExclusiveFunc	rwl_excl_lock_try;
	ReleaseSRWLockExclusiveFunc	rwl_excl_rel;
	AcquireSRWLockSharedFunc	rwl_shr_lock;
	TryAcquireSRWLockSharedFunc	rwl_shr_lock_try;
	ReleaseSRWLockSharedFunc	rwl_shr_rel;
} PRWLockVistaTable;

typedef struct PRWLockXP_ {
	volatile puint32	lock;
	HANDLE			event;
} PRWLockXP;

struct PRWLock_ {
	ppointer lock;
};

static PRWLockVistaTable pp_rwlock_vista_table = {NULL, NULL, NULL, NULL,
						  NULL, NULL, NULL};

/* SRWLock routines */
static pboolean pp_rwlock_init_vista (PRWLock *lock);
static void pp_rwlock_close_vista (PRWLock *lock);
static pboolean pp_rwlock_start_read_vista (PRWLock *lock);
static pboolean pp_rwlock_start_read_try_vista (PRWLock *lock);
static pboolean pp_rwlock_end_read_vista (PRWLock *lock);
static pboolean pp_rwlock_start_write_vista (PRWLock *lock);
static pboolean pp_rwlock_start_write_try_vista (PRWLock *lock);
static pboolean pp_rwlock_end_write_vista (PRWLock *lock);

/* Windows XP emulation routines */
static pboolean pp_rwlock_init_xp (PRWLock *lock);
static void pp_rwlock_close_xp (PRWLock *lock);
static pboolean pp_rwlock_start_read_xp (PRWLock *lock);
static pboolean pp_rwlock_start_read_try_xp (PRWLock *lock);
static pboolean pp_rwlock_end_read_xp (PRWLock *lock);
static pboolean pp_rwlock_start_write_xp (PRWLock *lock);
static pboolean pp_rwlock_start_write_try_xp (PRWLock *lock);
static pboolean pp_rwlock_end_write_xp (PRWLock *lock);

/* SRWLock routines */

static pboolean
pp_rwlock_init_vista (PRWLock *lock)
{
	pp_rwlock_vista_table.rwl_init (lock);

	return TRUE;
}

static void
pp_rwlock_close_vista (PRWLock *lock)
{
	P_UNUSED (lock);
}

static pboolean
pp_rwlock_start_read_vista (PRWLock *lock)
{
	pp_rwlock_vista_table.rwl_shr_lock (lock);

	return TRUE;
}

static pboolean
pp_rwlock_start_read_try_vista (PRWLock *lock)
{
	return pp_rwlock_vista_table.rwl_shr_lock_try (lock) != 0 ? TRUE : FALSE;
}

static pboolean
pp_rwlock_end_read_vista (PRWLock *lock)
{
	pp_rwlock_vista_table.rwl_shr_rel (lock);

	return TRUE;
}

static pboolean
pp_rwlock_start_write_vista (PRWLock *lock)
{
	pp_rwlock_vista_table.rwl_excl_lock (lock);

	return TRUE;
}

static pboolean
pp_rwlock_start_write_try_vista (PRWLock *lock)
{
	return pp_rwlock_vista_table.rwl_excl_lock_try (lock) != 0 ? TRUE : FALSE;
}

static pboolean
pp_rwlock_end_write_vista (PRWLock *lock)
{
	pp_rwlock_vista_table.rwl_excl_rel (lock);

	return TRUE;
}

/* Windows XP emulation routines */

static pboolean
pp_rwlock_init_xp (PRWLock *lock)
{
	PRWLockXP *rwl_xp;

	if ((lock->lock = p_malloc0 (sizeof (PRWLockXP))) == NULL) {
		P_ERROR ("PRWLock::pp_rwlock_init_xp: failed to allocate memory");
		return FALSE;
	}

	rwl_xp = ((PRWLockXP *) lock->lock);

	rwl_xp->lock  = 0;
	rwl_xp->event = CreateEventA (NULL, FALSE, FALSE, NULL);

	if (P_UNLIKELY (rwl_xp->event == NULL)) {
		P_ERROR ("PRWLock::pp_rwlock_init_xp: CreateEventA() failed");
		p_free (lock->lock);
		lock->lock = NULL;
		return FALSE;
	}

	return TRUE;
}

static void
pp_rwlock_close_xp (PRWLock *lock)
{
	CloseHandle (((PRWLockXP *) lock->lock)->event);
	p_free (lock->lock);
}

static pboolean
pp_rwlock_start_read_xp (PRWLock *lock)
{
	PRWLockXP	*rwl_xp = ((PRWLockXP *) lock->lock);
	int		i;
	puint32		tmp_lock;
	puint32		counter;

	for (i = 0; ; ++i) {
		tmp_lock = (puint32) p_atomic_int_get ((const volatile pint *) &rwl_xp->lock);

		if (!P_RWLOCK_XP_IS_WRITER (tmp_lock)) {
			counter = P_RWLOCK_XP_SET_READERS (tmp_lock, P_RWLOCK_XP_READER_COUNT (tmp_lock) + 1);

			if (p_atomic_int_compare_and_exchange ((volatile pint *) &rwl_xp->lock,
							       (pint) tmp_lock,
							       (pint) counter) == TRUE)
				return TRUE;
			else
				continue;
		} else {
			if (P_LIKELY (i < P_RWLOCK_XP_MAX_SPIN)) {
				p_uthread_yield ();
				continue;
			}

			counter = P_RWLOCK_XP_SET_WAITING (tmp_lock, P_RWLOCK_XP_WAITING_COUNT (tmp_lock) + 1);

			if (p_atomic_int_compare_and_exchange ((volatile pint *) &rwl_xp->lock,
							       (pint) tmp_lock,
							       (pint) counter) != TRUE)
				continue;

			i = 0;

			if (P_UNLIKELY (WaitForSingleObject (rwl_xp->event, INFINITE) != WAIT_OBJECT_0))
				P_WARNING ("PRWLock::pp_rwlock_start_read_xp: WaitForSingleObject() failed, go ahead");

			do {
				tmp_lock = p_atomic_int_get ((const volatile pint *) &rwl_xp->lock);
				counter  = P_RWLOCK_XP_SET_WAITING (tmp_lock, P_RWLOCK_XP_WAITING_COUNT (tmp_lock) - 1);
			} while (p_atomic_int_compare_and_exchange ((volatile pint *) &rwl_xp->lock,
								    (pint) tmp_lock,
								    (pint) counter) != TRUE);
		}
	}

	return TRUE;
}

static pboolean
pp_rwlock_start_read_try_xp (PRWLock *lock)
{
	PRWLockXP	*rwl_xp = ((PRWLockXP *) lock->lock);
	puint32		tmp_lock;
	puint32		counter;

	tmp_lock = (puint32) p_atomic_int_get ((const volatile pint *) &rwl_xp->lock);

	if (P_RWLOCK_XP_IS_WRITER (tmp_lock))
		return FALSE;

	counter = P_RWLOCK_XP_SET_READERS (tmp_lock, P_RWLOCK_XP_READER_COUNT (tmp_lock) + 1);

	return p_atomic_int_compare_and_exchange ((volatile pint *) &rwl_xp->lock,
						  (pint) tmp_lock,
						  (pint) counter);
}

static pboolean
pp_rwlock_end_read_xp (PRWLock *lock)
{
	PRWLockXP	*rwl_xp = ((PRWLockXP *) lock->lock);
	puint32		tmp_lock;
	puint32		counter;

	while (TRUE) {
		tmp_lock = (puint32) p_atomic_int_get ((const volatile pint *) &rwl_xp->lock);
		counter  = P_RWLOCK_XP_READER_COUNT (tmp_lock);

		if (P_UNLIKELY (counter == 0))
			return TRUE;

		if (counter == 1 && P_RWLOCK_XP_WAITING_COUNT (tmp_lock) != 0) {
			/* A duplicate wake up notification is possible */
			if (P_UNLIKELY (SetEvent (rwl_xp->event) == 0))
				P_WARNING ("PRWLock::pp_rwlock_end_read_xp: SetEvent() failed");
		}

		counter = P_RWLOCK_XP_SET_READERS (tmp_lock, counter - 1);

		if (p_atomic_int_compare_and_exchange ((volatile pint *) &rwl_xp->lock,
						       (pint) tmp_lock,
						       (pint) counter) == TRUE)
			break;
	}

	return TRUE;
}

static pboolean
pp_rwlock_start_write_xp (PRWLock *lock)
{
	PRWLockXP	*rwl_xp = ((PRWLockXP *) lock->lock);
	int		i;
	puint32		tmp_lock;
	puint32		counter;

	for (i = 0; ; ++i) {
		tmp_lock = (puint32) p_atomic_int_get ((const volatile pint *) &rwl_xp->lock);

		if (P_RWLOCK_XP_IS_CLEAR (tmp_lock)) {
			counter = P_RWLOCK_XP_SET_WRITER (tmp_lock);

			if (p_atomic_int_compare_and_exchange ((volatile pint *) &rwl_xp->lock,
							       (pint) tmp_lock,
							       (pint) counter) == TRUE)
				return TRUE;
			else
				continue;
		} else {
			if (P_LIKELY (i < P_RWLOCK_XP_MAX_SPIN)) {
				p_uthread_yield ();
				continue;
			}

			counter = P_RWLOCK_XP_SET_WAITING (tmp_lock, P_RWLOCK_XP_WAITING_COUNT (tmp_lock) + 1);

			if (p_atomic_int_compare_and_exchange ((volatile pint *) &rwl_xp->lock,
							       (pint) tmp_lock,
							       (pint) counter) != TRUE)
				continue;

			i = 0;

			if (P_UNLIKELY (WaitForSingleObject (rwl_xp->event, INFINITE) != WAIT_OBJECT_0))
				P_WARNING ("PRWLock::pp_rwlock_start_write_xp: WaitForSingleObject() failed, go ahead");

			do {
				tmp_lock = p_atomic_int_get ((const volatile pint *) &rwl_xp->lock);
				counter  = P_RWLOCK_XP_SET_WAITING (tmp_lock, P_RWLOCK_XP_WAITING_COUNT (tmp_lock) - 1);
			} while (p_atomic_int_compare_and_exchange ((volatile pint *) &rwl_xp->lock,
								    (pint) tmp_lock,
								    (pint) counter) != TRUE);
		}
	}

	return TRUE;
}

static pboolean
pp_rwlock_start_write_try_xp (PRWLock *lock)
{
	PRWLockXP	*rwl_xp = ((PRWLockXP *) lock->lock);
	puint32		tmp_lock;

	tmp_lock = (puint32) p_atomic_int_get ((const volatile pint *) &rwl_xp->lock);

	if (P_RWLOCK_XP_IS_CLEAR (tmp_lock)) {
		return p_atomic_int_compare_and_exchange ((volatile pint *) &rwl_xp->lock,
							  (pint) tmp_lock,
							  (pint) P_RWLOCK_XP_SET_WRITER (tmp_lock));
	}

	return FALSE;
}

static pboolean
pp_rwlock_end_write_xp (PRWLock *lock)
{
	PRWLockXP	*rwl_xp = ((PRWLockXP *) lock->lock);
	puint32		tmp_lock;

	while (TRUE) {
		while (TRUE) {
			tmp_lock = (puint32) p_atomic_int_get ((const volatile pint *) &rwl_xp->lock);

			if (P_UNLIKELY (!P_RWLOCK_XP_IS_WRITER (tmp_lock)))
				return TRUE;

			if (P_RWLOCK_XP_WAITING_COUNT (tmp_lock) == 0)
				break;

			/* Only the one end-of-write call can be */
			if (P_UNLIKELY (SetEvent (rwl_xp->event) == 0))
				P_WARNING ("PRWLock::pp_rwlock_end_write_xp: SetEvent() failed");
		}

		if (p_atomic_int_compare_and_exchange ((volatile pint *) &rwl_xp->lock,
						       (pint) tmp_lock,
						       (pint) P_RWLOCK_XP_UNSET_WRITER (tmp_lock)) == TRUE)
			break;
	}

	return TRUE;
}

P_LIB_API PRWLock *
p_rwlock_new (void)
{
	PRWLock *ret;

	if (P_UNLIKELY ((ret = p_malloc0 (sizeof (PRWLock))) == NULL)) {
		P_ERROR ("PRWLock::p_rwlock_new: failed to allocate memory");
		return NULL;
	}

	if (P_UNLIKELY (pp_rwlock_init_func (ret) != TRUE)) {
		P_ERROR ("PRWLock::p_rwlock_new: failed to initialize");
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

	return pp_rwlock_start_read_func (lock);
}

P_LIB_API pboolean
p_rwlock_reader_trylock (PRWLock *lock)
{
	if (P_UNLIKELY (lock == NULL))
		return FALSE;

	return pp_rwlock_start_read_try_func (lock);
}

P_LIB_API pboolean
p_rwlock_reader_unlock (PRWLock *lock)
{
	if (P_UNLIKELY (lock == NULL))
		return FALSE;

	return pp_rwlock_end_read_func (lock);
}

P_LIB_API pboolean
p_rwlock_writer_lock (PRWLock *lock)
{
	if (P_UNLIKELY (lock == NULL))
		return FALSE;

	return pp_rwlock_start_write_func (lock);
}

P_LIB_API pboolean
p_rwlock_writer_trylock (PRWLock *lock)
{
	if (P_UNLIKELY (lock == NULL))
		return FALSE;

	return pp_rwlock_start_write_try_func (lock);
}

P_LIB_API pboolean
p_rwlock_writer_unlock (PRWLock *lock)
{
	if (P_UNLIKELY (lock == NULL))
		return FALSE;

	return pp_rwlock_end_write_func (lock);
}

P_LIB_API void
p_rwlock_free (PRWLock *lock)
{
	if (P_UNLIKELY (lock == NULL))
		return;

	pp_rwlock_close_func (lock);
	p_free (lock);
}

void
p_rwlock_init (void)
{
	HMODULE hmodule;

	hmodule = GetModuleHandleA ("kernel32.dll");

	if (P_UNLIKELY (hmodule == NULL)) {
		P_ERROR ("PRWLock::p_rwlock_init: failed to load kernel32.dll module");
		return;
	}

	pp_rwlock_vista_table.rwl_init = (InitializeSRWLockFunc) GetProcAddress (hmodule,
										 "InitializeSRWLock");

	if (P_LIKELY (pp_rwlock_vista_table.rwl_init != NULL)) {
		pp_rwlock_vista_table.rwl_excl_lock     = (AcquireSRWLockExclusiveFunc) GetProcAddress (hmodule,
													"AcquireSRWLockExclusive");
		pp_rwlock_vista_table.rwl_excl_lock_try = (TryAcquireSRWLockExclusiveFunc) GetProcAddress (hmodule,
													   "TryAcquireSRWLockExclusive");
		pp_rwlock_vista_table.rwl_excl_rel      = (ReleaseSRWLockExclusiveFunc) GetProcAddress (hmodule,
													"ReleaseSRWLockExclusive");
		pp_rwlock_vista_table.rwl_shr_lock      = (AcquireSRWLockSharedFunc) GetProcAddress (hmodule,
												     "AcquireSRWLockShared");
		pp_rwlock_vista_table.rwl_shr_lock_try  = (TryAcquireSRWLockSharedFunc) GetProcAddress (hmodule,
													"TryAcquireSRWLockShared");
		pp_rwlock_vista_table.rwl_shr_rel       = (ReleaseSRWLockSharedFunc) GetProcAddress (hmodule,
												     "ReleaseSRWLockShared");
		pp_rwlock_init_func            = pp_rwlock_init_vista;
		pp_rwlock_close_func           = pp_rwlock_close_vista;
		pp_rwlock_start_read_func      = pp_rwlock_start_read_vista;
		pp_rwlock_start_read_try_func  = pp_rwlock_start_read_try_vista;
		pp_rwlock_end_read_func        = pp_rwlock_end_read_vista;
		pp_rwlock_start_write_func     = pp_rwlock_start_write_vista;
		pp_rwlock_start_write_try_func = pp_rwlock_start_write_try_vista;
		pp_rwlock_end_write_func       = pp_rwlock_end_write_vista;
	} else {
		pp_rwlock_init_func            = pp_rwlock_init_xp;
		pp_rwlock_close_func           = pp_rwlock_close_xp;
		pp_rwlock_start_read_func      = pp_rwlock_start_read_xp;
		pp_rwlock_start_read_try_func  = pp_rwlock_start_read_try_xp;
		pp_rwlock_end_read_func        = pp_rwlock_end_read_xp;
		pp_rwlock_start_write_func     = pp_rwlock_start_write_xp;
		pp_rwlock_start_write_try_func = pp_rwlock_start_write_try_xp;
		pp_rwlock_end_write_func       = pp_rwlock_end_write_xp;
	}
}

void
p_rwlock_shutdown (void)
{
	memset (&pp_rwlock_vista_table, 0, sizeof (pp_rwlock_vista_table));

	pp_rwlock_init_func            = NULL;
	pp_rwlock_close_func           = NULL;
	pp_rwlock_start_read_func      = NULL;
	pp_rwlock_start_read_try_func  = NULL;
	pp_rwlock_end_read_func        = NULL;
	pp_rwlock_start_write_func     = NULL;
	pp_rwlock_start_write_try_func = NULL;
	pp_rwlock_end_write_func       = NULL;
}
