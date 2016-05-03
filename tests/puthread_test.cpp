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

#ifndef PLIBSYS_TESTS_STATIC
#  define BOOST_TEST_DYN_LINK
#endif

#define BOOST_TEST_MODULE puthread_test

#include "plibsys.h"

#ifdef PLIBSYS_TESTS_STATIC
#  include <boost/test/included/unit_test.hpp>
#else
#  include <boost/test/unit_test.hpp>
#endif

static pint thread_wakes_1 = 0;
static pint thread_wakes_2 = 0;
static pint thread_to_wakes = 100;
static volatile pboolean is_threads_working = TRUE;

static P_HANDLE thread1_id = (P_HANDLE) NULL;
static P_HANDLE thread2_id = (P_HANDLE) NULL;

static PUThreadKey *tls_key = NULL;
static volatile pint free_counter = 0;

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

static void free_with_check (ppointer mem)
{
	p_free (mem);
	p_atomic_int_inc (&free_counter);
}

static void * test_thread_func (void *data)
{
	pint *counter =  static_cast < pint * > (data);

	if ((*counter) == 1)
		thread1_id = p_uthread_current_id ();
	else
		thread2_id = p_uthread_current_id ();

	p_uthread_set_local (tls_key, (ppointer) p_uthread_current_id ());

	*counter = 0;

	while (is_threads_working == TRUE) {
		p_uthread_sleep (10);
		++(*counter);
		p_uthread_yield ();

		if (p_uthread_get_local (tls_key) != (ppointer) p_uthread_current_id ())
			p_uthread_exit (-1);
	}

	p_uthread_exit (*counter);
}

static void * test_thread_nonjoinable_func (void *data)
{
	pint *counter =  static_cast < pint * > (data);

	is_threads_working = TRUE;

	for (int i = thread_to_wakes; i > 0; --i) {
		p_uthread_sleep (10);
		++(*counter);
		p_uthread_yield ();
	}

	is_threads_working = FALSE;

	p_uthread_exit (0);
}

static void * test_thread_tls_func (void *data)
{
	pint self_thread_free = *((pint *) data);

	pint *tls_value = (pint *) p_malloc0 (sizeof (pint));
	*tls_value = 0;
	p_uthread_set_local (tls_key, (ppointer) tls_value);

	pint prev_tls = 0;
	pint counter  = 0;

	while (is_threads_working == TRUE) {
		p_uthread_sleep (10);

		pint *last_tls = (pint *) p_uthread_get_local (tls_key);

		if ((*last_tls) != prev_tls)
			p_uthread_exit (-1);

		pint *tls_new_value = (pint *) p_malloc0 (sizeof (pint));

		*tls_new_value = (*last_tls) + 1;
		prev_tls       = (*last_tls) + 1;

		p_uthread_replace_local (tls_key, (ppointer) tls_new_value);

		if (self_thread_free)
			p_free (last_tls);

		++counter;

		p_uthread_yield ();
	}

	p_uthread_exit (counter);
}

BOOST_AUTO_TEST_SUITE (BOOST_TEST_MODULE)

BOOST_AUTO_TEST_CASE (puthread_nomem_test)
{
	p_libsys_init ();

	PUThreadKey *thread_key = p_uthread_local_new (p_free);
	BOOST_CHECK (thread_key != NULL);

	PMemVTable vtable;

	vtable.free	= pmem_free;
	vtable.malloc	= pmem_alloc;
	vtable.realloc	= pmem_realloc;

	BOOST_CHECK (p_mem_set_vtable (&vtable) == TRUE);

	BOOST_CHECK (p_uthread_create ((PUThreadFunc) test_thread_func,
				       (ppointer) &thread_wakes_1,
				       TRUE) == NULL);

	BOOST_CHECK (p_uthread_create_full ((PUThreadFunc) test_thread_func,
					    (ppointer) &thread_wakes_2,
					    TRUE,
					    P_UTHREAD_PRIORITY_NORMAL) == NULL);

	BOOST_CHECK (p_uthread_local_new (NULL) == NULL);

	p_uthread_set_local (thread_key, PINT_TO_POINTER (10));
	BOOST_CHECK (p_uthread_get_local (thread_key) == NULL);

	vtable.malloc	= (ppointer (*)(psize)) malloc;
	vtable.realloc	= (ppointer (*)(ppointer, psize)) realloc;
	vtable.free	= (void (*)(ppointer)) free;

	BOOST_CHECK (p_mem_set_vtable (&vtable) == TRUE);

	p_uthread_local_free (thread_key);

	p_libsys_shutdown ();
}

BOOST_AUTO_TEST_CASE (puthread_bad_input_test)
{
	p_libsys_init ();

	BOOST_CHECK (p_uthread_create (NULL, NULL, false) == NULL);
	BOOST_CHECK (p_uthread_create_full (NULL, NULL, false, P_UTHREAD_PRIORITY_NORMAL) == NULL);
	BOOST_CHECK (p_uthread_join (NULL) == -1);
	BOOST_CHECK (p_uthread_set_priority (NULL, P_UTHREAD_PRIORITY_NORMAL));
	BOOST_CHECK (p_uthread_get_local (NULL) == NULL);
	p_uthread_set_local (NULL, NULL);
	p_uthread_replace_local (NULL, NULL);
	p_uthread_free (NULL);
	p_uthread_local_free (NULL);

	p_libsys_shutdown ();
}

BOOST_AUTO_TEST_CASE (puthread_general_test)
{
	p_libsys_init ();

	thread_wakes_1 = 1;
	thread_wakes_2 = 2;

	tls_key = p_uthread_local_new (NULL);
	BOOST_CHECK (tls_key != NULL);

	PUThread *thr1 = p_uthread_create ((PUThreadFunc) test_thread_func,
					   (ppointer) &thread_wakes_1,
					   TRUE);

	PUThread *thr2 = p_uthread_create_full ((PUThreadFunc) test_thread_func,
						(ppointer) &thread_wakes_2,
						TRUE,
						P_UTHREAD_PRIORITY_NORMAL);

	BOOST_CHECK (p_uthread_set_priority (thr1, P_UTHREAD_PRIORITY_NORMAL) == 0);

	BOOST_REQUIRE (thr1 != NULL);
	BOOST_REQUIRE (thr2 != NULL);

	p_uthread_sleep (5000);

	is_threads_working = FALSE;

	BOOST_CHECK (p_uthread_join (thr1) == thread_wakes_1);
	BOOST_CHECK (p_uthread_join (thr2) == thread_wakes_2);

	BOOST_REQUIRE (thread1_id != thread2_id);
	BOOST_CHECK (thread1_id != p_uthread_current_id () &&
		     thread2_id != p_uthread_current_id ());

	p_uthread_local_free (tls_key);

	p_libsys_shutdown ();
}

BOOST_AUTO_TEST_CASE (puthread_nonjoinable_test)
{
	p_libsys_init ();

	thread_wakes_1 = 0;

	PUThread *thr1 = p_uthread_create ((PUThreadFunc) test_thread_nonjoinable_func,
					   (ppointer) &thread_wakes_1,
					   FALSE);

	BOOST_REQUIRE (thr1 != NULL);

	p_uthread_sleep (3000);

	BOOST_CHECK (p_uthread_join (thr1) == -1);

	while (is_threads_working == TRUE)
		p_uthread_sleep (10);

	BOOST_CHECK (thread_wakes_1 == thread_to_wakes);

	p_libsys_shutdown ();
}

BOOST_AUTO_TEST_CASE (puthread_tls_test)
{
	p_libsys_init ();

	/* With destroy notification */
	tls_key = p_uthread_local_new (free_with_check);

	is_threads_working = TRUE;

	pint self_thread_free = 0;

	PUThread *thr1 = p_uthread_create ((PUThreadFunc) test_thread_tls_func,
					   (ppointer) &self_thread_free,
					   TRUE);

	PUThread *thr2 = p_uthread_create ((PUThreadFunc) test_thread_tls_func,
					   (ppointer) &self_thread_free,
					   TRUE);

	BOOST_REQUIRE (thr1 != NULL);
	BOOST_REQUIRE (thr2 != NULL);

	p_uthread_sleep (5000);

	is_threads_working = FALSE;

	pint total_counter = 0;

	total_counter += p_uthread_join (thr1);
	total_counter += p_uthread_join (thr2);

	BOOST_CHECK (total_counter == free_counter);

	p_uthread_local_free (tls_key);

	/* Without destroy notification */
	tls_key = p_uthread_local_new (NULL);

	free_counter       = 0;
	is_threads_working = TRUE;
	self_thread_free   = 1;

	thr1 = p_uthread_create ((PUThreadFunc) test_thread_tls_func,
				 (ppointer) &self_thread_free,
				 TRUE);

	thr2 = p_uthread_create ((PUThreadFunc) test_thread_tls_func,
				 (ppointer) &self_thread_free,
				 TRUE);

	BOOST_REQUIRE (thr1 != NULL);
	BOOST_REQUIRE (thr2 != NULL);

	p_uthread_sleep (5000);

	is_threads_working = FALSE;

	total_counter = 0;

	total_counter += p_uthread_join (thr1);
	total_counter += p_uthread_join (thr2);

	BOOST_CHECK (total_counter > 0);
	BOOST_CHECK (free_counter == 0);

	p_uthread_local_free (tls_key);

	p_libsys_shutdown ();
}

BOOST_AUTO_TEST_SUITE_END()
