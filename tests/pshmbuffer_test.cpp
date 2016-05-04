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

#define BOOST_TEST_MODULE pshmbuffer_test

#include "plibsys.h"

#include <string.h>

#ifdef PLIBSYS_TESTS_STATIC
#  include <boost/test/included/unit_test.hpp>
#else
#  include <boost/test/unit_test.hpp>
#endif

#ifndef P_OS_MSYS
static pchar test_str[]    = "This is a test string!";
static pint is_thread_exit = 0;
static pint read_count     = 0;
static pint write_count    = 0;

#  ifndef P_OS_HPUX
volatile static pboolean is_working = TRUE;

static void * shm_buffer_test_write_thread (void *)
{
	PShmBuffer *buffer = p_shm_buffer_new ("pshm_test_buffer", 1024, NULL);

	if (buffer == NULL)
		p_uthread_exit (1);

	while (is_working == TRUE) {
		p_uthread_sleep (3);

		pssize op_result = p_shm_buffer_get_free_space (buffer, NULL);

		if (op_result < 0) {
			if (is_thread_exit > 0)
				break;
			else {
				++is_thread_exit;
				p_shm_buffer_free (buffer);
				p_uthread_exit (1);
			}
		}

		if (op_result < sizeof (test_str))
			continue;

		op_result = p_shm_buffer_write (buffer, (ppointer) test_str, sizeof (test_str), NULL);

		if (op_result < 0) {
			if (is_thread_exit > 0)
				break;
			else {
				++is_thread_exit;
				p_shm_buffer_free (buffer);
				p_uthread_exit (1);
			}
		}

		if (op_result != sizeof (test_str)) {
			++is_thread_exit;
			p_shm_buffer_free (buffer);
			p_uthread_exit (1);
		}

		++read_count;
	}

	++is_thread_exit;

	p_shm_buffer_free (buffer);
	p_uthread_exit (0);
}

static void * shm_buffer_test_read_thread (void *)
{
	PShmBuffer	*buffer = p_shm_buffer_new ("pshm_test_buffer", 1024, NULL);
	pchar		test_buf[sizeof (test_str)];

	if (buffer == NULL)
		p_uthread_exit (1);

	while (is_working == TRUE) {
		p_uthread_sleep (3);

		pssize op_result = p_shm_buffer_get_used_space (buffer, NULL);

		if (op_result < 0) {
			if (is_thread_exit > 0)
				break;
			else {
				++is_thread_exit;
				p_shm_buffer_free (buffer);
				p_uthread_exit (1);
			}
		}

		if (op_result < sizeof (test_str))
			continue;

		op_result = p_shm_buffer_read (buffer, (ppointer) test_buf, sizeof (test_buf), NULL);

		if (op_result < 0) {
			if (is_thread_exit > 0)
				break;
			else {
				++is_thread_exit;
				p_shm_buffer_free (buffer);
				p_uthread_exit (1);
			}
		}

		if (op_result != sizeof (test_buf)) {
			++is_thread_exit;
			p_shm_buffer_free (buffer);
			p_uthread_exit (1);
		}

		if (strncmp (test_buf, test_str, sizeof (test_buf)) != 0) {
			++is_thread_exit;
			p_shm_buffer_free (buffer);
			p_uthread_exit (1);
		}

		++write_count;
	}

	++is_thread_exit;

	p_shm_buffer_free (buffer);
	p_uthread_exit (0);
}
#  endif /* !P_OS_HPUX */
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

BOOST_AUTO_TEST_CASE (pshmbuffer_nomem_test)
{
	p_libsys_init ();

	PMemVTable vtable;

	vtable.free	= pmem_free;
	vtable.malloc	= pmem_alloc;
	vtable.realloc	= pmem_realloc;

	BOOST_CHECK (p_mem_set_vtable (&vtable) == TRUE);

	BOOST_CHECK (p_shm_buffer_new ("pshm_test_buffer", 1024, NULL) == NULL);

	vtable.malloc	= (ppointer (*)(psize)) malloc;
	vtable.realloc	= (ppointer (*)(ppointer, psize)) realloc;
	vtable.free	= (void (*)(ppointer)) free;

	BOOST_CHECK (p_mem_set_vtable (&vtable) == TRUE);

	p_libsys_shutdown ();
}

BOOST_AUTO_TEST_CASE (pshmbuffer_bad_input_test)
{
#ifndef P_OS_MSYS
	p_libsys_init ();

	BOOST_CHECK (p_shm_buffer_new (NULL, 0, NULL) == NULL);
	BOOST_CHECK (p_shm_buffer_read (NULL, NULL, 0, NULL) == -1);
	BOOST_CHECK (p_shm_buffer_write (NULL, NULL, 0, NULL) == -1);
	BOOST_CHECK (p_shm_buffer_get_free_space (NULL, NULL) == -1);
	BOOST_CHECK (p_shm_buffer_get_used_space (NULL, NULL) == -1);

	PShmBuffer *buf = p_shm_buffer_new ("pshm_invalid_buffer", 0, NULL);
	p_shm_buffer_take_ownership (buf);
	p_shm_buffer_free (buf);

	p_shm_buffer_clear (NULL);
	p_shm_buffer_free (NULL);

	p_libsys_shutdown ();
#endif /* !P_OS_MSYS */
}

BOOST_AUTO_TEST_CASE (pshmbuffer_general_test)
{
#ifndef P_OS_MSYS
	p_libsys_init ();

	pchar		test_buf[sizeof (test_str)];
	pchar		*large_buf;
	PShmBuffer	*buffer = NULL;

	/* Buffer may be from the previous test on UNIX systems */
	buffer = p_shm_buffer_new ("pshm_test_buffer", 1024, NULL);
	BOOST_REQUIRE (buffer != NULL);
	p_shm_buffer_take_ownership (buffer);
	p_shm_buffer_free (buffer);
	buffer = p_shm_buffer_new ("pshm_test_buffer", 1024, NULL);
	BOOST_REQUIRE (buffer != NULL);

	BOOST_CHECK (p_shm_buffer_get_free_space (buffer, NULL) == 1024);
	BOOST_CHECK (p_shm_buffer_get_used_space (buffer, NULL) == 0);
	p_shm_buffer_clear (buffer);
	BOOST_CHECK (p_shm_buffer_get_free_space (buffer, NULL) == 1024);
	BOOST_CHECK (p_shm_buffer_get_used_space (buffer, NULL) == 0);

	memset (test_buf, 0, sizeof (test_buf));

	BOOST_CHECK (p_shm_buffer_write (buffer, (ppointer) test_str, sizeof (test_str), NULL) == sizeof (test_str));
	BOOST_CHECK (p_shm_buffer_get_free_space (buffer, NULL) == (1024 - sizeof (test_str)));
	BOOST_CHECK (p_shm_buffer_get_used_space (buffer, NULL) == sizeof (test_str));
	BOOST_CHECK (p_shm_buffer_read (buffer, (ppointer) test_buf, sizeof (test_buf), NULL) == sizeof (test_str));
	BOOST_CHECK (p_shm_buffer_read (buffer, (ppointer) test_buf, sizeof (test_buf), NULL) == 0);

	BOOST_CHECK (strncmp (test_buf, test_str, sizeof (test_str)) == 0);
	BOOST_CHECK (p_shm_buffer_get_free_space (buffer, NULL) == 1024);
	BOOST_CHECK (p_shm_buffer_get_used_space (buffer, NULL) == 0);

	p_shm_buffer_clear (buffer);

	large_buf = (pchar *) p_malloc0 (2048);
	BOOST_REQUIRE (large_buf != NULL);
	BOOST_CHECK (p_shm_buffer_write (buffer, (ppointer) large_buf, 2048, NULL) == 0);

	p_free (large_buf);
	p_shm_buffer_free (buffer);

	p_libsys_shutdown ();
#endif
}

#if !defined (P_OS_HPUX) && !defined (P_OS_MSYS)
BOOST_AUTO_TEST_CASE (pshmbuffer_thread_test)
{
	p_libsys_init ();

	PShmBuffer *	buffer = NULL;
	PUThread	*thr1, *thr2;

	/* Buffer may be from the previous test on UNIX systems */
	buffer = p_shm_buffer_new ("pshm_test_buffer", 1024, NULL);
	BOOST_REQUIRE (buffer != NULL);
	p_shm_buffer_take_ownership (buffer);
	p_shm_buffer_free (buffer);

	buffer = p_shm_buffer_new ("pshm_test_buffer", 1024, NULL);
	BOOST_REQUIRE (buffer != NULL);

	thr1 = p_uthread_create ((PUThreadFunc) shm_buffer_test_write_thread, NULL, TRUE);
	BOOST_REQUIRE (thr1 != NULL);

	thr2 = p_uthread_create ((PUThreadFunc) shm_buffer_test_read_thread, NULL, TRUE);
	BOOST_REQUIRE (thr1 != NULL);

	p_uthread_sleep (5000);

	is_working = FALSE;

	BOOST_CHECK (p_uthread_join (thr1) == 0);
	BOOST_CHECK (p_uthread_join (thr2) == 0);

	BOOST_CHECK (read_count > 0);
	BOOST_CHECK (write_count > 0);

	p_shm_buffer_free (buffer);
	p_uthread_free (thr1);
	p_uthread_free (thr2);

	p_libsys_shutdown ();
}
#endif /* !P_OS_HPUX && !P_OS_MSYS */

BOOST_AUTO_TEST_SUITE_END()
