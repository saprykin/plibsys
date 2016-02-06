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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA.
 */

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE psemaphore_test

#include "plib.h"

#ifdef PLIB_TESTS_STATIC
#  include <boost/test/included/unit_test.hpp>
#else
#  include <boost/test/unit_test.hpp>
#endif

static pint semaphore_test_val = 10;

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
		if (!p_semaphore_acquire (sem, NULL))
			p_uthread_exit (1);

		if (semaphore_test_val == 10)
			--semaphore_test_val;
		else {
			p_uthread_sleep (1);
			++semaphore_test_val;
		}

		if (!p_semaphore_release (sem, NULL))
			p_uthread_exit (1);
	}

	p_semaphore_free (sem);
	p_uthread_exit (0);
}

BOOST_AUTO_TEST_SUITE (BOOST_TEST_MODULE)

BOOST_AUTO_TEST_CASE (psemaphore_general_test)
{
	PSemaphore	*sem = NULL;
	PError		*error = NULL;
	pint		i;

	p_lib_init ();

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

	p_lib_shutdown ();
}

BOOST_AUTO_TEST_CASE (psemaphore_thread_test)
{
	PUThread	*thr1, *thr2;
	PSemaphore	*sem = NULL;

	p_lib_init ();

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

	p_uthread_free (thr1);
	p_uthread_free (thr2);

	p_lib_shutdown ();
}

BOOST_AUTO_TEST_SUITE_END()
