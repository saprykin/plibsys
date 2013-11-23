#define BOOST_TEST_MODULE phashtable_test

#include "plib.h"

#include <string.h>

#include <boost/test/included/unit_test.hpp>

static int test_hash_table_values (pconstpointer a, pconstpointer b, ppointer data)
{
	P_UNUSED (data);

	return a > b ? 0 : (a < b ? -1 : 1);
}

BOOST_AUTO_TEST_SUITE (BOOST_TEST_MODULE)

BOOST_AUTO_TEST_CASE (phashtable_general_test)
{
	PHashTable	*table = NULL;
	PList		*list = NULL;

	BOOST_CHECK (p_hash_table_keys (table) == NULL);
	BOOST_CHECK (p_hash_table_values (table) == NULL);
	BOOST_CHECK (p_hash_table_lookup (table, NULL) == NULL);
	BOOST_CHECK (p_hash_table_lookup_by_value (table, NULL, NULL) == NULL);
	p_hash_table_insert (table, NULL, NULL);
	p_hash_table_remove (table, NULL);
	p_hash_table_free (table);

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

	/* More insertion */
	p_hash_table_insert (table, PINT_TO_POINTER (2), PINT_TO_POINTER (20));
	p_hash_table_insert (table, PINT_TO_POINTER (3), PINT_TO_POINTER (30));

	list = p_hash_table_values (table);
	BOOST_REQUIRE (list != NULL);
	BOOST_REQUIRE (p_list_length (list) == 3);
	BOOST_REQUIRE (PPOINTER_TO_INT (list->data) +
		       PPOINTER_TO_INT (list->next->data) +
		       PPOINTER_TO_INT (list->next->next->data) == 60);
	p_list_free (list);
	list = p_hash_table_keys (table);
	BOOST_REQUIRE (list != NULL);
	BOOST_REQUIRE (p_list_length (list) == 3);
	BOOST_REQUIRE (PPOINTER_TO_INT (list->data) +
		       PPOINTER_TO_INT (list->next->data) +
		       PPOINTER_TO_INT (list->next->next->data) == 6);
	p_list_free (list);

	BOOST_CHECK (PPOINTER_TO_INT (p_hash_table_lookup (table, PINT_TO_POINTER (1))) == 10);
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
		       PPOINTER_TO_INT (list->next->next->data)== 27);
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
}

BOOST_AUTO_TEST_SUITE_END()
