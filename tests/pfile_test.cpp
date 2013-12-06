#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE pfile_test

#include "plib.h"

#include <stdio.h>

#include <boost/test/unit_test.hpp>

#define PFILE_TEST_FILE "."P_DIR_SEPARATOR"pfile_test_file.txt"

BOOST_AUTO_TEST_SUITE (BOOST_TEST_MODULE)

BOOST_AUTO_TEST_CASE (pfile_general_test)
{
	p_lib_init ();

	BOOST_CHECK (p_file_is_exists (PFILE_TEST_FILE) == FALSE);
	BOOST_CHECK (p_file_remove ("."P_DIR_SEPARATOR"pfile_test_file_remove.txt") == FALSE);

	FILE *file = fopen (PFILE_TEST_FILE, "w");
	BOOST_REQUIRE (file != NULL);
	BOOST_CHECK (p_file_is_exists (PFILE_TEST_FILE) == TRUE);

	fprintf (file, "This is a test file string\n");

	BOOST_CHECK (fclose (file) == 0);
	BOOST_CHECK (p_file_remove (PFILE_TEST_FILE) == TRUE);

	p_lib_shutdown ();
}

BOOST_AUTO_TEST_SUITE_END()
