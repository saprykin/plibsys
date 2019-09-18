/*
 * The MIT License
 *
 * Copyright (C) 2013-2019 Alexander Saprykin <saprykin.spb@gmail.com>
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

P_TEST_MODULE_INIT ();

#define PCONDTEST_MAX_QUEUE 10

static pint              thread_wakeups   = 0;
static pint              thread_queue     = 0;
static PCondVariable *   queue_empty_cond = NULL;
static PCondVariable *   queue_full_cond  = NULL;
static PMutex *          cond_mutex       = NULL;
volatile static pboolean is_working       = TRUE;

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

P_TEST_CASE_BEGIN (pcondvariable_nomem_test)
{
	p_libsys_init ();

	PMemVTable vtable;

	vtable.free    = pmem_free;
	vtable.malloc  = pmem_alloc;
	vtable.realloc = pmem_realloc;

	P_TEST_CHECK (p_mem_set_vtable (&vtable) == TRUE);
	P_TEST_CHECK (p_cond_variable_new () == NULL);

	p_mem_restore_vtable ();

	p_libsys_shutdown ();
}
P_TEST_CASE_END ()

P_TEST_CASE_BEGIN (pcondvariable_bad_input_test)
{
	p_libsys_init ();

	P_TEST_REQUIRE (p_cond_variable_broadcast (NULL) == FALSE);
	P_TEST_REQUIRE (p_cond_variable_signal (NULL) == FALSE);
	P_TEST_REQUIRE (p_cond_variable_wait (NULL, NULL) == FALSE);
	p_cond_variable_free (NULL);

	p_libsys_shutdown ();
}
P_TEST_CASE_END ()

P_TEST_CASE_BEGIN (pcondvariable_general_test)
{
	PUThread *thr1, *thr2, *thr3;

	p_libsys_init ();

	queue_empty_cond = p_cond_variable_new ();
	P_TEST_REQUIRE (queue_empty_cond != NULL);
	queue_full_cond = p_cond_variable_new ();
	P_TEST_REQUIRE (queue_full_cond != NULL);
	cond_mutex = p_mutex_new ();
	P_TEST_REQUIRE (cond_mutex != NULL);

	is_working     = TRUE;
	thread_wakeups = 0;
	thread_queue   = 0;

	thr1 = p_uthread_create ((PUThreadFunc) producer_test_thread, NULL, TRUE, NULL);
	P_TEST_REQUIRE (thr1 != NULL);

	thr2 = p_uthread_create ((PUThreadFunc) consumer_test_thread, NULL, TRUE, NULL);
	P_TEST_REQUIRE (thr2 != NULL);

	thr3 = p_uthread_create ((PUThreadFunc) consumer_test_thread, NULL, TRUE, NULL);
	P_TEST_REQUIRE (thr3 != NULL);

	P_TEST_REQUIRE (p_cond_variable_broadcast (queue_empty_cond) == TRUE);
	P_TEST_REQUIRE (p_cond_variable_broadcast (queue_full_cond) == TRUE);

	p_uthread_sleep (4000);

	is_working = FALSE;

	P_TEST_CHECK (p_uthread_join (thr1) == 0);
	P_TEST_CHECK (p_uthread_join (thr2) == 0);
	P_TEST_CHECK (p_uthread_join (thr3) == 0);

	P_TEST_REQUIRE (thread_wakeups > 0 && thread_queue >= 0 && thread_queue <= 10);

	p_uthread_unref (thr1);
	p_uthread_unref (thr2);
	p_uthread_unref (thr3);
	p_cond_variable_free (queue_empty_cond);
	p_cond_variable_free (queue_full_cond);
	p_mutex_free (cond_mutex);

	p_libsys_shutdown ();
}
P_TEST_CASE_END ()

P_TEST_SUITE_BEGIN()
{
	P_TEST_SUITE_RUN_CASE (pcondvariable_nomem_test);
	P_TEST_SUITE_RUN_CASE (pcondvariable_bad_input_test);
	P_TEST_SUITE_RUN_CASE (pcondvariable_general_test);
}
P_TEST_SUITE_END()
