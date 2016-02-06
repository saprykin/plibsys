/*
 * Copyright (C) 2015-2016 Alexander Saprykin <xelfium@gmail.com>
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
#define BOOST_TEST_MODULE plibraryloader_test

#include "plib.h"

#ifdef PLIB_TESTS_STATIC
#  include <boost/test/included/unit_test.hpp>
#else
#  include <boost/test/unit_test.hpp>
#endif

BOOST_AUTO_TEST_SUITE (BOOST_TEST_MODULE)

BOOST_AUTO_TEST_CASE (plibraryloader_general_test)
{
	PLibraryLoader	*loader;
	pchar		*err_msg;
	void		(*shutdown_func) (void);

	p_lib_init ();

	/* We assume that 2nd argument is a PLib library path */
	BOOST_REQUIRE (boost::unit_test::framework::master_test_suite().argc == 2);

	/* Invalid usage */
	BOOST_CHECK (p_library_loader_new (NULL) == NULL);
	BOOST_CHECK (p_library_loader_new ("./unexistent_file.nofile") == NULL);
	BOOST_CHECK (p_library_loader_get_symbol (NULL, NULL) == NULL);
	BOOST_CHECK (p_library_loader_get_symbol (NULL, "unexistent_symbol") == NULL);
	p_library_loader_free (NULL);

	/* General tests */
	err_msg = p_library_loader_get_last_error ();
	p_free (err_msg);

	loader = p_library_loader_new (boost::unit_test::framework::master_test_suite().argv[1]);
	BOOST_REQUIRE (loader != NULL);

	shutdown_func = (void (*) (void)) p_library_loader_get_symbol (loader, "p_lib_shutdown");
	BOOST_REQUIRE (shutdown_func != NULL);

	err_msg = p_library_loader_get_last_error ();
	p_free (err_msg);

	p_library_loader_free (loader);

	/* We have already loaded reference to PLib, it's OK */
	shutdown_func ();
}

BOOST_AUTO_TEST_SUITE_END()
