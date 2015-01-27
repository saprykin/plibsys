/*
 * Copyright (C) 2013-2014 Alexander Saprykin <xelfium@gmail.com>
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

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE pstring_test

#include "plib.h"

#include <string.h>
#include <float.h>
#include <math.h>

#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE (BOOST_TEST_MODULE)

BOOST_AUTO_TEST_CASE (strdup_test)
{
	p_lib_init ();

	const pchar *test_str_1 = "Test string";

	pchar *new_string = p_strdup (test_str_1);
	BOOST_CHECK (new_string != NULL);
	p_free (new_string);

	p_lib_shutdown ();
}

BOOST_AUTO_TEST_CASE (strchomp_test)
{
	p_lib_init ();

	const pchar *test_chomp_str_orig = "Test chomp string";
	const pchar *test_chomp_str_1 = "Test chomp string  ";
	const pchar *test_chomp_str_2 = "\n\nTest chomp string  ";
	const pchar *test_chomp_str_3 = "\n\rTest chomp string  \n";
	const pchar *test_chomp_str_4 = "Test chomp string\n\n";
	const pchar *test_chomp_str_5 = "  \rTest chomp string \n\n  ";
	const pchar *test_chomp_str_6 = "  \rI\n\n  ";
	const pchar *test_chomp_str_7 = "\n";
	const pchar *test_chomp_str_8 = "I";
	const pchar *test_chomp_str_9 = "";
	const pchar *test_chomp_str_10 = " ";
	const pchar *test_chomp_str_11 = NULL;

	pchar *new_string = p_strchomp (test_chomp_str_1);
	BOOST_REQUIRE (new_string != NULL);
	BOOST_CHECK_EQUAL (strcmp (test_chomp_str_orig, new_string), 0);
	p_free (new_string);

	new_string = p_strchomp (test_chomp_str_2);
	BOOST_REQUIRE (new_string != NULL);
	BOOST_CHECK_EQUAL (strcmp (test_chomp_str_orig, new_string), 0);
	p_free (new_string);

	new_string = p_strchomp (test_chomp_str_3);
	BOOST_REQUIRE (new_string != NULL);
	BOOST_CHECK_EQUAL (strcmp (test_chomp_str_orig, new_string), 0);
	p_free (new_string);

	new_string = p_strchomp (test_chomp_str_4);
	BOOST_REQUIRE (new_string != NULL);
	BOOST_CHECK_EQUAL (strcmp (test_chomp_str_orig, new_string), 0);
	p_free (new_string);

	new_string = p_strchomp (test_chomp_str_5);
	BOOST_REQUIRE (new_string != NULL);
	BOOST_CHECK_EQUAL (strcmp (test_chomp_str_orig, new_string), 0);
	p_free (new_string);

	new_string = p_strchomp (test_chomp_str_6);
	BOOST_REQUIRE (new_string != NULL);
	BOOST_CHECK_EQUAL (strcmp ("I", new_string), 0);
	p_free (new_string);

	new_string = p_strchomp (test_chomp_str_7);
	BOOST_REQUIRE (new_string != NULL);
	BOOST_CHECK (*new_string == '\0');
	p_free (new_string);

	new_string = p_strchomp (test_chomp_str_8);
	BOOST_REQUIRE (new_string != NULL);
	BOOST_CHECK_EQUAL (strcmp ("I", new_string), 0);
	p_free (new_string);

	new_string = p_strchomp (test_chomp_str_9);
	BOOST_REQUIRE (new_string != NULL);
	BOOST_CHECK (*new_string == '\0');
	p_free (new_string);

	new_string = p_strchomp (test_chomp_str_10);
	BOOST_REQUIRE (new_string != NULL);
	BOOST_CHECK (*new_string == '\0');
	p_free (new_string);

	new_string = p_strchomp (test_chomp_str_11);
	BOOST_CHECK (new_string == NULL);

	p_lib_shutdown ();
}

BOOST_AUTO_TEST_CASE (strtok_test)
{
	p_lib_init ();

	BOOST_CHECK (p_strtok (NULL, NULL, NULL) == NULL);

	/* First string */
	pchar test_string[] = "1,2,3";
	pchar *token, *next_token;

	token = p_strtok (test_string, ",", &next_token);
	BOOST_CHECK (token != NULL);
	BOOST_CHECK (strcmp (token, "1") == 0);

	token = p_strtok (NULL, ",", &next_token);
	BOOST_CHECK (token != NULL);
	BOOST_CHECK (strcmp (token, "2") == 0);

	token = p_strtok (NULL, ",", &next_token);
	BOOST_CHECK (token != NULL);
	BOOST_CHECK (strcmp (token, "3") == 0);

	token = p_strtok (NULL, ",", &next_token);
	BOOST_CHECK (token == NULL);

	/* Second string */
	pchar test_string_2[] = "Test string, to test";

	token = p_strtok (test_string_2, " ", &next_token);
	BOOST_CHECK (token != NULL);
	BOOST_CHECK (strcmp (token, "Test") == 0);

	token = p_strtok (NULL, ", ", &next_token);
	BOOST_CHECK (token != NULL);
	BOOST_CHECK (strcmp (token, "string") == 0);

	token = p_strtok (NULL, ", ", &next_token);
	BOOST_CHECK (token != NULL);
	BOOST_CHECK (strcmp (token, "to") == 0);

	token = p_strtok (NULL, ", \t\n", &next_token);
	BOOST_CHECK (token != NULL);
	BOOST_CHECK (strcmp (token, "test") == 0);

	token = p_strtok (NULL, ", \t\n", &next_token);
	BOOST_CHECK (token == NULL);

	/* Third string */
	pchar test_string_3[] = "compile\ttest\ndeploy";

	token = p_strtok (test_string_3, "\t\n", &next_token);
	BOOST_CHECK (token != NULL);
	BOOST_CHECK (strcmp (token, "compile") == 0);

	token = p_strtok (NULL, "\t\n", &next_token);
	BOOST_CHECK (token != NULL);
	BOOST_CHECK (strcmp (token, "test") == 0);

	token = p_strtok (NULL, "\t\n", &next_token);
	BOOST_CHECK (token != NULL);
	BOOST_CHECK (strcmp (token, "deploy") == 0);

	token = p_strtok (NULL, ", \t\n", &next_token);
	BOOST_CHECK (token == NULL);

	/* Fourth string */
	pchar test_string_4[] = "\t  \t\n  \t";

	token = p_strtok (test_string_4, "\t\n ", &next_token);
	BOOST_CHECK (token == NULL);

	p_lib_shutdown ();
}

BOOST_AUTO_TEST_CASE (strtod_test)
{
	p_lib_init ();

	/* Incorrect input */
	BOOST_CHECK (fabs (p_strtod (NULL)) <= DBL_EPSILON);
	BOOST_CHECK (fabs (p_strtod ("e2")) <= DBL_EPSILON);
	BOOST_CHECK (fabs (p_strtod ("e-2")) <= DBL_EPSILON);
	BOOST_CHECK (fabs (p_strtod ("-e2")) <= DBL_EPSILON);
	BOOST_CHECK (fabs (p_strtod ("-e-2")) <= DBL_EPSILON);
	BOOST_CHECK (fabs (p_strtod ("0,3")) <= DBL_EPSILON);
	BOOST_CHECK (fabs (p_strtod ("12,3") - 12.0) <= DBL_EPSILON);

	/* Correct input */
	BOOST_CHECK (fabs (p_strtod ("0")) <= DBL_EPSILON);
	BOOST_CHECK (fabs (p_strtod ("0.0")) <= DBL_EPSILON);
	BOOST_CHECK (fabs (p_strtod ("-0")) <= DBL_EPSILON);
	BOOST_CHECK (fabs (p_strtod ("-0.0")) <= DBL_EPSILON);
	BOOST_CHECK (fabs (p_strtod ("3.14") - 3.14) <= DBL_EPSILON);
	BOOST_CHECK (fabs (p_strtod ("-12.256") - (-12.256)) <= DBL_EPSILON);
	BOOST_CHECK (fabs (p_strtod ("0.056") - 0.056) <= DBL_EPSILON);
	BOOST_CHECK (fabs (p_strtod ("-0.057") - (-0.057)) <= DBL_EPSILON);
	BOOST_CHECK (fabs (p_strtod ("1.5423e2") - 154.23) <= DBL_EPSILON);
	BOOST_CHECK (fabs (p_strtod ("1e3") - 1000.0) <= DBL_EPSILON);
	BOOST_CHECK (fabs (p_strtod ("-2.56e1") - (-25.6)) <= DBL_EPSILON);
	BOOST_CHECK (fabs (p_strtod ("123e-2") - 1.23) <= DBL_EPSILON);
	BOOST_CHECK (fabs (p_strtod ("3.14e-1") - 0.314) <= DBL_EPSILON);

	p_lib_shutdown ();
}

BOOST_AUTO_TEST_SUITE_END()
