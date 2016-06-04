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

#include "prwlock.h"

#include <stdlib.h>

struct PRWLock_ {
	pint hdl;
};

P_LIB_API PRWLock *
p_rwlock_new (void)
{
	return NULL;
}

P_LIB_API pboolean
p_rwlock_reader_lock (PRWLock *lock)
{
	P_UNUSED (lock);

	return FALSE;
}

P_LIB_API pboolean
p_rwlock_reader_trylock (PRWLock *lock)
{
	P_UNUSED (lock);

	return FALSE;
}

P_LIB_API pboolean
p_rwlock_reader_unlock (PRWLock *lock)
{
	P_UNUSED (lock);

	return FALSE;
}

P_LIB_API pboolean
p_rwlock_writer_lock (PRWLock *lock)
{
	P_UNUSED (lock);

	return FALSE;
}

P_LIB_API pboolean
p_rwlock_writer_trylock (PRWLock *lock)
{
	P_UNUSED (lock);

	return FALSE;
}

P_LIB_API pboolean
p_rwlock_writer_unlock (PRWLock *lock)
{
	P_UNUSED (lock);

	return FALSE;
}

P_LIB_API void
p_rwlock_free (PRWLock *lock)
{
	P_UNUSED (lock);
}
