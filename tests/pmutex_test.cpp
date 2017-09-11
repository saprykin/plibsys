/*
 * Copyright (C) 2013-2017 Alexander Saprykin <xelfium@gmail.com>
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

#include "plibsys.h"
#include "ptestmacros.h"

P_TEST_MODULE_INIT ();

static pint mutex_test_val  = 0;
static PMutex *global_mutex = NULL;

extern "C" ppointer pmem_alloc (psize nbytes)
{
	P_UNUSED (nbytes);
	return (ppointer) NULL;
}

extern "C" ppointer pmem_realloc (ppointer block, psize nbytes)
{
	P_UNUSED (block);
	P_UNUSED (nbytes);
	return (ppointer) NULL;
}

extern "C" void pmem_free (ppointer block)
{
	P_UNUSED (block);
}

static void * mutex_test_thread (void *)
{
	pint i;

	for (i = 0; i < 1000; ++i) {
		if (!p_mutex_trylock (global_mutex)) {
			if (!p_mutex_lock (global_mutex))
				p_uthread_exit (1);
		}

		if (mutex_test_val == 10)
			--mutex_test_val;
		else {
			p_uthread_sleep (1);
			++mutex_test_val;
		}

		if (!p_mutex_unlock (global_mutex))
			p_uthread_exit (1);
	}

	p_uthread_exit (0);

	return NULL;
}

P_TEST_CASE_BEGIN (pmutex_nomem_test)
{
	p_libsys_init ();

	PMemVTable vtable;

	vtable.free    = pmem_free;
	vtable.malloc  = pmem_alloc;
	vtable.realloc = pmem_realloc;

	P_TEST_CHECK (p_mem_set_vtable (&vtable) == TRUE);
	P_TEST_CHECK (p_mutex_new () == NULL);

	p_mem_restore_vtable ();

	p_libsys_shutdown ();
}
P_TEST_CASE_END ()

P_TEST_CASE_BEGIN (pmutex_bad_input_test)
{
	p_libsys_init ();

	P_TEST_REQUIRE (p_mutex_lock (NULL) == FALSE);
	P_TEST_REQUIRE (p_mutex_unlock (NULL) == FALSE);
	P_TEST_REQUIRE (p_mutex_trylock (NULL) == FALSE);
	p_mutex_free (NULL);

	p_libsys_shutdown ();
}
P_TEST_CASE_END ()

P_TEST_CASE_BEGIN (pmutex_general_test)
{
	PUThread *thr1, *thr2;

	p_libsys_init ();

	global_mutex = p_mutex_new ();
	P_TEST_REQUIRE (global_mutex != NULL);

	mutex_test_val = 10;

	thr1 = p_uthread_create ((PUThreadFunc) mutex_test_thread, NULL, true);
	P_TEST_REQUIRE (thr1 != NULL);

	thr2 = p_uthread_create ((PUThreadFunc) mutex_test_thread, NULL, true);
	P_TEST_REQUIRE (thr2 != NULL);

	P_TEST_CHECK (p_uthread_join (thr1) == 0);
	P_TEST_CHECK (p_uthread_join (thr2) == 0);

	P_TEST_REQUIRE (mutex_test_val == 10);

	p_uthread_unref (thr1);
	p_uthread_unref (thr2);
	p_mutex_free (global_mutex);

	p_libsys_shutdown ();
}
P_TEST_CASE_END ()

P_TEST_SUITE_BEGIN()
{
	P_TEST_SUITE_RUN_CASE (pmutex_nomem_test);
	P_TEST_SUITE_RUN_CASE (pmutex_bad_input_test);
	P_TEST_SUITE_RUN_CASE (pmutex_general_test);
}
P_TEST_SUITE_END()
