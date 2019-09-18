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

static pint              thread_wakes_1     = 0;
static pint              thread_wakes_2     = 0;
static pint              thread_to_wakes    = 0;
static volatile pboolean is_threads_working = FALSE;

static P_HANDLE   thread1_id  = (P_HANDLE) NULL;
static P_HANDLE   thread2_id  = (P_HANDLE) NULL;
static PUThread * thread1_obj = NULL;
static PUThread * thread2_obj = NULL;

static PUThreadKey * tls_key      = NULL;
static PUThreadKey * tls_key_2    = NULL;
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

extern "C" void free_with_check (ppointer mem)
{
	p_free (mem);
	p_atomic_int_inc (&free_counter);
}

static void * test_thread_func (void *data)
{
	pint *counter = static_cast < pint * > (data);

	if ((*counter) == 1) {
		thread1_id  = p_uthread_current_id ();
		thread1_obj = p_uthread_current ();
	} else {
		thread2_id  = p_uthread_current_id ();
		thread2_obj = p_uthread_current ();
	}

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

	return NULL;
}

static void * test_thread_nonjoinable_func (void *data)
{
	pint *counter = static_cast < pint * > (data);

	is_threads_working = TRUE;

	for (int i = thread_to_wakes; i > 0; --i) {
		p_uthread_sleep (10);
		++(*counter);
		p_uthread_yield ();
	}

	is_threads_working = FALSE;

	p_uthread_exit (0);

	return NULL;
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

	if (self_thread_free) {
		pint *last_tls = (pint *) p_uthread_get_local (tls_key);

		if ((*last_tls) != prev_tls)
			p_uthread_exit (-1);

		p_free (last_tls);

		p_uthread_replace_local (tls_key, (ppointer) NULL);
	}

	p_uthread_exit (counter);

	return NULL;
}

static void * test_thread_tls_create_func (void *data)
{
	P_UNUSED (data);

	pint *tls_value = (pint *) p_malloc0 (sizeof (pint));
	*tls_value = 0;
	p_uthread_set_local (tls_key, (ppointer) tls_value);

	pint *tls_value_2 = (pint *) p_malloc0 (sizeof (pint));
	*tls_value_2 = 0;
	p_uthread_set_local (tls_key_2, (ppointer) tls_value_2);

	return NULL;
}

P_TEST_CASE_BEGIN (puthread_nomem_test)
{
	p_libsys_init ();

	PUThreadKey *thread_key = p_uthread_local_new (p_free);
	P_TEST_CHECK (thread_key != NULL);

	PMemVTable vtable;

	vtable.free    = pmem_free;
	vtable.malloc  = pmem_alloc;
	vtable.realloc = pmem_realloc;

	P_TEST_CHECK (p_mem_set_vtable (&vtable) == TRUE);

	thread_wakes_1 = 0;
	thread_wakes_2 = 0;

	P_TEST_CHECK (p_uthread_create ((PUThreadFunc) test_thread_func,
					(ppointer) &thread_wakes_1,
					TRUE,
					NULL) == NULL);

	P_TEST_CHECK (p_uthread_create_full ((PUThreadFunc) test_thread_func,
					     (ppointer) &thread_wakes_2,
					     TRUE,
					     P_UTHREAD_PRIORITY_NORMAL,
					     0,
					     NULL) == NULL);

	P_TEST_CHECK (p_uthread_current () == NULL);
	P_TEST_CHECK (p_uthread_local_new (NULL) == NULL);

	p_uthread_exit (0);

	p_uthread_set_local (thread_key, PINT_TO_POINTER (10));

	ppointer tls_value = p_uthread_get_local (thread_key);

	if (tls_value != NULL) {
		P_TEST_CHECK (tls_value == PINT_TO_POINTER (10));
		p_uthread_set_local (thread_key, NULL);
	}

	p_uthread_replace_local (thread_key, PINT_TO_POINTER (12));

	tls_value = p_uthread_get_local (thread_key);

	if (tls_value != NULL) {
		P_TEST_CHECK (tls_value == PINT_TO_POINTER (12));
		p_uthread_set_local (thread_key, NULL);
	}

	p_mem_restore_vtable ();

	p_uthread_local_free (thread_key);

	p_libsys_shutdown ();
}
P_TEST_CASE_END ()

P_TEST_CASE_BEGIN (puthread_bad_input_test)
{
	p_libsys_init ();

	P_TEST_CHECK (p_uthread_create (NULL, NULL, FALSE, NULL) == NULL);
	P_TEST_CHECK (p_uthread_create_full (NULL, NULL, FALSE, P_UTHREAD_PRIORITY_NORMAL, 0, NULL) == NULL);
	P_TEST_CHECK (p_uthread_join (NULL) == -1);
	P_TEST_CHECK (p_uthread_set_priority (NULL, P_UTHREAD_PRIORITY_NORMAL) == FALSE);
	P_TEST_CHECK (p_uthread_get_local (NULL) == NULL);
	p_uthread_set_local (NULL, NULL);
	p_uthread_replace_local (NULL, NULL);
	p_uthread_ref (NULL);
	p_uthread_unref (NULL);
	p_uthread_local_free (NULL);
	p_uthread_exit (0);

	p_libsys_shutdown ();
}
P_TEST_CASE_END ()

P_TEST_CASE_BEGIN (puthread_general_test)
{
	p_libsys_init ();

	thread_wakes_1 = 1;
	thread_wakes_2 = 2;
	thread1_id     = (P_HANDLE) NULL;
	thread2_id     = (P_HANDLE) NULL;
	thread1_obj    = NULL;
	thread2_obj    = NULL;

	tls_key = p_uthread_local_new (NULL);
	P_TEST_CHECK (tls_key != NULL);

	/* Threre is no guarantee that we wouldn't get one of the IDs
	 * of the finished test threads */

	P_HANDLE main_id = p_uthread_current_id ();

	is_threads_working = TRUE;

	PUThread *thr1 = p_uthread_create_full ((PUThreadFunc) test_thread_func,
						(ppointer) &thread_wakes_1,
						TRUE,
						P_UTHREAD_PRIORITY_NORMAL,
						64 * 1024,
						"thread_name");

	PUThread *thr2 = p_uthread_create_full ((PUThreadFunc) test_thread_func,
						(ppointer) &thread_wakes_2,
						TRUE,
						P_UTHREAD_PRIORITY_NORMAL,
						64 * 1024,
						"very_long_name_for_thread_testing");

	p_uthread_ref (thr1);

	p_uthread_set_priority (thr1, P_UTHREAD_PRIORITY_NORMAL);

	P_TEST_REQUIRE (thr1 != NULL);
	P_TEST_REQUIRE (thr2 != NULL);

	p_uthread_sleep (5000);

	is_threads_working = FALSE;

	P_TEST_CHECK (p_uthread_join (thr1) == thread_wakes_1);
	P_TEST_CHECK (p_uthread_join (thr2) == thread_wakes_2);

	P_TEST_REQUIRE (thread1_id != thread2_id);
	P_TEST_CHECK (thread1_id != main_id && thread2_id != main_id);

	P_TEST_CHECK (thread1_obj == thr1);
	P_TEST_CHECK (thread2_obj == thr2);

	p_uthread_local_free (tls_key);
	p_uthread_unref (thr1);
	p_uthread_unref (thr2);

	p_uthread_unref (thr1);

	PUThread *cur_thr = p_uthread_current ();
	P_TEST_CHECK (cur_thr != NULL);

	P_TEST_CHECK (p_uthread_ideal_count () > 0);

	p_libsys_shutdown ();
}
P_TEST_CASE_END ()

P_TEST_CASE_BEGIN (puthread_nonjoinable_test)
{
	p_libsys_init ();

	thread_wakes_1     = 0;
	thread_to_wakes    = 100;
	is_threads_working = TRUE;

	PUThread *thr1 = p_uthread_create ((PUThreadFunc) test_thread_nonjoinable_func,
					   (ppointer) &thread_wakes_1,
					   FALSE,
					   NULL);

	P_TEST_REQUIRE (thr1 != NULL);

	p_uthread_sleep (3000);

	P_TEST_CHECK (p_uthread_join (thr1) == -1);

	while (is_threads_working == TRUE)
		p_uthread_sleep (10);

	P_TEST_CHECK (thread_wakes_1 == thread_to_wakes);

	p_uthread_unref (thr1);

	p_libsys_shutdown ();
}
P_TEST_CASE_END ()

P_TEST_CASE_BEGIN (puthread_tls_test)
{
	p_libsys_init ();

	/* With destroy notification */
	tls_key = p_uthread_local_new (free_with_check);

	is_threads_working = TRUE;
	free_counter       = 0;

	pint self_thread_free = 0;

	PUThread *thr1 = p_uthread_create ((PUThreadFunc) test_thread_tls_func,
					   (ppointer) &self_thread_free,
					   TRUE,
					   NULL);

	PUThread *thr2 = p_uthread_create ((PUThreadFunc) test_thread_tls_func,
					   (ppointer) &self_thread_free,
					   TRUE,
					   NULL);

	P_TEST_REQUIRE (thr1 != NULL);
	P_TEST_REQUIRE (thr2 != NULL);

	p_uthread_sleep (5000);

	is_threads_working = FALSE;

	pint total_counter = 0;

	total_counter += (p_uthread_join (thr1) + 1);
	total_counter += (p_uthread_join (thr2) + 1);

	P_TEST_CHECK (total_counter == free_counter);

	p_uthread_local_free (tls_key);
	p_uthread_unref (thr1);
	p_uthread_unref (thr2);

	/* Without destroy notification */
	tls_key = p_uthread_local_new (NULL);

	free_counter       = 0;
	is_threads_working = TRUE;
	self_thread_free   = 1;

	thr1 = p_uthread_create ((PUThreadFunc) test_thread_tls_func,
				 (ppointer) &self_thread_free,
				 TRUE,
				 NULL);

	thr2 = p_uthread_create ((PUThreadFunc) test_thread_tls_func,
				 (ppointer) &self_thread_free,
				 TRUE,
				 NULL);

	P_TEST_REQUIRE (thr1 != NULL);
	P_TEST_REQUIRE (thr2 != NULL);

	p_uthread_sleep (5000);

	is_threads_working = FALSE;

	total_counter = 0;

	total_counter += (p_uthread_join (thr1) + 1);
	total_counter += (p_uthread_join (thr2) + 1);

	P_TEST_CHECK (total_counter > 0);
	P_TEST_CHECK (free_counter == 0);

	p_uthread_local_free (tls_key);
	p_uthread_unref (thr1);
	p_uthread_unref (thr2);

	/* With implicit thread exit */
	tls_key = p_uthread_local_new (free_with_check);
	tls_key_2 = p_uthread_local_new (free_with_check);

	free_counter = 0;

	thr1 = p_uthread_create ((PUThreadFunc) test_thread_tls_create_func,
				 NULL,
				 TRUE,
				 NULL);

	thr2 = p_uthread_create ((PUThreadFunc) test_thread_tls_create_func,
				 NULL,
				 TRUE,
				 NULL);

	P_TEST_REQUIRE (thr1 != NULL);
	P_TEST_REQUIRE (thr2 != NULL);

	p_uthread_join (thr1);
	p_uthread_join (thr2);

	P_TEST_CHECK (free_counter == 4);

	p_uthread_local_free (tls_key);
	p_uthread_local_free (tls_key_2);
	p_uthread_unref (thr1);
	p_uthread_unref (thr2);

	p_libsys_shutdown ();
}
P_TEST_CASE_END ()

P_TEST_SUITE_BEGIN()
{
	P_TEST_SUITE_RUN_CASE (puthread_nomem_test);
	P_TEST_SUITE_RUN_CASE (puthread_bad_input_test);
	P_TEST_SUITE_RUN_CASE (puthread_general_test);
	P_TEST_SUITE_RUN_CASE (puthread_nonjoinable_test);
	P_TEST_SUITE_RUN_CASE (puthread_tls_test);
}
P_TEST_SUITE_END()
