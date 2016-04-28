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

#ifndef PLIBSYS_TESTS_STATIC
#  define BOOST_TEST_DYN_LINK
#endif

#define BOOST_TEST_MODULE phashtable_test

#include "plibsys.h"

#include <stdlib.h>
#include <time.h>

#ifdef PLIBSYS_TESTS_STATIC
#  include <boost/test/included/unit_test.hpp>
#else
#  include <boost/test/unit_test.hpp>
#endif

#define PHASHTABLE_STRESS_COUNT	10000

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

static int test_hash_table_values (pconstpointer a, pconstpointer b)
{
	return a > b ? 0 : (a < b ? -1 : 1);
}

BOOST_AUTO_TEST_SUITE (BOOST_TEST_MODULE)

BOOST_AUTO_TEST_CASE (phashtable_nomem_test)
{
	p_libsys_init ();

	PHashTable *table = p_hash_table_new ();
	BOOST_CHECK (table != NULL);

	PMemVTable vtable;

	vtable.free	= pmem_free;
	vtable.malloc	= pmem_alloc;
	vtable.realloc	= pmem_realloc;

	BOOST_CHECK (p_mem_set_vtable (&vtable) == TRUE);

	BOOST_CHECK (p_hash_table_new () == NULL);
	p_hash_table_insert (table, PINT_TO_POINTER (1), PINT_TO_POINTER (10));
	BOOST_CHECK (p_hash_table_keys (table) == NULL);
	BOOST_CHECK (p_hash_table_values (table) == NULL);

	vtable.malloc	= (ppointer (*)(psize)) malloc;
	vtable.realloc	= (ppointer (*)(ppointer, psize)) realloc;
	vtable.free	= (void (*)(ppointer)) free;

	BOOST_CHECK (p_mem_set_vtable (&vtable) == TRUE);

	p_hash_table_free (table);

	p_libsys_shutdown ();
}

BOOST_AUTO_TEST_CASE (phashtable_invalid_test)
{
	p_libsys_init ();

	BOOST_CHECK (p_hash_table_keys (NULL) == NULL);
	BOOST_CHECK (p_hash_table_values (NULL) == NULL);
	BOOST_CHECK (p_hash_table_lookup (NULL, NULL) == NULL);
	BOOST_CHECK (p_hash_table_lookup_by_value (NULL, NULL, NULL) == NULL);
	p_hash_table_insert (NULL, NULL, NULL);
	p_hash_table_remove (NULL, NULL);
	p_hash_table_free (NULL);

	p_libsys_shutdown ();
}

BOOST_AUTO_TEST_CASE (phashtable_general_test)
{
	PHashTable	*table = NULL;
	PList		*list = NULL;

	p_libsys_init ();

	table = p_hash_table_new ();
	BOOST_REQUIRE (table != NULL);

	/* Test for NULL key */
	p_hash_table_insert (table, NULL, PINT_TO_POINTER (1));
	list = p_hash_table_keys (table);
	BOOST_REQUIRE (p_list_length (list) == 1);
	BOOST_REQUIRE (PPOINTER_TO_INT (list->data) == 0);
	p_list_free (list);
	list = p_hash_table_values (table);
	BOOST_REQUIRE (p_list_length (list) == 1);
	BOOST_REQUIRE (PPOINTER_TO_INT (list->data) == 1);
	p_list_free (list);
	p_hash_table_remove (table, NULL);

	/* Test for insertion */
	p_hash_table_insert (table, PINT_TO_POINTER (1), PINT_TO_POINTER (10));
	list = p_hash_table_values (table);
	BOOST_REQUIRE (list != NULL);
	BOOST_REQUIRE (p_list_length (list) == 1);
	BOOST_REQUIRE (PPOINTER_TO_INT (list->data) == 10);
	p_list_free (list);
	list = p_hash_table_keys (table);
	BOOST_REQUIRE (list != NULL);
	BOOST_REQUIRE (p_list_length (list) == 1);
	BOOST_REQUIRE (PPOINTER_TO_INT (list->data) == 1);
	p_list_free (list);

	/* False remove */
	p_hash_table_remove (table, PINT_TO_POINTER (2));
	list = p_hash_table_values (table);
	BOOST_REQUIRE (list != NULL);
	BOOST_REQUIRE (p_list_length (list) == 1);
	BOOST_REQUIRE (PPOINTER_TO_INT (list->data) == 10);
	p_list_free (list);
	list = p_hash_table_keys (table);
	BOOST_REQUIRE (list != NULL);
	BOOST_REQUIRE (p_list_length (list) == 1);
	BOOST_REQUIRE (PPOINTER_TO_INT (list->data) == 1);
	p_list_free (list);

	/* Replace existing value */
	p_hash_table_insert (table, PINT_TO_POINTER (1), PINT_TO_POINTER (15));
	list = p_hash_table_values (table);
	BOOST_REQUIRE (list != NULL);
	BOOST_REQUIRE (p_list_length (list) == 1);
	BOOST_REQUIRE (PPOINTER_TO_INT (list->data) == 15);
	p_list_free (list);
	list = p_hash_table_keys (table);
	BOOST_REQUIRE (list != NULL);
	BOOST_REQUIRE (p_list_length (list) == 1);
	BOOST_REQUIRE (PPOINTER_TO_INT (list->data) == 1);
	p_list_free (list);

	/* More insertion */
	p_hash_table_insert (table, PINT_TO_POINTER (2), PINT_TO_POINTER (20));
	p_hash_table_insert (table, PINT_TO_POINTER (3), PINT_TO_POINTER (30));

	list = p_hash_table_values (table);
	BOOST_REQUIRE (list != NULL);
	BOOST_REQUIRE (p_list_length (list) == 3);
	BOOST_REQUIRE (PPOINTER_TO_INT (list->data) +
		       PPOINTER_TO_INT (list->next->data) +
		       PPOINTER_TO_INT (list->next->next->data) == 65);
	p_list_free (list);
	list = p_hash_table_keys (table);
	BOOST_REQUIRE (list != NULL);
	BOOST_REQUIRE (p_list_length (list) == 3);
	BOOST_REQUIRE (PPOINTER_TO_INT (list->data) +
		       PPOINTER_TO_INT (list->next->data) +
		       PPOINTER_TO_INT (list->next->next->data) == 6);
	p_list_free (list);

	BOOST_CHECK (PPOINTER_TO_INT (p_hash_table_lookup (table, PINT_TO_POINTER (1))) == 15);
	BOOST_CHECK (PPOINTER_TO_INT (p_hash_table_lookup (table, PINT_TO_POINTER (2))) == 20);
	BOOST_CHECK (PPOINTER_TO_INT (p_hash_table_lookup (table, PINT_TO_POINTER (3))) == 30);
	BOOST_CHECK (p_hash_table_lookup (table, PINT_TO_POINTER (4)) == (ppointer) -1);
	p_hash_table_insert (table, PINT_TO_POINTER (22), PINT_TO_POINTER (20));

	list = p_hash_table_lookup_by_value (table,
					     PINT_TO_POINTER (19),
					     (PCompareFunc) test_hash_table_values);
	BOOST_REQUIRE (list != NULL);
	BOOST_REQUIRE (p_list_length (list) == 3);
	BOOST_REQUIRE (PPOINTER_TO_INT (list->data) +
		       PPOINTER_TO_INT (list->next->data) +
		       PPOINTER_TO_INT (list->next->next->data) == 27);
	p_list_free (list);

	list = p_hash_table_lookup_by_value (table,
					     PINT_TO_POINTER (20),
					     NULL);
	BOOST_REQUIRE (list != NULL);
	BOOST_REQUIRE (p_list_length (list) == 2);
	BOOST_REQUIRE (PPOINTER_TO_INT (list->data) +
		       PPOINTER_TO_INT (list->next->data) == 24);
	p_list_free (list);

	BOOST_REQUIRE (PPOINTER_TO_INT (p_hash_table_lookup (table, PINT_TO_POINTER (22))) == 20);

	p_hash_table_remove (table, PINT_TO_POINTER (1));
	p_hash_table_remove (table, PINT_TO_POINTER (2));

	list = p_hash_table_keys (table);
	BOOST_REQUIRE (p_list_length (list) == 2);
	p_list_free (list);
	list = p_hash_table_values (table);
	BOOST_REQUIRE (p_list_length (list) == 2);
	p_list_free (list);

	p_hash_table_remove (table, PINT_TO_POINTER (3));

	list = p_hash_table_keys (table);
	BOOST_REQUIRE (p_list_length (list) == 1);
	BOOST_REQUIRE (PPOINTER_TO_INT (list->data) == 22);
	p_list_free (list);
	list = p_hash_table_values (table);
	BOOST_REQUIRE (p_list_length (list) == 1);
	BOOST_REQUIRE (PPOINTER_TO_INT (list->data) == 20);
	p_list_free (list);

	p_hash_table_remove (table, PINT_TO_POINTER (22));

	BOOST_REQUIRE (p_hash_table_keys (table) == NULL);
	BOOST_REQUIRE (p_hash_table_values (table) == NULL);

	p_hash_table_free (table);

	p_libsys_shutdown ();
}

BOOST_AUTO_TEST_CASE (phashtable_stress_test)
{
	p_libsys_init ();

	PHashTable *table = p_hash_table_new ();
	BOOST_REQUIRE (table != NULL);

	srand ((unsigned int) time (NULL));

	int counter = 0;

	pint *keys   = (pint *) p_malloc0 (PHASHTABLE_STRESS_COUNT * sizeof (pint));
	pint *values = (pint *) p_malloc0 (PHASHTABLE_STRESS_COUNT * sizeof (pint));

	BOOST_REQUIRE (keys != NULL);
	BOOST_REQUIRE (values != NULL);

	while (counter != PHASHTABLE_STRESS_COUNT) {
		pint rand_number = rand ();

		if (p_hash_table_lookup (table, PINT_TO_POINTER (rand_number)) != (ppointer) (-1))
			continue;

		keys[counter]   = rand_number;
		values[counter] = rand () + 1;

		p_hash_table_remove (table, PINT_TO_POINTER (keys[counter]));
		p_hash_table_insert (table, PINT_TO_POINTER (keys[counter]), PINT_TO_POINTER (values[counter]));

		++counter;
	}

	for (int i = 0; i < PHASHTABLE_STRESS_COUNT; ++i) {
		BOOST_CHECK (p_hash_table_lookup (table, PINT_TO_POINTER (keys[i])) ==
			     PINT_TO_POINTER (values[i]));

		p_hash_table_remove (table, PINT_TO_POINTER (keys[i]));
		BOOST_CHECK (p_hash_table_lookup (table, PINT_TO_POINTER (keys[i])) == (ppointer) (-1));
	}

	BOOST_CHECK (p_hash_table_keys (table) == NULL);
	BOOST_CHECK (p_hash_table_values (table) == NULL);

	p_free (keys);
	p_free (values);

	p_hash_table_free (table);

	/* Try to free at once */
	table = p_hash_table_new ();
	BOOST_REQUIRE (table != NULL);

	counter = 0;

	while (counter != PHASHTABLE_STRESS_COUNT) {
		pint rand_number = rand ();

		if (p_hash_table_lookup (table, PINT_TO_POINTER (rand_number)) != (ppointer) (-1))
			continue;

		p_hash_table_insert (table, PINT_TO_POINTER (rand_number), PINT_TO_POINTER (rand () + 1));

		++counter;
	}

	p_hash_table_free (table);

	p_libsys_shutdown ();
}

BOOST_AUTO_TEST_SUITE_END()
