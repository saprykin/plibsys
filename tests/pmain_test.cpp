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

#include <stdlib.h>
#include <string.h>

P_TEST_MODULE_INIT ();

static pint alloc_counter   = 0;
static pint realloc_counter = 0;
static pint free_counter    = 0;

extern "C" ppointer pmem_alloc (psize nbytes)
{
	++alloc_counter;
	return (ppointer) malloc (nbytes);
}

extern "C" ppointer pmem_realloc (ppointer block, psize nbytes)
{
	++realloc_counter;
	return (ppointer) realloc (block, nbytes);
}

extern "C" void pmem_free (ppointer block)
{
	++free_counter;
	free (block);
}

P_TEST_CASE_BEGIN (pmain_general_test)
{
	p_libsys_init ();
	p_libsys_shutdown ();
}
P_TEST_CASE_END ()

P_TEST_CASE_BEGIN (pmain_double_test)
{
	p_libsys_init_full (NULL);
	p_libsys_init ();
	p_libsys_shutdown ();
	p_libsys_shutdown ();
}
P_TEST_CASE_END ()

P_TEST_CASE_BEGIN (pmain_vtable_test)
{
	PMemVTable	vtable;

	vtable.free    = pmem_free;
	vtable.malloc  = pmem_alloc;
	vtable.realloc = pmem_realloc;

	p_libsys_init_full (&vtable);

	alloc_counter   = 0;
	realloc_counter = 0;
	free_counter    = 0;

	pchar *buf = (pchar *) p_malloc0 (10);
	pchar *new_buf = (pchar *) p_realloc ((ppointer) buf, 20);

	P_TEST_REQUIRE (new_buf != NULL);

	buf = new_buf;

	p_free (buf);

	P_TEST_CHECK (alloc_counter > 0);
	P_TEST_CHECK (realloc_counter > 0);
	P_TEST_CHECK (free_counter > 0);

	P_TEST_CHECK (strcmp (p_libsys_version (), PLIBSYS_VERSION_STR) == 0);

	p_libsys_shutdown ();
}
P_TEST_CASE_END ()

P_TEST_SUITE_BEGIN()
{
	P_TEST_SUITE_RUN_CASE (pmain_general_test);
	P_TEST_SUITE_RUN_CASE (pmain_double_test);
	P_TEST_SUITE_RUN_CASE (pmain_vtable_test);
}
P_TEST_SUITE_END()
