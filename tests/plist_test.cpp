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

#define BOOST_TEST_MODULE plist_test

#include "plib.h"

#include <string.h>

#ifdef PLIB_TESTS_STATIC
#  include <boost/test/included/unit_test.hpp>
#else
#  include <boost/test/unit_test.hpp>
#endif

typedef struct _TestData {
	pint test_array[3];
	pint index;
} TestData;

static void foreach_test_func (ppointer data, ppointer user_data)
{
	if (user_data == NULL)
		return;

	TestData *test_data = (TestData *) user_data;

	if (test_data->index < 0 || test_data->index > 2)
		return;

	test_data->test_array[test_data->index] = P_POINTER_TO_INT (data);
	++test_data->index;
}

BOOST_AUTO_TEST_SUITE (BOOST_TEST_MODULE)

BOOST_AUTO_TEST_CASE (plist_invalid_test)
{
	p_lib_init ();

	BOOST_CHECK (p_list_remove (NULL, NULL) == NULL);
	BOOST_CHECK (p_list_last (NULL) == NULL);
	BOOST_CHECK (p_list_length (NULL) == 0);
	BOOST_CHECK (p_list_reverse (NULL) == NULL);

	p_list_free (NULL);
	p_list_foreach (NULL, NULL, NULL);

	p_lib_shutdown ();
}

BOOST_AUTO_TEST_CASE (plist_general_test)
{
	PList		*list = NULL;
	TestData	test_data;

	p_lib_init ();

	/* Testing append */
	list = p_list_append (list, P_INT_TO_POINTER (32));
	list = p_list_append (list, P_INT_TO_POINTER (64));

	BOOST_REQUIRE (list != NULL);
	BOOST_CHECK (p_list_length (list) == 2);

	/* Testing data access */
	BOOST_CHECK (P_POINTER_TO_INT (list->data) == 32);
	BOOST_CHECK (P_POINTER_TO_INT (p_list_last(list)->data) == 64);

	/* Testing prepend */
	list = p_list_prepend (list, P_INT_TO_POINTER (128));
	BOOST_REQUIRE (list != NULL);
	BOOST_CHECK (p_list_length (list) == 3);
	BOOST_CHECK (P_POINTER_TO_INT (list->data) == 128);
	BOOST_CHECK (P_POINTER_TO_INT (p_list_last(list)->data) == 64);

	/* Testing for each loop */
	memset (&test_data, 0, sizeof (test_data));

	BOOST_REQUIRE (test_data.test_array[0] == 0);
	BOOST_REQUIRE (test_data.test_array[1] == 0);
	BOOST_REQUIRE (test_data.test_array[2] == 0);
	BOOST_REQUIRE (test_data.index == 0);

	p_list_foreach (list, (PFunc) foreach_test_func, (ppointer) &test_data);

	BOOST_CHECK (test_data.index == 3);
	BOOST_CHECK (test_data.test_array[0] == 128);
	BOOST_CHECK (test_data.test_array[1] == 32);
	BOOST_CHECK (test_data.test_array[2] == 64);

	/* Testing reverse */

	list = p_list_reverse (list);

	BOOST_CHECK (list != NULL);
	BOOST_CHECK (p_list_length (list) == 3);
	BOOST_CHECK (P_POINTER_TO_INT (list->data) == 64);
	BOOST_CHECK (P_POINTER_TO_INT (p_list_last(list)->data) == 128);

	/* Testing for each loop */
	memset (&test_data, 0, sizeof (test_data));

	BOOST_REQUIRE (test_data.test_array[0] == 0);
	BOOST_REQUIRE (test_data.test_array[1] == 0);
	BOOST_REQUIRE (test_data.test_array[2] == 0);
	BOOST_REQUIRE (test_data.index == 0);

	p_list_foreach (list, (PFunc) foreach_test_func, (ppointer) &test_data);

	BOOST_CHECK (test_data.index == 3);
	BOOST_CHECK (test_data.test_array[0] == 64);
	BOOST_CHECK (test_data.test_array[1] == 32);
	BOOST_CHECK (test_data.test_array[2] == 128);

	/* Testing remove */
	list = p_list_remove (list, P_INT_TO_POINTER (32));
	BOOST_REQUIRE (list != NULL);
	BOOST_CHECK (p_list_length (list) == 2);

	list = p_list_remove (list, P_INT_TO_POINTER (128));
	BOOST_REQUIRE (list != NULL);
	BOOST_CHECK (p_list_length (list) == 1);

	list = p_list_remove (list, P_INT_TO_POINTER (256));
	BOOST_REQUIRE (list != NULL);
	BOOST_CHECK (p_list_length (list) == 1);

	list = p_list_remove (list, P_INT_TO_POINTER (64));
	BOOST_REQUIRE (list == NULL);
	BOOST_CHECK (p_list_length (list) == 0);

	p_list_free (list);

	p_lib_shutdown ();
}

BOOST_AUTO_TEST_SUITE_END()
