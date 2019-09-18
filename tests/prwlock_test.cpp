/*
 * The MIT License
 *
 * Copyright (C) 2016-2019 Alexander Saprykin <saprykin.spb@gmail.com>
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

#include "plibsys.h"
#include "ptestmacros.h"

#include <string.h>

P_TEST_MODULE_INIT ();

#define PRWLOCK_TEST_STRING_1 "This is a test string."
#define PRWLOCK_TEST_STRING_2 "Ouh, yet another string to check!"

static PRWLock *         test_rwlock        = NULL;
static volatile pboolean is_threads_working = FALSE;
static volatile pint     writers_counter    = 0;
static pchar             string_buf[50];

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

static void * reader_thread_func (void *data)
{
	P_UNUSED (data);

	pint counter = 0;

	while (p_atomic_int_get (&writers_counter) == 0)
		p_uthread_sleep (10);

	while (is_threads_working == TRUE) {
		p_uthread_sleep (10);

		if (p_rwlock_reader_trylock (test_rwlock) == FALSE) {
			if (p_rwlock_reader_lock (test_rwlock) == FALSE)
				p_uthread_exit (-1);
		}

		if (strcmp (string_buf, PRWLOCK_TEST_STRING_1) != 0 &&
		    strcmp (string_buf, PRWLOCK_TEST_STRING_2) != 0) {
			p_rwlock_reader_unlock (test_rwlock);
			p_uthread_exit (-1);
		}

		if (p_rwlock_reader_unlock (test_rwlock) == FALSE)
			p_uthread_exit (-1);

		++counter;
	}

	p_uthread_exit (counter);

	return NULL;
}

static void * writer_thread_func (void *data)
{
	pint string_num = PPOINTER_TO_INT (data);
	pint counter    = 0;

	while (is_threads_working == TRUE) {
		p_uthread_sleep (10);

		if (p_rwlock_writer_trylock (test_rwlock) == FALSE) {
			if (p_rwlock_writer_lock (test_rwlock) == FALSE)
				p_uthread_exit (-1);
		}

		memset (string_buf, 0, sizeof (string_buf));

		if (string_num == 1)
			strcpy (string_buf, PRWLOCK_TEST_STRING_1);
		else
			strcpy (string_buf, PRWLOCK_TEST_STRING_1);

		if (p_rwlock_writer_unlock (test_rwlock) == FALSE)
			p_uthread_exit (-1);

		++counter;

		p_atomic_int_inc ((&writers_counter));
	}

	p_uthread_exit (counter);

	return NULL;
}

P_TEST_CASE_BEGIN (prwlock_nomem_test)
{
	p_libsys_init ();

	PMemVTable vtable;

	vtable.free    = pmem_free;
	vtable.malloc  = pmem_alloc;
	vtable.realloc = pmem_realloc;

	P_TEST_CHECK (p_mem_set_vtable (&vtable) == TRUE);

	P_TEST_CHECK (p_rwlock_new () == NULL);

	p_mem_restore_vtable ();

	p_libsys_shutdown ();
}
P_TEST_CASE_END ()

P_TEST_CASE_BEGIN (prwlock_bad_input_test)
{
	p_libsys_init ();

	P_TEST_CHECK (p_rwlock_reader_lock (NULL) == FALSE);
	P_TEST_CHECK (p_rwlock_reader_trylock (NULL) == FALSE);
	P_TEST_CHECK (p_rwlock_reader_unlock (NULL) == FALSE);
	P_TEST_CHECK (p_rwlock_writer_lock (NULL) == FALSE);
	P_TEST_CHECK (p_rwlock_writer_trylock (NULL) == FALSE);
	P_TEST_CHECK (p_rwlock_writer_unlock (NULL) == FALSE);
	p_rwlock_free (NULL);

	p_libsys_shutdown ();
}
P_TEST_CASE_END ()

P_TEST_CASE_BEGIN (prwlock_general_test)
{
	p_libsys_init ();

	test_rwlock = p_rwlock_new ();

	P_TEST_REQUIRE (test_rwlock != NULL);

	is_threads_working = TRUE;
	writers_counter    = 0;

	PUThread *reader_thr1 = p_uthread_create ((PUThreadFunc) reader_thread_func,
						  NULL,
						  TRUE,
						  NULL);

	PUThread *reader_thr2 = p_uthread_create ((PUThreadFunc) reader_thread_func,
						  NULL,
						  TRUE,
						  NULL);

	PUThread *writer_thr1 = p_uthread_create ((PUThreadFunc) writer_thread_func,
						  NULL,
						  TRUE,
						  NULL);

	PUThread *writer_thr2 = p_uthread_create ((PUThreadFunc) writer_thread_func,
						  NULL,
						  TRUE,
						  NULL);

	P_TEST_REQUIRE (reader_thr1 != NULL);
	P_TEST_REQUIRE (reader_thr2 != NULL);
	P_TEST_REQUIRE (writer_thr1 != NULL);
	P_TEST_REQUIRE (writer_thr2 != NULL);

	p_uthread_sleep (10000);

	is_threads_working = FALSE;

	P_TEST_CHECK (p_uthread_join (reader_thr1) > 0);
	P_TEST_CHECK (p_uthread_join (reader_thr2) > 0);
	P_TEST_CHECK (p_uthread_join (writer_thr1) > 0);
	P_TEST_CHECK (p_uthread_join (writer_thr2) > 0);

	p_uthread_unref (reader_thr1);
	p_uthread_unref (reader_thr2);
	p_uthread_unref (writer_thr1);
	p_uthread_unref (writer_thr2);

	p_rwlock_free (test_rwlock);

	p_libsys_shutdown ();
}
P_TEST_CASE_END ()

P_TEST_SUITE_BEGIN()
{
	P_TEST_SUITE_RUN_CASE (prwlock_nomem_test);
	P_TEST_SUITE_RUN_CASE (prwlock_bad_input_test);
	P_TEST_SUITE_RUN_CASE (prwlock_general_test);
}
P_TEST_SUITE_END()
