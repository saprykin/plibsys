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
