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

#ifndef PLIB_TESTS_STATIC
#  define BOOST_TEST_DYN_LINK
#endif

#define BOOST_TEST_MODULE pmem_test

#include "plib.h"

#include <string.h>

#ifdef PLIB_TESTS_STATIC
#  include <boost/test/included/unit_test.hpp>
#else
#  include <boost/test/unit_test.hpp>
#endif

BOOST_AUTO_TEST_SUITE (BOOST_TEST_MODULE)

static ppointer pmem_alloc (psize nbytes)
{
	return (ppointer) malloc (nbytes);
}

static ppointer pmem_realloc (ppointer block, psize nbytes)
{
	return (ppointer) realloc (block, nbytes);
}

static void pmem_free (ppointer block)
{
	free (block);
}

BOOST_AUTO_TEST_CASE (pmem_general_test)
{
	PMemVTable	vtable;
	ppointer	ptr = NULL;
	pint		i;

	p_lib_init ();

	vtable.free	= NULL;
	vtable.malloc	= NULL;
	vtable.realloc	= NULL;

	BOOST_CHECK (p_malloc (0) == NULL);
	BOOST_CHECK (p_malloc0 (0) == NULL);
	BOOST_CHECK (p_realloc (NULL, 0) == NULL);
	BOOST_CHECK (p_mem_set_vtable (NULL) == FALSE);
	BOOST_CHECK (p_mem_set_vtable (&vtable) == FALSE);
	p_free (NULL);

	vtable.free	= pmem_free;
	vtable.malloc	= pmem_alloc;
	vtable.realloc	= pmem_realloc;

	BOOST_CHECK (p_mem_set_vtable (&vtable) == TRUE);

	/* Test memory allocation using system functions */
	ptr = p_malloc (1024);
	BOOST_REQUIRE (ptr != NULL);

	for (int i = 0; i < 1024; ++i)
		*(((pchar *) ptr) + i) = (pchar) (i % 127);

	for (int i = 0; i < 1024; ++i)
		BOOST_CHECK (*(((pchar *) ptr) + i) == (pchar) (i % 127));

	p_free (ptr);

	ptr = p_malloc0 (2048);
	BOOST_REQUIRE (ptr != NULL);

	for (int i = 0; i < 2048; ++i)
		BOOST_CHECK (*(((pchar *) ptr) + i) == 0);

	for (int i = 0; i < 2048; ++i)
		*(((pchar *) ptr) + i) = (pchar) (i % 127);

	for (int i = 0; i < 2048; ++i)
		BOOST_CHECK (*(((pchar *) ptr) + i) == (pchar) (i % 127));

	p_free (ptr);

	ptr = p_realloc (NULL, 1024);
	BOOST_REQUIRE (ptr != NULL);

	for (int i = 0; i < 1024; ++i)
		*(((pchar *) ptr) + i) = (pchar) (i % 127);

	ptr = p_realloc (ptr, 2048);

	for (int i = 1024; i < 2048; ++i)
		*(((pchar *) ptr) + i) = (pchar) ((i - 1) % 127);

	for (int i = 0; i < 1024; ++i)
		BOOST_CHECK (*(((pchar *) ptr) + i) == (pchar) (i % 127));

	for (int i = 1024; i < 2048; ++i)
		BOOST_CHECK (*(((pchar *) ptr) + i) == (pchar) ((i - 1) % 127));

	p_free (ptr);

	vtable.malloc	= (ppointer (*)(psize)) malloc;
	vtable.realloc	= (ppointer (*)(ppointer, psize)) realloc;
	vtable.free	= (void (*)(ppointer)) free;

	BOOST_CHECK (p_mem_set_vtable (&vtable) == TRUE);

	/* Test memory mapping */
	ptr = p_mem_mmap (0, NULL);
	BOOST_CHECK (ptr == NULL);

	ptr = p_mem_mmap (1024, NULL);
	BOOST_REQUIRE (ptr != NULL);

	for (i = 0; i < 1024; ++i)
		*(((pchar *) ptr) + i) = i % 127;

	for (i = 0; i < 1024; ++i)
		BOOST_CHECK (*(((pchar *) ptr) + i) == i % 127);

	BOOST_CHECK (p_mem_munmap (NULL, 1024, NULL) == FALSE);
	BOOST_CHECK (p_mem_munmap (ptr, 1024, NULL) == TRUE);

	p_lib_shutdown ();
}

BOOST_AUTO_TEST_SUITE_END()
