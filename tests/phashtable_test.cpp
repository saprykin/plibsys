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
#include <time.h>

P_TEST_MODULE_INIT ();

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

P_TEST_CASE_BEGIN (phashtable_nomem_test)
{
	p_libsys_init ();

	PHashTable *table = p_hash_table_new ();
	P_TEST_CHECK (table != NULL);

	PMemVTable vtable;

	vtable.free    = pmem_free;
	vtable.malloc  = pmem_alloc;
	vtable.realloc = pmem_realloc;

	P_TEST_CHECK (p_mem_set_vtable (&vtable) == TRUE);

	P_TEST_CHECK (p_hash_table_new () == NULL);
	p_hash_table_insert (table, PINT_TO_POINTER (1), PINT_TO_POINTER (10));
	P_TEST_CHECK (p_hash_table_keys (table) == NULL);
	P_TEST_CHECK (p_hash_table_values (table) == NULL);

	p_mem_restore_vtable ();

	p_hash_table_free (table);

	p_libsys_shutdown ();
}
P_TEST_CASE_END ()

P_TEST_CASE_BEGIN (phashtable_invalid_test)
{
	p_libsys_init ();

	P_TEST_CHECK (p_hash_table_keys (NULL) == NULL);
	P_TEST_CHECK (p_hash_table_values (NULL) == NULL);
	P_TEST_CHECK (p_hash_table_lookup (NULL, NULL) == NULL);
	P_TEST_CHECK (p_hash_table_lookup_by_value (NULL, NULL, NULL) == NULL);
	p_hash_table_insert (NULL, NULL, NULL);
	p_hash_table_remove (NULL, NULL);
	p_hash_table_free (NULL);

	p_libsys_shutdown ();
}
P_TEST_CASE_END ()

P_TEST_CASE_BEGIN (phashtable_general_test)
{
	PHashTable	*table = NULL;
	PList		*list = NULL;

	p_libsys_init ();

	table = p_hash_table_new ();
	P_TEST_REQUIRE (table != NULL);

	/* Test for NULL key */
	p_hash_table_insert (table, NULL, PINT_TO_POINTER (1));
	list = p_hash_table_keys (table);
	P_TEST_REQUIRE (p_list_length (list) == 1);
	P_TEST_REQUIRE (PPOINTER_TO_INT (list->data) == 0);
	p_list_free (list);
	list = p_hash_table_values (table);
	P_TEST_REQUIRE (p_list_length (list) == 1);
	P_TEST_REQUIRE (PPOINTER_TO_INT (list->data) == 1);
	p_list_free (list);
	p_hash_table_remove (table, NULL);

	/* Test for insertion */
	p_hash_table_insert (table, PINT_TO_POINTER (1), PINT_TO_POINTER (10));
	list = p_hash_table_values (table);
	P_TEST_REQUIRE (list != NULL);
	P_TEST_REQUIRE (p_list_length (list) == 1);
	P_TEST_REQUIRE (PPOINTER_TO_INT (list->data) == 10);
	p_list_free (list);
	list = p_hash_table_keys (table);
	P_TEST_REQUIRE (list != NULL);
	P_TEST_REQUIRE (p_list_length (list) == 1);
	P_TEST_REQUIRE (PPOINTER_TO_INT (list->data) == 1);
	p_list_free (list);

	/* False remove */
	p_hash_table_remove (table, PINT_TO_POINTER (2));
	list = p_hash_table_values (table);
	P_TEST_REQUIRE (list != NULL);
	P_TEST_REQUIRE (p_list_length (list) == 1);
	P_TEST_REQUIRE (PPOINTER_TO_INT (list->data) == 10);
	p_list_free (list);
	list = p_hash_table_keys (table);
	P_TEST_REQUIRE (list != NULL);
	P_TEST_REQUIRE (p_list_length (list) == 1);
	P_TEST_REQUIRE (PPOINTER_TO_INT (list->data) == 1);
	p_list_free (list);

	/* Replace existing value */
	p_hash_table_insert (table, PINT_TO_POINTER (1), PINT_TO_POINTER (15));
	list = p_hash_table_values (table);
	P_TEST_REQUIRE (list != NULL);
	P_TEST_REQUIRE (p_list_length (list) == 1);
	P_TEST_REQUIRE (PPOINTER_TO_INT (list->data) == 15);
	p_list_free (list);
	list = p_hash_table_keys (table);
	P_TEST_REQUIRE (list != NULL);
	P_TEST_REQUIRE (p_list_length (list) == 1);
	P_TEST_REQUIRE (PPOINTER_TO_INT (list->data) == 1);
	p_list_free (list);

	/* More insertion */
	p_hash_table_insert (table, PINT_TO_POINTER (2), PINT_TO_POINTER (20));
	p_hash_table_insert (table, PINT_TO_POINTER (3), PINT_TO_POINTER (30));

	list = p_hash_table_values (table);
	P_TEST_REQUIRE (list != NULL);
	P_TEST_REQUIRE (p_list_length (list) == 3);
	P_TEST_REQUIRE (PPOINTER_TO_INT (list->data) +
		       PPOINTER_TO_INT (list->next->data) +
		       PPOINTER_TO_INT (list->next->next->data) == 65);
	p_list_free (list);
	list = p_hash_table_keys (table);
	P_TEST_REQUIRE (list != NULL);
	P_TEST_REQUIRE (p_list_length (list) == 3);
	P_TEST_REQUIRE (PPOINTER_TO_INT (list->data) +
		       PPOINTER_TO_INT (list->next->data) +
		       PPOINTER_TO_INT (list->next->next->data) == 6);
	p_list_free (list);

	P_TEST_CHECK (PPOINTER_TO_INT (p_hash_table_lookup (table, PINT_TO_POINTER (1))) == 15);
	P_TEST_CHECK (PPOINTER_TO_INT (p_hash_table_lookup (table, PINT_TO_POINTER (2))) == 20);
	P_TEST_CHECK (PPOINTER_TO_INT (p_hash_table_lookup (table, PINT_TO_POINTER (3))) == 30);
	P_TEST_CHECK (p_hash_table_lookup (table, PINT_TO_POINTER (4)) == (ppointer) -1);
	p_hash_table_insert (table, PINT_TO_POINTER (22), PINT_TO_POINTER (20));

	list = p_hash_table_lookup_by_value (table,
					     PINT_TO_POINTER (19),
					     (PCompareFunc) test_hash_table_values);
	P_TEST_REQUIRE (list != NULL);
	P_TEST_REQUIRE (p_list_length (list) == 3);
	P_TEST_REQUIRE (PPOINTER_TO_INT (list->data) +
		       PPOINTER_TO_INT (list->next->data) +
		       PPOINTER_TO_INT (list->next->next->data) == 27);
	p_list_free (list);

	list = p_hash_table_lookup_by_value (table,
					     PINT_TO_POINTER (20),
					     NULL);
	P_TEST_REQUIRE (list != NULL);
	P_TEST_REQUIRE (p_list_length (list) == 2);
	P_TEST_REQUIRE (PPOINTER_TO_INT (list->data) +
		       PPOINTER_TO_INT (list->next->data) == 24);
	p_list_free (list);

	P_TEST_REQUIRE (PPOINTER_TO_INT (p_hash_table_lookup (table, PINT_TO_POINTER (22))) == 20);

	p_hash_table_remove (table, PINT_TO_POINTER (1));
	p_hash_table_remove (table, PINT_TO_POINTER (2));

	list = p_hash_table_keys (table);
	P_TEST_REQUIRE (p_list_length (list) == 2);
	p_list_free (list);
	list = p_hash_table_values (table);
	P_TEST_REQUIRE (p_list_length (list) == 2);
	p_list_free (list);

	p_hash_table_remove (table, PINT_TO_POINTER (3));

	list = p_hash_table_keys (table);
	P_TEST_REQUIRE (p_list_length (list) == 1);
	P_TEST_REQUIRE (PPOINTER_TO_INT (list->data) == 22);
	p_list_free (list);
	list = p_hash_table_values (table);
	P_TEST_REQUIRE (p_list_length (list) == 1);
	P_TEST_REQUIRE (PPOINTER_TO_INT (list->data) == 20);
	p_list_free (list);

	p_hash_table_remove (table, PINT_TO_POINTER (22));

	P_TEST_REQUIRE (p_hash_table_keys (table) == NULL);
	P_TEST_REQUIRE (p_hash_table_values (table) == NULL);

	p_hash_table_free (table);

	p_libsys_shutdown ();
}
P_TEST_CASE_END ()

P_TEST_CASE_BEGIN (phashtable_stress_test)
{
	p_libsys_init ();

	PHashTable *table = p_hash_table_new ();
	P_TEST_REQUIRE (table != NULL);

	srand ((unsigned int) time (NULL));

	int counter = 0;

	pint *keys   = (pint *) p_malloc0 (PHASHTABLE_STRESS_COUNT * sizeof (pint));
	pint *values = (pint *) p_malloc0 (PHASHTABLE_STRESS_COUNT * sizeof (pint));

	P_TEST_REQUIRE (keys != NULL);
	P_TEST_REQUIRE (values != NULL);

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
		P_TEST_CHECK (p_hash_table_lookup (table, PINT_TO_POINTER (keys[i])) ==
			     PINT_TO_POINTER (values[i]));

		p_hash_table_remove (table, PINT_TO_POINTER (keys[i]));
		P_TEST_CHECK (p_hash_table_lookup (table, PINT_TO_POINTER (keys[i])) == (ppointer) (-1));
	}

	P_TEST_CHECK (p_hash_table_keys (table) == NULL);
	P_TEST_CHECK (p_hash_table_values (table) == NULL);

	p_free (keys);
	p_free (values);

	p_hash_table_free (table);

	/* Try to free at once */
	table = p_hash_table_new ();
	P_TEST_REQUIRE (table != NULL);

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
P_TEST_CASE_END ()

P_TEST_SUITE_BEGIN()
{
	P_TEST_SUITE_RUN_CASE (phashtable_nomem_test);
	P_TEST_SUITE_RUN_CASE (phashtable_invalid_test);
	P_TEST_SUITE_RUN_CASE (phashtable_general_test);
	P_TEST_SUITE_RUN_CASE (phashtable_stress_test);
}
P_TEST_SUITE_END()
