/*
 * Copyright (C) 2013 Alexander Saprykin <xelfium@gmail.com>
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
#define BOOST_TEST_MODULE puthread_test

#include "plib.h"

#include <boost/test/unit_test.hpp>

static pint thread_wakes_1 = 0;
static pint thread_wakes_2 = 0;
static pint thread_to_wakes = 100;
static volatile pboolean is_threads_working = TRUE;

static void * test_thread_func (void *data)
{
	pint *counter =  static_cast < pint * > (data);

	while (is_threads_working == TRUE) {
		p_uthread_sleep (10);
		++(*counter);
		p_uthread_yield ();
	}

	p_uthread_exit (*counter);
}

static void * test_thread_nonjoinable_func (void *data)
{
	pint *counter =  static_cast < pint * > (data);

	for (int i = thread_to_wakes; i > 0; --i) {
		p_uthread_sleep (10);
		++(*counter);
		p_uthread_yield ();
	}

	p_uthread_exit (0);
}

BOOST_AUTO_TEST_SUITE (BOOST_TEST_MODULE)

BOOST_AUTO_TEST_CASE (puthread_bad_input_test)
{
	p_lib_init ();

	BOOST_CHECK (p_uthread_create (NULL, NULL, false) == NULL);
	BOOST_CHECK (p_uthread_create_full(NULL, NULL, false, P_UTHREAD_PRIORITY_NORMAL) == NULL);
	BOOST_CHECK (p_uthread_join (NULL) == -1);
	BOOST_CHECK (p_uthread_set_priority (NULL, P_UTHREAD_PRIORITY_NORMAL));
	p_uthread_free (NULL);

	p_lib_shutdown ();
}

BOOST_AUTO_TEST_CASE (puthread_general_test)
{
	p_lib_init ();

	thread_wakes_1 = 0;
	thread_wakes_2 = 0;

	PUThread *thr1 = p_uthread_create ((PUThreadFunc) test_thread_func, (ppointer) &thread_wakes_1, TRUE);
	PUThread *thr2 = p_uthread_create_full ((PUThreadFunc) test_thread_func, (ppointer) &thread_wakes_2, TRUE, P_UTHREAD_PRIORITY_NORMAL);

	BOOST_REQUIRE (thr1 != NULL);
	BOOST_REQUIRE (thr2 != NULL);

	p_uthread_sleep (5000);

	is_threads_working = FALSE;

	BOOST_CHECK (p_uthread_join (thr1) == thread_wakes_1);
	BOOST_CHECK (p_uthread_join (thr2) == thread_wakes_2);

	p_lib_shutdown ();
}

BOOST_AUTO_TEST_CASE (puthread_nonjoinable_test)
{
	p_lib_init ();

	thread_wakes_1 = 0;

	PUThread *thr1 = p_uthread_create ((PUThreadFunc) test_thread_nonjoinable_func, (ppointer) &thread_wakes_1, FALSE);

	BOOST_REQUIRE (thr1 != NULL);

	p_uthread_sleep (3000);

	BOOST_CHECK (p_uthread_join (thr1) == -1);
	BOOST_CHECK (thread_wakes_1 == thread_to_wakes);

	p_lib_shutdown ();
}

BOOST_AUTO_TEST_SUITE_END()
