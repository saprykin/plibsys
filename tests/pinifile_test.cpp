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

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE pinifile_test

#include "plib.h"

#include <stdio.h>
#include <float.h>

#include <boost/test/unit_test.hpp>

static bool create_test_ini_file ()
{
	FILE *file = fopen ("." P_DIR_SEPARATOR "p_ini_test_file.ini", "w");

	if (file == NULL)
		return false;

	/* Empty section */
	fprintf (file, "[empty_section]\n");

	/* Numeric section */
	fprintf (file, "[numeric_section]\n");
	fprintf (file, "int_parameter_1 = 4\n");
	fprintf (file, "int_parameter_2 = 5 ;This is a comment\n");
	fprintf (file, "int_parameter_3 = 6 #This is another type of a comment\n");
	fprintf (file, "# Whole line is a comment\n");
	fprintf (file, "; Yet another comment line\n");
	fprintf (file, "float_parameter_1 = 3.24\n");
	fprintf (file, "float_parameter_2 = 0.15\n");

	/* String section */
	fprintf (file, "[string_section]\n");
	fprintf (file, "string_parameter_1 = Test string\n");
	fprintf (file, "string_parameter_2 = \"Test string with #'\"\n");
	fprintf (file, "string_parameter_3 = \n");
	fprintf (file, "string_parameter_4 = 12345 ;Comment\n");
	fprintf (file, "string_parameter_4 = 54321\n");
	fprintf (file, "string_parameter_5 = 'Test string'\n");

	/* Boolean section */
	fprintf (file, "[boolean_section]\n");
	fprintf (file, "boolean_parameter_1 = TRUE ;True value\n");
	fprintf (file, "boolean_parameter_2 = 0 ;False value\n");
	fprintf (file, "boolean_parameter_3 = false ;False value\n");

	/* List section */
	fprintf (file, "[list_section]\n");
	fprintf (file, "list_parameter_1 = {1\t2\t5\t10} ;First list\n");
	fprintf (file, "list_parameter_2 = {2.0 3.0 5.0} #Second list\n");
	fprintf (file, "list_parameter_3 = {true FALSE 1} #Last list\n");

	return fclose (file) == 0;
}

BOOST_AUTO_TEST_SUITE (BOOST_TEST_MODULE)

BOOST_AUTO_TEST_CASE (pinifile_bad_input_test)
{
	PIniFile *ini = NULL;

	p_lib_init ();

	p_ini_file_free (ini);
	BOOST_CHECK (p_ini_file_parse (ini) == FALSE);
	BOOST_CHECK (p_ini_file_is_parsed (ini) == FALSE);
	BOOST_CHECK (p_ini_file_is_key_exists (ini, "string_section", "string_paramter_1") == FALSE);
	BOOST_CHECK (p_ini_file_sections (ini) == NULL);
	BOOST_CHECK (p_ini_file_keys (ini, "string_section") == NULL);
	BOOST_CHECK (p_ini_file_parameter_boolean (ini, "boolean_section", "boolean_parameter_1", FALSE) == FALSE);
	BOOST_CHECK_CLOSE (p_ini_file_parameter_double (ini, "numeric_section", "float_parameter_1", 1.0), 1.0, 0.0001);
	BOOST_CHECK (p_ini_file_parameter_int (ini, "numeric_section", "int_parameter_1", 0) == 0);
	BOOST_CHECK (p_ini_file_parameter_list (ini, "list_section", "list_parameter_1") == NULL);
	BOOST_CHECK (p_ini_file_parameter_string (ini, "string_section", "string_parameter_1", NULL) == NULL);

	BOOST_REQUIRE (create_test_ini_file ());

	p_lib_shutdown ();
}

BOOST_AUTO_TEST_CASE (pinifile_read_test)
{
	p_lib_init ();

	PIniFile *ini = p_ini_file_new ("." P_DIR_SEPARATOR "p_ini_test_file.ini");
	BOOST_REQUIRE (ini != NULL);
	BOOST_CHECK (p_ini_file_is_parsed (ini) == FALSE);

	BOOST_REQUIRE (p_ini_file_parse (ini) == TRUE);
	BOOST_CHECK (p_ini_file_is_parsed (ini) == TRUE);

	/* Test list of sections */
	PList *list = p_ini_file_sections (ini);
	BOOST_CHECK (list != NULL);
	BOOST_CHECK (p_list_length (list) == 4);

	p_list_foreach (list, (PFunc) p_free, NULL);
	p_list_free (list);

	/* Test empty section */
	list = p_ini_file_keys (ini, "empty_section");
	BOOST_CHECK (list == NULL);

	/* Test numeric section */
	list = p_ini_file_keys (ini, "numeric_section");
	BOOST_CHECK (p_list_length (list) == 5);
	p_list_foreach (list, (PFunc) p_free, NULL);
	p_list_free (list);

	BOOST_CHECK (p_ini_file_parameter_int (ini, "numeric_section", "int_parameter_1", -1) == 4);
	BOOST_CHECK (p_ini_file_parameter_int (ini, "numeric_section", "int_parameter_2", -1) == 5);
	BOOST_CHECK (p_ini_file_parameter_int (ini, "numeric_section", "int_parameter_3", -1) == 6);
	BOOST_CHECK_CLOSE (p_ini_file_parameter_double (ini, "numeric_section", "float_parameter_1", -1.0), 3.24, 0.0001);
	BOOST_CHECK_CLOSE (p_ini_file_parameter_double (ini, "numeric_section", "float_parameter_2", -1.0), 0.15, 0.0001);
	BOOST_CHECK (p_ini_file_is_key_exists (ini, "numeric_section", "int_parameter_1") == TRUE);
	BOOST_CHECK (p_ini_file_is_key_exists (ini, "numeric_section", "float_parameter_1") == TRUE);

	/* Test string section */
	list = p_ini_file_keys (ini, "string_section");
	BOOST_CHECK (p_list_length (list) == 5);
	p_list_foreach (list, (PFunc) p_free, NULL);
	p_list_free (list);

	pchar *str = p_ini_file_parameter_string (ini, "string_section", "string_parameter_1", NULL);
	BOOST_REQUIRE (str != NULL);
	BOOST_CHECK (strcmp (str, "Test string") == 0);
	p_free (str);

	str = p_ini_file_parameter_string (ini, "string_section", "string_parameter_2", NULL);
	BOOST_REQUIRE (str != NULL);
	BOOST_CHECK (strcmp (str, "Test string with #'") == 0);
	p_free (str);

	str = p_ini_file_parameter_string (ini, "string_section", "string_parameter_3", NULL);
	BOOST_REQUIRE (str == NULL);
	BOOST_CHECK (p_ini_file_is_key_exists (ini, "string_section", "string_parameter_3") == FALSE);

	str = p_ini_file_parameter_string (ini, "string_section", "string_parameter_4", NULL);
	BOOST_REQUIRE (str != NULL);
	BOOST_CHECK (strcmp (str, "54321") == 0);
	p_free (str);

	str = p_ini_file_parameter_string (ini, "string_section", "string_parameter_5", NULL);
	BOOST_REQUIRE (str != NULL);
	BOOST_CHECK (strcmp (str, "Test string") == 0);
	p_free (str);

	/* Test boolean section */
	list = p_ini_file_keys (ini, "boolean_section");
	BOOST_CHECK (p_list_length (list) == 3);
	p_list_foreach (list, (PFunc) p_free, NULL);
	p_list_free (list);

	BOOST_CHECK (p_ini_file_parameter_boolean (ini, "boolean_section", "boolean_parameter_1", FALSE) == TRUE);
	BOOST_CHECK (p_ini_file_parameter_boolean (ini, "boolean_section", "boolean_parameter_2", TRUE) == FALSE);
	BOOST_CHECK (p_ini_file_parameter_boolean (ini, "boolean_section", "boolean_parameter_3", TRUE) == FALSE);

	/* Test list section */
	list = p_ini_file_keys (ini, "list_section");
	BOOST_CHECK (p_list_length (list) == 3);
	p_list_foreach (list, (PFunc) p_free, NULL);
	p_list_free (list);

	/* -- First list parameter */
	PList *list_val = p_ini_file_parameter_list (ini, "list_section", "list_parameter_1");
	BOOST_CHECK (list_val != NULL);
	BOOST_CHECK (p_list_length (list_val) == 4);

	pint int_sum = 0;
	for (PList *iter = list_val; iter != NULL; iter = iter->next)
		int_sum +=  atoi ((const pchar *) (iter->data));

	BOOST_CHECK (int_sum == 18);
	p_list_foreach (list_val, (PFunc) p_free, NULL);
	p_list_free (list_val);

	/* -- Second list parameter */
	list_val = p_ini_file_parameter_list (ini, "list_section", "list_parameter_2");
	BOOST_CHECK (list_val != NULL);
	BOOST_CHECK (p_list_length (list_val) == 3);

	double flt_sum = 0;
	for (PList *iter = list_val; iter != NULL; iter = iter->next)
		flt_sum +=  atof ((const pchar *) (iter->data));

	BOOST_CHECK_CLOSE (flt_sum, 10.0, 0.0001);
	p_list_foreach (list_val, (PFunc) p_free, NULL);
	p_list_free (list_val);

	/* -- Third list parameter */
	list_val = p_ini_file_parameter_list (ini, "list_section", "list_parameter_3");
	BOOST_CHECK (list_val != NULL);
	BOOST_CHECK (p_list_length (list_val) == 3);

	pboolean bool_sum = TRUE;
	for (PList *iter = list_val; iter != NULL; iter = iter->next)
		bool_sum = bool_sum && atoi ((const pchar *) (iter->data));

	BOOST_CHECK (bool_sum == FALSE);
	p_list_foreach (list_val, (PFunc) p_free, NULL);
	p_list_free (list_val);

	p_ini_file_free (ini);

	BOOST_CHECK (p_file_remove ("." P_DIR_SEPARATOR "p_ini_test_file.ini"));

	p_lib_shutdown ();
}

BOOST_AUTO_TEST_SUITE_END()
