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

#define BOOST_TEST_MODULE pcondvariable_test

#include "plibsys.h"

#ifdef PLIBSYS_TESTS_STATIC
#  include <boost/test/included/unit_test.hpp>
#else
#  include <boost/test/unit_test.hpp>
#endif

#define PCONDTEST_MAX_QUEUE 10

static pint thread_wakeups = 0;
static pint thread_queue = 0;
static PCondVariable *queue_empty_cond = NULL;
static PCondVariable *queue_full_cond = NULL;
static PMutex *cond_mutex = NULL;
volatile static pboolean is_working = TRUE;

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

static void * producer_test_thread (void *)
{
	while (is_working == TRUE) {
		if (!p_mutex_lock (cond_mutex)) {
			is_working = FALSE;
			p_cond_variable_broadcast (queue_full_cond);
			p_uthread_exit (1);
		}

		while (thread_queue >= PCONDTEST_MAX_QUEUE && is_working == TRUE) {
			if (!p_cond_variable_wait (queue_empty_cond, cond_mutex)) {
				is_working = FALSE;
				p_cond_variable_broadcast (queue_full_cond);
				p_mutex_unlock (cond_mutex);
				p_uthread_exit (1);
			}
		}

		if (is_working) {
			++thread_queue;
			++thread_wakeups;
		}

		if (!p_cond_variable_broadcast (queue_full_cond)) {
			is_working = FALSE;
			p_mutex_unlock (cond_mutex);
			p_uthread_exit (1);
		}

		if (!p_mutex_unlock (cond_mutex)) {
			is_working = FALSE;
			p_cond_variable_broadcast (queue_full_cond);
			p_uthread_exit (1);
		}
	}

	p_cond_variable_broadcast (queue_full_cond);
	p_uthread_exit (0);

	return NULL;
}

static void * consumer_test_thread (void *)
{
	while (is_working == TRUE) {
		if (!p_mutex_lock (cond_mutex)) {
			is_working = FALSE;
			p_cond_variable_signal (queue_empty_cond);
			p_uthread_exit (1);
		}

		while (thread_queue <= 0 && is_working == TRUE) {
			if (!p_cond_variable_wait (queue_full_cond, cond_mutex)) {
				is_working = FALSE;
				p_cond_variable_signal (queue_empty_cond);
				p_mutex_unlock (cond_mutex);
				p_uthread_exit (1);
			}
		}

		if (is_working) {
			--thread_queue;
			++thread_wakeups;
		}

		if (!p_cond_variable_signal (queue_empty_cond)) {
			is_working = FALSE;
			p_mutex_unlock (cond_mutex);
			p_uthread_exit (1);
		}

		if (!p_mutex_unlock (cond_mutex)) {
			is_working = FALSE;
			p_cond_variable_signal (queue_empty_cond);
			p_uthread_exit (1);
		}
	}

	p_cond_variable_signal (queue_empty_cond);
	p_uthread_exit (0);

	return NULL;
}

BOOST_AUTO_TEST_SUITE (BOOST_TEST_MODULE)

BOOST_AUTO_TEST_CASE (pcondvariable_nomem_test)
{
	p_libsys_init ();

	PMemVTable vtable;

	vtable.free	= pmem_free;
	vtable.malloc	= pmem_alloc;
	vtable.realloc	= pmem_realloc;

	BOOST_CHECK (p_mem_set_vtable (&vtable) == TRUE);
	BOOST_CHECK (p_cond_variable_new () == NULL);

	p_mem_restore_vtable ();

	p_libsys_shutdown ();
}

BOOST_AUTO_TEST_CASE (pcondvariable_general_test)
{
	PUThread	*thr1, *thr2, *thr3;

	p_libsys_init ();

	BOOST_REQUIRE (p_cond_variable_broadcast (queue_empty_cond) == FALSE);
	BOOST_REQUIRE (p_cond_variable_signal (queue_empty_cond) == FALSE);
	BOOST_REQUIRE (p_cond_variable_wait (queue_empty_cond, cond_mutex) == FALSE);
	p_cond_variable_free (NULL);

	queue_empty_cond = p_cond_variable_new ();
	BOOST_REQUIRE (queue_empty_cond != NULL);
	queue_full_cond = p_cond_variable_new ();
	BOOST_REQUIRE (queue_full_cond != NULL);
	cond_mutex = p_mutex_new ();
	BOOST_REQUIRE (cond_mutex != NULL);

	thr1 = p_uthread_create ((PUThreadFunc) producer_test_thread, NULL, true);
	BOOST_REQUIRE (thr1 != NULL);

	thr2 = p_uthread_create ((PUThreadFunc) consumer_test_thread, NULL, true);
	BOOST_REQUIRE (thr2 != NULL);

	thr3 = p_uthread_create ((PUThreadFunc) consumer_test_thread, NULL, true);
	BOOST_REQUIRE (thr3 != NULL);

	BOOST_REQUIRE (p_cond_variable_broadcast (queue_empty_cond) == TRUE);
	BOOST_REQUIRE (p_cond_variable_broadcast (queue_full_cond) == TRUE);

	p_uthread_sleep (4000);

	is_working = FALSE;

	BOOST_CHECK (p_uthread_join (thr1) == 0);
	BOOST_CHECK (p_uthread_join (thr2) == 0);
	BOOST_CHECK (p_uthread_join (thr3) == 0);

	BOOST_REQUIRE (thread_wakeups > 0 && thread_queue >= 0 && thread_queue <= 10);

	p_uthread_unref (thr1);
	p_uthread_unref (thr2);
	p_uthread_unref (thr3);
	p_cond_variable_free (queue_empty_cond);
	p_cond_variable_free (queue_full_cond);
	p_mutex_free (cond_mutex);

	p_libsys_shutdown ();
}

BOOST_AUTO_TEST_SUITE_END()
