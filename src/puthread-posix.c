/* 
 * 15.09.2010
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

/* TODO: conditional variables */
/* TODO: priorities */
/* TODO: once routines */
/* TODO: barriers */
/* TODO: _full version of create func */

#include "pmem.h"
#include "puthread.h"

#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <pthread.h>

typedef pthread_t puthread_hdl;

struct _PUThread {
	puthread_hdl hdl;
	pboolean joinable;
};

P_LIB_API PUThread *
p_uthread_create (PUThreadFunc func, ppointer data, pboolean joinable)
{
	PUThread	*ret;
	pthread_attr_t	attr;

	if (!func)
		return NULL;

	if ((ret = p_malloc0 (sizeof (PUThread))) == NULL) {
		P_ERROR ("PUThread: failed to allocate memory");
		return NULL;
	}

	ret->joinable = joinable;

	if (pthread_attr_init (&attr) != 0) {
		P_ERROR ("PUThread: failed to init thread attributes");
		p_free (ret);
		return NULL;
	}

	if (pthread_attr_setdetachstate (&attr, joinable ? PTHREAD_CREATE_JOINABLE : PTHREAD_CREATE_DETACHED) != 0) {
		P_ERROR ("PUTread: failed to set joinable/detached attribute");
		pthread_attr_destroy (&attr);
		p_free (ret);
		return NULL;
	}

	/* We need this for some old systems where non-bound threads are not timesliced,
	 * see puthread-sun.c for more explanation */
	if (pthread_attr_setscope (&attr, PTHREAD_SCOPE_SYSTEM) != 0) {
		P_ERROR ("PUTread: failed to set contention attribute");
		pthread_attr_destroy (&attr);
		p_free (ret);
		return NULL;
	}
	
	if (pthread_create (&ret->hdl, &attr, func, data) != 0) {
		P_ERROR ("PUThread: failed to create thread");
		pthread_attr_destroy (&attr);
		p_free (ret);
		return NULL;
	}

	pthread_attr_destroy (&attr);
	return ret;
}

P_LIB_API P_NO_RETURN void
p_uthread_exit (pint code)
{
	pthread_exit (P_INT_TO_POINTER (code));
}

P_LIB_API pint
p_uthread_join (PUThread *thread)
{
	ppointer ret;

	if (!thread || !thread->joinable)
		return -1;

	if (pthread_join (thread->hdl, &ret) != 0) {
		P_ERROR ("PUThread: failed to join thread");
		return -1;
	}

	return P_POINTER_TO_INT (ret);
}

P_LIB_API void
p_uthread_free (PUThread *thread)
{
	if (!thread)
		return;

	p_free (thread);
}

P_LIB_API void
p_uthread_yield (void)
{
	sched_yield ();
}

