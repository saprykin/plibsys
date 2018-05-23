/*
 * The MIT License
 *
 * Copyright (C) 2013-2017 Alexander Saprykin <saprykin.spb@gmail.com>
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

P_TEST_CASE_BEGIN (ptimeprofiler_nomem_test)
{
	p_libsys_init ();

	PMemVTable vtable;

	vtable.free    = pmem_free;
	vtable.malloc  = pmem_alloc;
	vtable.realloc = pmem_realloc;

	P_TEST_CHECK (p_mem_set_vtable (&vtable) == TRUE);

	P_TEST_CHECK (p_time_profiler_new () == NULL);

	p_mem_restore_vtable ();

	p_libsys_shutdown ();
}
P_TEST_CASE_END ()

P_TEST_CASE_BEGIN (ptimeprofiler_bad_input_test)
{
	p_libsys_init ();

	P_TEST_CHECK (p_time_profiler_elapsed_usecs (NULL) == 0);
	p_time_profiler_reset (NULL);
	p_time_profiler_free (NULL);

	p_libsys_shutdown ();
}
P_TEST_CASE_END ()

P_TEST_CASE_BEGIN (ptimeprofiler_general_test)
{
	PTimeProfiler	*profiler = NULL;
	puint64		prev_val, val;

	p_libsys_init ();

	profiler = p_time_profiler_new ();
	P_TEST_REQUIRE (profiler != NULL);

	p_uthread_sleep (50);
	prev_val = p_time_profiler_elapsed_usecs (profiler);
	P_TEST_CHECK (prev_val > 0);

	p_uthread_sleep (100);
	val = p_time_profiler_elapsed_usecs (profiler);
	P_TEST_CHECK (val > prev_val);
	prev_val = val;

	p_uthread_sleep (1000);
	val = p_time_profiler_elapsed_usecs (profiler);
	P_TEST_CHECK (val > prev_val);

	p_time_profiler_reset (profiler);

	p_uthread_sleep (15);
	prev_val = p_time_profiler_elapsed_usecs (profiler);
	P_TEST_CHECK (prev_val > 0);

	p_uthread_sleep (178);
	val = p_time_profiler_elapsed_usecs (profiler);
	P_TEST_CHECK (val > prev_val);

	p_time_profiler_free (profiler);

	p_libsys_shutdown ();
}
P_TEST_CASE_END ()

P_TEST_SUITE_BEGIN()
{
	P_TEST_SUITE_RUN_CASE (ptimeprofiler_nomem_test);
	P_TEST_SUITE_RUN_CASE (ptimeprofiler_bad_input_test);
	P_TEST_SUITE_RUN_CASE (ptimeprofiler_general_test);
}
P_TEST_SUITE_END()
