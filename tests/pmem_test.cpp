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

P_TEST_CASE_BEGIN (pmem_bad_input_test)
{
	PMemVTable vtable;

	p_libsys_init ();

	vtable.free    = NULL;
	vtable.malloc  = NULL;
	vtable.realloc = NULL;

	P_TEST_CHECK (p_malloc (0) == NULL);
	P_TEST_CHECK (p_malloc0 (0) == NULL);
	P_TEST_CHECK (p_realloc (NULL, 0) == NULL);
	P_TEST_CHECK (p_mem_set_vtable (NULL) == FALSE);
	P_TEST_CHECK (p_mem_set_vtable (&vtable) == FALSE);
	p_free (NULL);

	p_libsys_shutdown ();
}
P_TEST_CASE_END ()

P_TEST_CASE_BEGIN (pmem_general_test)
{
	PMemVTable	vtable;
	ppointer	ptr = NULL;
	pint		i;

	p_libsys_init ();

	alloc_counter   = 0;
	realloc_counter = 0;
	free_counter    = 0;

	vtable.free    = pmem_free;
	vtable.malloc  = pmem_alloc;
	vtable.realloc = pmem_realloc;

	P_TEST_CHECK (p_mem_set_vtable (&vtable) == TRUE);

	/* Test memory allocation using system functions */
	ptr = p_malloc (1024);
	P_TEST_REQUIRE (ptr != NULL);

	for (int i = 0; i < 1024; ++i)
		*(((pchar *) ptr) + i) = (pchar) (i % 127);

	for (int i = 0; i < 1024; ++i)
		P_TEST_CHECK (*(((pchar *) ptr) + i) == (pchar) (i % 127));

	p_free (ptr);

	ptr = p_malloc0 (2048);
	P_TEST_REQUIRE (ptr != NULL);

	for (int i = 0; i < 2048; ++i)
		P_TEST_CHECK (*(((pchar *) ptr) + i) == 0);

	for (int i = 0; i < 2048; ++i)
		*(((pchar *) ptr) + i) = (pchar) (i % 127);

	for (int i = 0; i < 2048; ++i)
		P_TEST_CHECK (*(((pchar *) ptr) + i) == (pchar) (i % 127));

	p_free (ptr);

	ptr = p_realloc (NULL, 1024);
	P_TEST_REQUIRE (ptr != NULL);

	for (int i = 0; i < 1024; ++i)
		*(((pchar *) ptr) + i) = (pchar) (i % 127);

	ptr = p_realloc (ptr, 2048);

	for (int i = 1024; i < 2048; ++i)
		*(((pchar *) ptr) + i) = (pchar) ((i - 1) % 127);

	for (int i = 0; i < 1024; ++i)
		P_TEST_CHECK (*(((pchar *) ptr) + i) == (pchar) (i % 127));

	for (int i = 1024; i < 2048; ++i)
		P_TEST_CHECK (*(((pchar *) ptr) + i) == (pchar) ((i - 1) % 127));

	p_free (ptr);

	P_TEST_CHECK (alloc_counter > 0);
	P_TEST_CHECK (realloc_counter > 0);
	P_TEST_CHECK (free_counter > 0);

	p_mem_restore_vtable ();

	/* Test memory mapping */
	ptr = p_mem_mmap (0, NULL);
	P_TEST_CHECK (ptr == NULL);

	ptr = p_mem_mmap (1024, NULL);
	P_TEST_REQUIRE (ptr != NULL);

	for (i = 0; i < 1024; ++i)
		*(((pchar *) ptr) + i) = i % 127;

	for (i = 0; i < 1024; ++i)
		P_TEST_CHECK (*(((pchar *) ptr) + i) == i % 127);

	P_TEST_CHECK (p_mem_munmap (NULL, 1024, NULL) == FALSE);
	P_TEST_CHECK (p_mem_munmap (ptr, 1024, NULL) == TRUE);

	p_libsys_shutdown ();
}
P_TEST_CASE_END ()

P_TEST_SUITE_BEGIN()
{
	P_TEST_SUITE_RUN_CASE (pmem_bad_input_test);
	P_TEST_SUITE_RUN_CASE (pmem_general_test);
}
P_TEST_SUITE_END()
