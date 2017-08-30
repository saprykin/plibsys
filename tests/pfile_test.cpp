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

#include <stdio.h>

P_TEST_MODULE_INIT ();

#define PFILE_TEST_FILE "." P_DIR_SEPARATOR "pfile_test_file.txt"

P_TEST_CASE_BEGIN (pfile_general_test)
{
	p_libsys_init ();

	P_TEST_CHECK (p_file_remove (NULL, NULL) == FALSE);

	P_TEST_CHECK (p_file_is_exists (PFILE_TEST_FILE) == FALSE);
	P_TEST_CHECK (p_file_remove ("." P_DIR_SEPARATOR" pfile_test_file_remove.txt", NULL) == FALSE);

	FILE *file = fopen (PFILE_TEST_FILE, "w");
	P_TEST_REQUIRE (file != NULL);
	P_TEST_CHECK (p_file_is_exists (PFILE_TEST_FILE) == TRUE);

	fprintf (file, "This is a test file string\n");

	P_TEST_CHECK (fclose (file) == 0);
	P_TEST_CHECK (p_file_remove (PFILE_TEST_FILE, NULL) == TRUE);

	p_libsys_shutdown ();
}
P_TEST_CASE_END ()

P_TEST_SUITE_BEGIN()
{
	P_TEST_SUITE_RUN_CASE (pfile_general_test);
}
P_TEST_SUITE_END()
