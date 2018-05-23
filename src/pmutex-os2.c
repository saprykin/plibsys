/*
 * The MIT License
 *
 * Copyright (C) 2017 Alexander Saprykin <saprykin.spb@gmail.com>
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

#include "pmem.h"
#include "pmutex.h"

#include <stdlib.h>

#define INCL_DOSSEMAPHORES
#define INCL_DOSERRORS
#include <os2.h>

typedef HMTX mutex_hdl;

struct PMutex_ {
	mutex_hdl	hdl;
};

P_LIB_API PMutex *
p_mutex_new (void)
{
	PMutex *ret;

	if (P_UNLIKELY ((ret = p_malloc0 (sizeof (PMutex))) == NULL)) {
		P_ERROR ("PMutex::p_mutex_new: failed to allocate memory");
		return NULL;
	}

	if (P_UNLIKELY (DosCreateMutexSem (NULL, (PHMTX) &ret->hdl, 0, FALSE) != NO_ERROR)) {
		P_ERROR ("PMutex::p_mutex_new: DosCreateMutexSem() failed");
		p_free (ret);
		return NULL;
	}

	return ret;
}

P_LIB_API pboolean
p_mutex_lock (PMutex *mutex)
{
	APIRET ulrc;

	if (P_UNLIKELY (mutex == NULL))
		return FALSE;

	while ((ulrc = DosRequestMutexSem (mutex->hdl, SEM_INDEFINITE_WAIT)) == ERROR_INTERRUPT)
		;

	if (P_LIKELY (ulrc == NO_ERROR))
		return TRUE;
	else {
		P_ERROR ("PMutex::p_mutex_lock: DosRequestMutexSem() failed");
		return FALSE;
	}
}

P_LIB_API pboolean
p_mutex_trylock (PMutex *mutex)
{
	if (P_UNLIKELY (mutex == NULL))
		return FALSE;

	return (DosRequestMutexSem (mutex->hdl, SEM_IMMEDIATE_RETURN)) == NO_ERROR ? TRUE : FALSE;
}

P_LIB_API pboolean
p_mutex_unlock (PMutex *mutex)
{
	if (P_UNLIKELY (mutex == NULL))
		return FALSE;

	if (P_LIKELY (DosReleaseMutexSem (mutex->hdl) == NO_ERROR))
		return TRUE;
	else {
		P_ERROR ("PMutex::p_mutex_unlock: DosReleaseMutexSem() failed");
		return FALSE;
	}
}

P_LIB_API void
p_mutex_free (PMutex *mutex)
{
	if (P_UNLIKELY (mutex == NULL))
		return;

	if (P_UNLIKELY (DosCloseMutexSem (mutex->hdl) != NO_ERROR))
		P_ERROR ("PMutex::p_mutex_free: DosCloseMutexSem() failed");

	p_free (mutex);
}
