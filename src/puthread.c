/*
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
#include "pspinlock.h"
#include "pmem.h"
#include "plibsys-private.h"

#include <stdlib.h>
#include <time.h>
#include <string.h>

extern void __p_uthread_init_internal (void);
extern void __p_uthread_shutdown_internal (void);
extern PUThread * __p_uthread_create_internal (PUThreadFunc	func,
					       pboolean		joinable,
					       PUThreadPriority	prio);
extern void __p_uthread_exit_internal (void);
extern void __p_uthread_wait_internal (PUThread *thread);
extern void __p_uthread_free_internal (PUThread *thread);

static void __p_uthread_cleanup (ppointer data);
static ppointer __p_uthread_proxy (ppointer data);

static PUThreadKey *__p_uthread_specific_data = NULL;
static PSpinLock * __p_uthread_new_spin = NULL;

static void
__p_uthread_cleanup (ppointer data)
{
	p_uthread_unref (data);
}

static ppointer
__p_uthread_proxy (ppointer data)
{
	__PUThreadBase* base_thread = data;

	p_uthread_set_local (__p_uthread_specific_data, data);

	p_spinlock_lock (__p_uthread_new_spin);
	p_spinlock_unlock (__p_uthread_new_spin);

	base_thread->func (base_thread->data);

	return NULL;
}

void
__p_uthread_init (void)
{
	if (P_LIKELY (__p_uthread_specific_data == NULL))
		__p_uthread_specific_data = p_uthread_local_new ((PDestroyFunc) __p_uthread_cleanup);

	if (P_LIKELY (__p_uthread_new_spin == NULL))
		__p_uthread_new_spin = p_spinlock_new ();

	__p_uthread_init_internal ();
}

void
__p_uthread_shutdown (void)
{
	if (P_LIKELY (__p_uthread_specific_data != NULL)) {
		p_uthread_local_free (__p_uthread_specific_data);
		__p_uthread_specific_data = NULL;
	}

	if (P_LIKELY (__p_uthread_new_spin != NULL)) {
		p_spinlock_free (__p_uthread_new_spin);
		__p_uthread_new_spin = NULL;
	}

	__p_uthread_shutdown_internal ();
}

P_LIB_API PUThread *
p_uthread_create_full (PUThreadFunc	func,
		       ppointer		data,
		       pboolean		joinable,
		       PUThreadPriority	prio)
{
	__PUThreadBase *base_thread;

	if (P_UNLIKELY (func == NULL))
		return NULL;

	p_spinlock_lock (__p_uthread_new_spin);

	base_thread = (__PUThreadBase *) __p_uthread_create_internal (__p_uthread_proxy,
								      joinable,
								      prio);

	if (P_LIKELY (base_thread != NULL)) {
		base_thread->ref_count = 2;
		base_thread->ours      = TRUE;
		base_thread->joinable  = joinable;
		base_thread->func      = func;
		base_thread->data      = data;
	}

	p_spinlock_unlock (__p_uthread_new_spin);

	return (PUThread *) base_thread;
}

P_LIB_API PUThread *
p_uthread_create (PUThreadFunc	func,
		  ppointer	data,
		  pboolean	joinable)
{
	/* All checks will be inside */
	return p_uthread_create_full (func, data, joinable, P_UTHREAD_PRIORITY_INHERIT);
}

P_LIB_API void
p_uthread_exit (pint code)
{
	__PUThreadBase *base_thread = (__PUThreadBase *) p_uthread_current ();

	if (P_UNLIKELY (base_thread == NULL))
		return;

	if (P_UNLIKELY (base_thread->ours == FALSE)) {
		P_WARNING ("Trying to call p_uthread_exit() from an unknown thread");
		return;
	}

	base_thread->ret_code = code;

	__p_uthread_exit_internal ();
}

P_LIB_API pint
p_uthread_join (PUThread *thread)
{
	__PUThreadBase *base_thread;

	if (P_UNLIKELY (thread == NULL))
		return -1;

	 base_thread = (__PUThreadBase *) thread;

	if (base_thread->joinable == FALSE)
		return -1;

	__p_uthread_wait_internal (thread);

	return base_thread->ret_code;
}

P_LIB_API PUThread *
p_uthread_current (void)
{
	__PUThreadBase *base_thread = p_uthread_get_local (__p_uthread_specific_data);

	if (P_UNLIKELY (base_thread == NULL)) {
		if (P_UNLIKELY ((base_thread = p_malloc0 (sizeof (__PUThreadBase))) == NULL)) {
			P_ERROR ("PUThread: failed to allocate memory for a thread local structure");
			return NULL;
		}

		base_thread->ref_count = 1;

		p_uthread_set_local (__p_uthread_specific_data, base_thread);
	}

	return (PUThread *) base_thread;
}

P_LIB_API void
p_uthread_ref (PUThread *thread)
{
	if (P_UNLIKELY (thread == NULL))
		return;

	p_atomic_int_inc (&((__PUThreadBase *) thread)->ref_count);
}

P_LIB_API void
p_uthread_unref (PUThread *thread)
{
	__PUThreadBase *base_thread;

	if (P_UNLIKELY (thread == NULL))
		return;

	base_thread = (__PUThreadBase *) thread;

	if (p_atomic_int_dec_and_test (&base_thread->ref_count) == TRUE) {
		if (base_thread->ours == TRUE)
			__p_uthread_free_internal (thread);
		else
			p_free (thread);
	}
}

#ifdef P_OS_WIN
#  include <windows.h>
#else
#  include <errno.h>
#  if !defined (PLIBSYS_HAS_NANOSLEEP)
#    include <sys/select.h>
#    include <sys/time.h>
static int __p_uthread_nanosleep (puint32 msec)
{
	int		rc;
	struct timeval	tstart, tstop, tremain, time2wait;

	time2wait.tv_sec  = msec / 1000;
	time2wait.tv_usec = (msec % 1000) * 1000;

	if (gettimeofday (&tstart, NULL) != 0)
		return -1;

	rc = -1;

	while (rc != 0) {
		if (P_UNLIKELY ((rc = select (0, NULL, NULL, NULL, &time2wait)) != 0)) {
			if (__p_error_get_last_error () == EINTR) {
				if (gettimeofday (&tstop, NULL) != 0)
					return -1;

				tremain.tv_sec = time2wait.tv_sec -
						 (tstop.tv_sec - tstart.tv_sec);
				tremain.tv_usec = time2wait.tv_usec -
						  (tstop.tv_usec - tstart.tv_usec);
				tremain.tv_sec += tremain.tv_usec / 1000000L;
				tremain.tv_usec %= 1000000L;
			} else
				return -1;
		}
	}

	return 0;
}
#  endif
#endif

P_LIB_API pint
p_uthread_sleep (puint32 msec)
{
#ifdef P_OS_WIN
	Sleep (msec);
	return 0;
#elif defined (PLIBSYS_HAS_NANOSLEEP)
	pint result;
	struct timespec time_req;
	struct timespec time_rem;

	memset (&time_rem, 0, sizeof (struct timespec));

	time_req.tv_nsec = (msec % 1000) * 1000000L;
	time_req.tv_sec = (time_t) (msec / 1000);

	result = -1;
	while (result != 0) {
		if (P_UNLIKELY ((result = nanosleep (&time_req, &time_rem)) != 0)) {
			if (__p_error_get_last_error () == EINTR)
				time_req = time_rem;
			else
				return -1;
		}
	}

	return 0;
#else
	return __p_uthread_nanosleep (msec);
#endif
}
