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

#include "perror-private.h"
#include "psysclose-private.h"

#include <unistd.h>
#include <errno.h>

pint
p_sys_close (pint fd)
{
#if defined (EINTR) && defined (P_OS_HPUX)
	pint res, err_code;

	for (;;) {
		res = close (fd);

		if (P_LIKELY (res == 0))
			return 0;

		err_code = p_error_get_last_error ();

		if (err_code == EINTR)
			continue;
		else
			return -1;
	}
#else
	return close (fd);
#endif
}
