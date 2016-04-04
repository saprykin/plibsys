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

#define BOOST_TEST_MODULE pfile_test

#include "plib.h"

#include <stdio.h>

#ifdef PLIB_TESTS_STATIC
#  include <boost/test/included/unit_test.hpp>
#else
#  include <boost/test/unit_test.hpp>
#endif

#define PFILE_TEST_FILE "."P_DIR_SEPARATOR"pfile_test_file.txt"

BOOST_AUTO_TEST_SUITE (BOOST_TEST_MODULE)

BOOST_AUTO_TEST_CASE (pfile_general_test)
{
	p_lib_init ();

	BOOST_CHECK (p_file_remove (NULL, NULL) == FALSE);

	BOOST_CHECK (p_file_is_exists (PFILE_TEST_FILE) == FALSE);
	BOOST_CHECK (p_file_remove ("."P_DIR_SEPARATOR"pfile_test_file_remove.txt", NULL) == FALSE);

	FILE *file = fopen (PFILE_TEST_FILE, "w");
	BOOST_REQUIRE (file != NULL);
	BOOST_CHECK (p_file_is_exists (PFILE_TEST_FILE) == TRUE);

	fprintf (file, "This is a test file string\n");

	BOOST_CHECK (fclose (file) == 0);
	BOOST_CHECK (p_file_remove (PFILE_TEST_FILE, NULL) == TRUE);

	p_lib_shutdown ();
}

BOOST_AUTO_TEST_SUITE_END()
