/*
 * Copyright (C) 2013-2017 Alexander Saprykin <xelfium@gmail.com>
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, see <http://www.gnu.org/licenses/>.
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
