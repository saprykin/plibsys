/*
 * The MIT License
 *
 * Copyright (C) 2010-2018 Alexander Saprykin <saprykin.spb@gmail.com>
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
#ifndef P_OS_WIN
#  include "perror.h"
#endif
#include "pmem.h"
#include "pspinlock.h"
#include "puthread.h"
#include "puthread-private.h"

#ifdef P_OS_OS2
#  define INCL_DOSPROCESS
#  define INCL_DOSERRORS
#  define INCL_DOSMISC
#  include <os2.h>
#endif

#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifndef P_OS_WIN
#  include <unistd.h>
#endif

#ifdef P_OS_WIN
typedef void (WINAPI * SystemInfoFunc) (LPSYSTEM_INFO);
#endif

#ifdef P_OS_HPUX
#  include <sys/pstat.h>
#endif

#ifdef P_OS_BSD4
#  include <sys/param.h>
#  include <sys/types.h>
#  include <sys/sysctl.h>
#endif

#ifdef P_OS_VMS
#  define __NEW_STARLET 1
#  include <starlet.h>
#  include <ssdef.h>
#  include <stsdef.h>
#  include <efndef.h>
#  include <iledef.h>
#  include <iosbdef.h>
#  include <syidef.h>
#  include <tis.h>
#  include <lib$routines.h>
#endif

#ifdef P_OS_QNX6
#  include <sys/syspage.h>
#endif

#ifdef P_OS_BEOS
#  include <kernel/OS.h>
#endif

#ifdef P_OS_SYLLABLE
#  include <atheos/sysinfo.h>
#endif

#if defined (P_OS_SCO) && !defined (_SC_NPROCESSORS_ONLN)
#  include <sys/utsname.h>
#endif

#ifdef P_OS_AMIGA
#  include <clib/alib_protos.h> 
#  include <proto/dos.h>
#  include <proto/exec.h>
#endif

extern void p_uthread_init_internal (void);
extern void p_uthread_shutdown_internal (void);
extern void p_uthread_exit_internal (void);
extern void p_uthread_wait_internal (PUThread *thread);
extern void p_uthread_free_internal (PUThread *thread);
extern PUThread * p_uthread_create_internal (PUThreadFunc	func,
					     pboolean		joinable,
					     PUThreadPriority	prio,
					     psize		stack_size);

static void pp_uthread_cleanup (ppointer data);
static ppointer pp_uthread_proxy (ppointer data);

#ifndef P_OS_WIN
#  if !defined (PLIBSYS_HAS_CLOCKNANOSLEEP) && !defined (PLIBSYS_HAS_NANOSLEEP)
static pint pp_uthread_nanosleep (puint32 msec);
#  endif
#endif

static PUThreadKey * pp_uthread_specific_data = NULL;
static PSpinLock * pp_uthread_new_spin = NULL;

static void
pp_uthread_cleanup (ppointer data)
{
	p_uthread_unref (data);
}

static ppointer
pp_uthread_proxy (ppointer data)
{
	PUThreadBase *base_thread = data;

	p_uthread_set_local (pp_uthread_specific_data, data);

	p_spinlock_lock (pp_uthread_new_spin);
	p_spinlock_unlock (pp_uthread_new_spin);

	base_thread->func (base_thread->data);

	return NULL;
}

void
p_uthread_init (void)
{
	if (P_LIKELY (pp_uthread_specific_data == NULL))
		pp_uthread_specific_data = p_uthread_local_new ((PDestroyFunc) pp_uthread_cleanup);

	if (P_LIKELY (pp_uthread_new_spin == NULL))
		pp_uthread_new_spin = p_spinlock_new ();

	p_uthread_init_internal ();
}

void
p_uthread_shutdown (void)
{
	PUThread *cur_thread;

	if (P_LIKELY (pp_uthread_specific_data != NULL)) {
		cur_thread = p_uthread_get_local (pp_uthread_specific_data);

		if (P_UNLIKELY (cur_thread != NULL)) {
			p_uthread_unref (cur_thread);
			p_uthread_set_local (pp_uthread_specific_data, NULL);
		}

		p_uthread_local_free (pp_uthread_specific_data);
		pp_uthread_specific_data = NULL;
	}

	if (P_LIKELY (pp_uthread_new_spin != NULL)) {
		p_spinlock_free (pp_uthread_new_spin);
		pp_uthread_new_spin = NULL;
	}

	p_uthread_shutdown_internal ();
}

P_LIB_API PUThread *
p_uthread_create_full (PUThreadFunc	func,
		       ppointer		data,
		       pboolean		joinable,
		       PUThreadPriority	prio,
		       psize		stack_size)
{
	PUThreadBase *base_thread;

	if (P_UNLIKELY (func == NULL))
		return NULL;

	p_spinlock_lock (pp_uthread_new_spin);

	base_thread = (PUThreadBase *) p_uthread_create_internal (pp_uthread_proxy,
								  joinable,
								  prio,
								  stack_size);

	if (P_LIKELY (base_thread != NULL)) {
		base_thread->ref_count = 2;
		base_thread->ours      = TRUE;
		base_thread->joinable  = joinable;
		base_thread->func      = func;
		base_thread->data      = data;
	}

	p_spinlock_unlock (pp_uthread_new_spin);

	return (PUThread *) base_thread;
}

P_LIB_API PUThread *
p_uthread_create (PUThreadFunc	func,
		  ppointer	data,
		  pboolean	joinable)
{
	/* All checks will be inside */
	return p_uthread_create_full (func, data, joinable, P_UTHREAD_PRIORITY_INHERIT, 0);
}

P_LIB_API void
p_uthread_exit (pint code)
{
	PUThreadBase *base_thread = (PUThreadBase *) p_uthread_current ();

	if (P_UNLIKELY (base_thread == NULL))
		return;

	if (P_UNLIKELY (base_thread->ours == FALSE)) {
		P_WARNING ("PUThread::p_uthread_exit: p_uthread_exit() cannot be called from an unknown thread");
		return;
	}

	base_thread->ret_code = code;

	p_uthread_exit_internal ();
}

P_LIB_API pint
p_uthread_join (PUThread *thread)
{
	PUThreadBase *base_thread;

	if (P_UNLIKELY (thread == NULL))
		return -1;

	base_thread = (PUThreadBase *) thread;

	if (base_thread->joinable == FALSE)
		return -1;

	p_uthread_wait_internal (thread);

	return base_thread->ret_code;
}

P_LIB_API PUThread *
p_uthread_current (void)
{
	PUThreadBase *base_thread = p_uthread_get_local (pp_uthread_specific_data);

	if (P_UNLIKELY (base_thread == NULL)) {
		if (P_UNLIKELY ((base_thread = p_malloc0 (sizeof (PUThreadBase))) == NULL)) {
			P_ERROR ("PUThread::p_uthread_current: failed to allocate memory");
			return NULL;
		}

		base_thread->ref_count = 1;

		p_uthread_set_local (pp_uthread_specific_data, base_thread);
	}

	return (PUThread *) base_thread;
}

P_LIB_API pint
p_uthread_ideal_count (void)
{
#if defined (P_OS_WIN)
	SYSTEM_INFO	sys_info;
	SystemInfoFunc	sys_info_func;

	sys_info_func = (SystemInfoFunc) GetProcAddress (GetModuleHandleA ("kernel32.dll"),
							 "GetNativeSystemInfo");

	if (P_UNLIKELY (sys_info_func == NULL))
		sys_info_func = (SystemInfoFunc) GetProcAddress (GetModuleHandleA ("kernel32.dll"),
								 "GetSystemInfo");

	if (P_UNLIKELY (sys_info_func == NULL)) {
		P_ERROR ("PUThread::p_uthread_ideal_count: failed to get address of system info procedure");
		return 1;
	}

	sys_info_func (&sys_info);

	return (pint) sys_info.dwNumberOfProcessors;
#elif defined (P_OS_HPUX)
	struct pst_dynamic psd;

	if (P_LIKELY (pstat_getdynamic (&psd, sizeof (psd), 1, 0) != -1))
		return (pint) psd.psd_proc_cnt;
	else {
		P_WARNING ("PUThread::p_uthread_ideal_count: failed to call pstat_getdynamic()");
		return 1;
	}
#elif defined (P_OS_IRIX)
	pint cores;

	cores = sysconf (_SC_NPROC_ONLN);

	if (P_UNLIKELY (cores < 0)) {
		P_WARNING ("PUThread::p_uthread_ideal_count: failed to call sysconf(_SC_NPROC_ONLN)");
		cores = 1;
	}

	return cores;
#elif defined (P_OS_BSD4)
	pint	cores;
	pint	mib[2];
	size_t	len = sizeof (cores);

	mib[0] = CTL_HW;
	mib[1] = HW_NCPU;

	if (P_UNLIKELY (sysctl (mib, 2, &cores, &len, NULL, 0) == -1)) {
		P_WARNING ("PUThread::p_uthread_ideal_count: failed to call sysctl()");
		return 1;
	}

	return (pint) cores;
#elif defined (P_OS_VMS)
	pint	cores;
	pint	status;
	puint	efn;
	IOSB	iosb;
#  if (PLIBSYS_SIZEOF_VOID_P == 4)
	ILE3	itmlst[] = { { sizeof (cores), SYI$_AVAILCPU_CNT, &cores, NULL},
			     { 0, 0, NULL, NULL}
			   };
#  else
	ILEB_64	itmlst[] = { { 1, SYI$_AVAILCPU_CNT, -1, sizeof (cores), &cores, NULL},
			     { 0, 0, 0, 0, NULL, NULL}
			   };
#  endif

	status = lib$get_ef (&efn);

	if (P_UNLIKELY (!$VMS_STATUS_SUCCESS (status))) {
		P_WARNING ("PUThread::p_uthread_ideal_count: failed to call lib$get_ef()");
		return 1;
	}

	status = sys$getsyi (efn, NULL, NULL, itmlst, &iosb, tis_io_complete, 0);

	if (P_UNLIKELY (!$VMS_STATUS_SUCCESS (status))) {
		P_WARNING ("PUThread::p_uthread_ideal_count: failed to call sys$getsyiw()");
		lib$free_ef (&efn);
		return 1;
	}

	status = tis_synch (efn, &iosb);

	if (P_UNLIKELY (!$VMS_STATUS_SUCCESS (status))) {
		P_WARNING ("PUThread::p_uthread_ideal_count: failed to call tis_synch()");
		lib$free_ef (&efn);
		return 1;
	}

	if (P_UNLIKELY (iosb.iosb$l_getxxi_status != SS$_NORMAL)) {
		P_WARNING ("PUThread::p_uthread_ideal_count: l_getxxi_status is not normal");
		lib$free_ef (&efn);
		return 1;
	}

	lib$free_ef (&efn);

	return cores;
#elif defined (P_OS_OS2)
	APIRET	ulrc;
	ULONG	cores;

	if (P_UNLIKELY (DosQuerySysInfo (QSV_NUMPROCESSORS,
					 QSV_NUMPROCESSORS,
					 &cores,
					 sizeof (cores)) != NO_ERROR)) {
		P_WARNING ("PUThread::p_uthread_ideal_count: failed to call DosQuerySysInfo()");
		return 1;
	}

	return (pint) cores;
#elif defined (P_OS_QNX6)
	return (pint) _syspage_ptr->num_cpu;
#elif defined (P_OS_BEOS)
	system_info sys_info;

	get_system_info (&sys_info);

	return (pint) sys_info.cpu_count;
#elif defined (P_OS_SYLLABLE)
	system_info sys_info;

	if (P_UNLIKELY (get_system_info_v (&sys_info, SYS_INFO_VERSION) != 0)) {
		P_WARNING ("PUThread::p_uthread_ideal_count: failed to call get_system_info_v()");
		return 1;
	}

	return (pint) sys_info.nCPUCount;
#elif defined (P_OS_AMIGA)
	puint32 cores;

	IExec->GetCPUInfoTags (GCIT_NumberOfCPUs, &cores, TAG_END);

	return (pint) cores;
#elif defined (P_OS_SCO) && !defined (_SC_NPROCESSORS_ONLN)
	struct scoutsname utsn;

	if (P_UNLIKELY (__scoinfo (&utsn, sizeof (utsn)) == -1)) {
		P_ERROR ("PUThread::p_uthread_ideal_count: failed to call __scoinfo()");
		return 1;
	}

	return (pint) utsn.numcpu;
#elif defined (_SC_NPROCESSORS_ONLN)
	pint cores;

	cores = (pint) sysconf (_SC_NPROCESSORS_ONLN);

	if (P_UNLIKELY (cores  == -1)) {
		P_WARNING ("PUThread::p_uthread_ideal_count: failed to call sysconf(_SC_NPROCESSORS_ONLN)");
		return 1;
	}

	return cores;
#else
	return 1;
#endif
}

P_LIB_API void
p_uthread_ref (PUThread *thread)
{
	if (P_UNLIKELY (thread == NULL))
		return;

	p_atomic_int_inc (&((PUThreadBase *) thread)->ref_count);
}

P_LIB_API void
p_uthread_unref (PUThread *thread)
{
	PUThreadBase *base_thread;

	if (P_UNLIKELY (thread == NULL))
		return;

	base_thread = (PUThreadBase *) thread;

	if (p_atomic_int_dec_and_test (&base_thread->ref_count) == TRUE) {
		if (base_thread->ours == TRUE)
			p_uthread_free_internal (thread);
		else
			p_free (thread);
	}
}

#ifndef P_OS_WIN
#  include <errno.h>
#  if !defined (PLIBSYS_HAS_CLOCKNANOSLEEP) && !defined (PLIBSYS_HAS_NANOSLEEP)
#    include <sys/select.h>
#    include <sys/time.h>
static pint pp_uthread_nanosleep (puint32 msec)
{
	pint		rc;
	struct timeval	tstart, tstop, tremain, time2wait;

	time2wait.tv_sec  = msec / 1000;
	time2wait.tv_usec = (msec % 1000) * 1000;

	if (P_UNLIKELY (gettimeofday (&tstart, NULL) != 0))
		return -1;

	rc = -1;

	while (rc != 0) {
		if (P_UNLIKELY ((rc = select (0, NULL, NULL, NULL, &time2wait)) != 0)) {
			if (p_error_get_last_system () == EINTR) {
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
#if defined (P_OS_WIN)
	Sleep (msec);
	return 0;
#elif defined (P_OS_OS2)
	return (DosSleep (msec) == NO_ERROR) ? 0 : -1;
#elif defined (P_OS_AMIGA)
	return TimeDelay (0, msec / 1000, (msec % 1000) * 1000) == 0 ? 0 : -1;
#elif defined (PLIBSYS_HAS_CLOCKNANOSLEEP) || defined (PLIBSYS_HAS_NANOSLEEP)
	pint result;
	struct timespec time_req;
	struct timespec time_rem;

	memset (&time_rem, 0, sizeof (struct timespec));

	time_req.tv_nsec = (msec % 1000) * 1000000L;
	time_req.tv_sec  = (time_t) (msec / 1000);

	result = -1;
	while (result != 0) {
		/* Syllable has unimplemented clock_nanocleep() call */
#  if defined (PLIBSYS_HAS_CLOCKNANOSLEEP) && !defined (P_OS_SYLLABLE)
		if (P_UNLIKELY ((result = clock_nanosleep (CLOCK_MONOTONIC,
							   0,
							   &time_req,
							   &time_rem)) != 0)) {
#  else
		if (P_UNLIKELY ((result = nanosleep (&time_req, &time_rem)) != 0)) {
#  endif
			if (p_error_get_last_system () == EINTR)
				time_req = time_rem;
			else
				return -1;
		}
	}

	return 0;
#else
	return pp_uthread_nanosleep (msec);
#endif
}
