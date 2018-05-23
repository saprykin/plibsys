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

void
p_rwlock_init (void)
{
}

void
p_rwlock_shutdown (void)
{
}

