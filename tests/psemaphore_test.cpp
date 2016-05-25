/*
 * Copyright (C) 2013-2016 Alexander Saprykin <xelfium@gmail.com>
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

#ifndef PLIBSYS_TESTS_STATIC
#  define BOOST_TEST_DYN_LINK
#endif

#define BOOST_TEST_MODULE psemaphore_test

#include "plibsys.h"

#ifdef PLIBSYS_TESTS_STATIC
#  include <boost/test/included/unit_test.hpp>
#else
#  include <boost/test/unit_test.hpp>
#endif

#ifndef P_OS_MSYS
static pint semaphore_test_val	= 10;
static pint is_thread_exit	= 0;

static void clean_error (PError **error)
{
	if (error == NULL || *error == NULL)
		return;

	p_error_free (*error);
	*error = NULL;
}

static void * semaphore_test_thread (void *)
{
	PSemaphore	*sem;
	pint		i;

	sem = p_semaphore_new ("p_semaphore_test_object", 1, P_SEM_ACCESS_OPEN, NULL);

	if (sem == NULL)
		p_uthread_exit (1);

	for (i = 0; i < 1000; ++i) {
		if (!p_semaphore_acquire (sem, NULL)) {
			if (is_thread_exit > 0) {
				semaphore_test_val = 10;
				break;
			}

			p_uthread_exit (1);
		}

		if (semaphore_test_val == 10)
			--semaphore_test_val;
		else {
			p_uthread_sleep (1);
			++semaphore_test_val;
		}

		if (!p_semaphore_release (sem, NULL)) {
			if (is_thread_exit > 0) {
				semaphore_test_val = 10;
				break;
			}

			p_uthread_exit (1);
		}
	}

	++is_thread_exit;

	p_semaphore_free (sem);
	p_uthread_exit (0);

	return NULL;
}
#endif /* !P_OS_MSYS */

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


BOOST_AUTO_TEST_SUITE (BOOST_TEST_MODULE)

BOOST_AUTO_TEST_CASE (psemaphore_nomem_test)
{
	p_libsys_init ();

	PMemVTable vtable;

	vtable.free	= pmem_free;
	vtable.malloc	= pmem_alloc;
	vtable.realloc	= pmem_realloc;

	BOOST_CHECK (p_mem_set_vtable (&vtable) == TRUE);

	BOOST_CHECK (p_semaphore_new ("p_semaphore_test_object", 1, P_SEM_ACCESS_CREATE, NULL) == NULL);

	p_mem_restore_vtable ();

	p_libsys_shutdown ();
}

BOOST_AUTO_TEST_CASE (psemaphore_general_test)
{
#ifndef P_OS_MSYS
	PSemaphore	*sem = NULL;
	PError		*error = NULL;
	pint		i;

	p_libsys_init ();

	BOOST_CHECK (p_semaphore_new (NULL, 0, P_SEM_ACCESS_CREATE, &error) == NULL);
	BOOST_CHECK (error != NULL);
	clean_error (&error);

	BOOST_REQUIRE (p_semaphore_acquire (sem, &error) == FALSE);
	BOOST_CHECK (error != NULL);
	clean_error (&error);

	BOOST_REQUIRE (p_semaphore_release (sem, &error) == FALSE);
	BOOST_CHECK (error != NULL);
	clean_error (&error);

	p_semaphore_take_ownership (sem);
	p_semaphore_free (NULL);

	sem = p_semaphore_new ("p_semaphore_test_object", 10, P_SEM_ACCESS_CREATE, NULL);
	BOOST_REQUIRE (sem != NULL);
	p_semaphore_take_ownership (sem);
	p_semaphore_free (sem);

	sem = p_semaphore_new ("p_semaphore_test_object", 10, P_SEM_ACCESS_CREATE, NULL);
	BOOST_REQUIRE (sem != NULL);

	for (i = 0; i < 10; ++i)
		BOOST_CHECK (p_semaphore_acquire (sem, NULL));

	for (i = 0; i < 10; ++i)
		BOOST_CHECK (p_semaphore_release (sem, NULL));

	for (i = 0; i < 1000; ++i) {
		BOOST_CHECK (p_semaphore_acquire (sem, NULL));
		BOOST_CHECK (p_semaphore_release (sem, NULL));
	}

	p_semaphore_free (sem);

	p_libsys_shutdown ();
#endif /* !P_OS_MSYS */
}

BOOST_AUTO_TEST_CASE (psemaphore_thread_test)
{
#ifndef P_OS_MSYS
	PUThread	*thr1, *thr2;
	PSemaphore	*sem = NULL;

	p_libsys_init ();

	sem = p_semaphore_new ("p_semaphore_test_object", 10, P_SEM_ACCESS_CREATE, NULL);
	BOOST_REQUIRE (sem != NULL);
	p_semaphore_take_ownership (sem);
	p_semaphore_free (sem);

	sem = NULL;

	thr1 = p_uthread_create ((PUThreadFunc) semaphore_test_thread, NULL, true);
	BOOST_REQUIRE (thr1 != NULL);

	thr2 = p_uthread_create ((PUThreadFunc) semaphore_test_thread, NULL, true);
	BOOST_REQUIRE (thr2 != NULL);

	BOOST_CHECK (p_uthread_join (thr1) == 0);
	BOOST_CHECK (p_uthread_join (thr2) == 0);

	BOOST_REQUIRE (semaphore_test_val == 10);

	BOOST_REQUIRE (p_semaphore_acquire (sem, NULL) == FALSE);
	BOOST_REQUIRE (p_semaphore_release (sem, NULL) == FALSE);
	p_semaphore_free (sem);
	p_semaphore_take_ownership (sem);

	sem = p_semaphore_new ("p_semaphore_test_object", 1, P_SEM_ACCESS_OPEN, NULL);
	BOOST_REQUIRE (sem != NULL);
	p_semaphore_take_ownership (sem);
	p_semaphore_free (sem);

	p_uthread_unref (thr1);
	p_uthread_unref (thr2);

	p_libsys_shutdown ();
#endif /* !P_OS_MSYS */
}

BOOST_AUTO_TEST_SUITE_END()
