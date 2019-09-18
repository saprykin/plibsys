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

P_TEST_MODULE_INIT ();

#define PSPINLOCK_MAX_VAL 10

static pint        spinlock_test_val = 0;
static PSpinLock * global_spinlock   = NULL;

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

static void * spinlock_test_thread (void *)
{
	pint	i;

	for (i = 0; i < 1000; ++i) {
		if (!p_spinlock_trylock (global_spinlock)) {
			if (!p_spinlock_lock (global_spinlock))
				p_uthread_exit (1);
		}

		if (spinlock_test_val == PSPINLOCK_MAX_VAL)
			--spinlock_test_val;
		else {
			p_uthread_sleep (1);
			++spinlock_test_val;
		}

		if (!p_spinlock_unlock (global_spinlock))
			p_uthread_exit (1);
	}

	p_uthread_exit (0);

	return NULL;
}

P_TEST_CASE_BEGIN (pspinlock_nomem_test)
{
	p_libsys_init ();

	PMemVTable vtable;

	vtable.free    = pmem_free;
	vtable.malloc  = pmem_alloc;
	vtable.realloc = pmem_realloc;

	P_TEST_CHECK (p_mem_set_vtable (&vtable) == TRUE);
	P_TEST_CHECK (p_spinlock_new () == NULL);

	p_mem_restore_vtable ();

	p_libsys_shutdown ();
}
P_TEST_CASE_END ()

P_TEST_CASE_BEGIN (pspinlock_bad_input_test)
{
	p_libsys_init ();

	P_TEST_REQUIRE (p_spinlock_lock (NULL) == FALSE);
	P_TEST_REQUIRE (p_spinlock_unlock (NULL) == FALSE);
	P_TEST_REQUIRE (p_spinlock_trylock (NULL) == FALSE);
	p_spinlock_free (NULL);

	p_libsys_shutdown ();
}
P_TEST_CASE_END ()

P_TEST_CASE_BEGIN (pspinlock_general_test)
{
	PUThread *thr1, *thr2;

	p_libsys_init ();

	spinlock_test_val = PSPINLOCK_MAX_VAL;
	global_spinlock   = p_spinlock_new ();

	P_TEST_REQUIRE (global_spinlock != NULL);

	thr1 = p_uthread_create ((PUThreadFunc) spinlock_test_thread, NULL, TRUE, NULL);
	P_TEST_REQUIRE (thr1 != NULL);

	thr2 = p_uthread_create ((PUThreadFunc) spinlock_test_thread, NULL, TRUE, NULL);
	P_TEST_REQUIRE (thr2 != NULL);

	P_TEST_CHECK (p_uthread_join (thr1) == 0);
	P_TEST_CHECK (p_uthread_join (thr2) == 0);

	P_TEST_REQUIRE (spinlock_test_val == PSPINLOCK_MAX_VAL);

	p_uthread_unref (thr1);
	p_uthread_unref (thr2);
	p_spinlock_free (global_spinlock);

	p_libsys_shutdown ();
}
P_TEST_CASE_END ()

P_TEST_SUITE_BEGIN()
{
	P_TEST_SUITE_RUN_CASE (pspinlock_nomem_test);
	P_TEST_SUITE_RUN_CASE (pspinlock_bad_input_test);
	P_TEST_SUITE_RUN_CASE (pspinlock_general_test);
}
P_TEST_SUITE_END()
