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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA.
 */

#include "puthread.h"
#include "plib-private.h"

#include <stdlib.h>
#include <time.h>
#include <string.h>

#ifdef P_OS_WIN
#include <windows.h>
#else
#include <errno.h>
#endif

P_LIB_API pint
p_uthread_sleep (puint32 msec)
{
#ifdef P_OS_WIN
	Sleep (msec);
#else
	pint result;
	struct timespec time_req;
	struct timespec time_rem;

	memset (&time_rem, 0, sizeof (struct timespec));

	time_req.tv_nsec = (msec % 1000) * 1000000L;
	time_req.tv_sec = (time_t) (msec / 1000);

	result = -1;
	while (result != 0) {
		if ((result = nanosleep (&time_req, &time_rem)) != 0) {
			if (__p_error_get_last_error () == EINTR)
				time_req = time_rem;
			else
				return -1;
		}
	}
#endif
	return 0;
}
