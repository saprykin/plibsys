/* 
 * 29.12.2010
 * Copyright (C) 2010 Alexander Saprykin <xelfium@gmail.com>
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

#include "pmem.h"
#include "pmutex.h"

#include <stdlib.h>

#define p_static_mutex_get_mutex_impl(mutex) NULL

struct _PMutex {
	pint	hdl;
};

P_LIB_API PMutex *
p_mutex_new (void)
{
	return NULL;
}

P_LIB_API pboolean
p_mutex_lock (PMutex *mutex)
{
	return FALSE;
}

P_LIB_API pboolean
p_mutex_trylock (PMutex *mutex)
{
	return FALSE;
}

P_LIB_API pboolean
p_mutex_unlock (PMutex *mutex)
{
	return FALSE;
}

P_LIB_API void
p_mutex_free (PMutex *mutex)
{
}

P_LIB_API void
p_static_mutex_init (PStaticMutex *mutex)
{
	mutex = NULL;
}

P_LIB_API void
p_static_mutex_free (PStaticMutex* mutex)
{
}

