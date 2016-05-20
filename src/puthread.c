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

#include "puthread.h"
#include "plibsys-private.h"

#include <stdlib.h>
#include <time.h>
#include <string.h>

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
