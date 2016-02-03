/*
 * Copyright (C) 2016 Alexander Saprykin <xelfium@gmail.com>
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
#define BOOST_TEST_MODULE perror_test

#include "plib.h"

#include <string.h>

#include <boost/test/unit_test.hpp>

#define PERROR_TEST_MESSAGE	"PError test error message"
#define PERROR_TEST_MESSAGE_2	"Another PError test error message"

BOOST_AUTO_TEST_SUITE (BOOST_TEST_MODULE)

BOOST_AUTO_TEST_CASE (perror_invalid_test)
{
	p_lib_init ();

	BOOST_CHECK (p_error_get_message (NULL) == NULL);
	BOOST_CHECK (p_error_get_code (NULL) == 0);
	BOOST_CHECK (p_error_copy (NULL) == NULL);

	PError *error = (PError *) 0x1;

	p_error_set_error (NULL, 0, NULL);
	p_error_set_error_p (NULL, 0, NULL);

	p_error_set_error_p (&error, 0, NULL);
	BOOST_CHECK (error == (PError *) 0x1);

	p_error_clear (NULL);
	p_error_free (NULL);

	p_lib_shutdown ();
}

BOOST_AUTO_TEST_CASE (perror_general_test)
{
	p_lib_init ();

	/* Empty initialization test */
	PError *error = p_error_new ();

	BOOST_CHECK (error != NULL);
	BOOST_CHECK (p_error_get_code (error) == 0);
	BOOST_CHECK (p_error_get_message (error) == NULL);

	PError *copy_error = p_error_copy (error);

	BOOST_CHECK (copy_error != NULL);
	BOOST_CHECK (p_error_get_code (copy_error) == 0);
	BOOST_CHECK (p_error_get_message (copy_error) == NULL);

	p_error_free (copy_error);
	copy_error = NULL;

	p_error_set_error (error, 10, PERROR_TEST_MESSAGE);

	BOOST_CHECK (p_error_get_code (error) == 10);
	BOOST_CHECK (strcmp (p_error_get_message (error), PERROR_TEST_MESSAGE) == 0);

	copy_error = p_error_copy (error);

	BOOST_CHECK (copy_error != NULL);
	BOOST_CHECK (p_error_get_code (copy_error) == 10);
	BOOST_CHECK (strcmp (p_error_get_message (copy_error), PERROR_TEST_MESSAGE) == 0);

	p_error_free (copy_error);
	copy_error = NULL;

	p_error_set_error (error, 20, PERROR_TEST_MESSAGE_2);

	BOOST_CHECK (p_error_get_code (error) == 20);
	BOOST_CHECK (strcmp (p_error_get_message (error), PERROR_TEST_MESSAGE_2) == 0);

	p_error_clear (error);

	BOOST_CHECK (p_error_get_code (error) == 0);
	BOOST_CHECK (p_error_get_message (error) == NULL);

	p_error_free (error);
	error = NULL;

	/* Literal initialization test */
	error = p_error_new_literal (30, PERROR_TEST_MESSAGE);

	BOOST_CHECK (p_error_get_code (error) == 30);
	BOOST_CHECK (strcmp (p_error_get_message (error), PERROR_TEST_MESSAGE) == 0);

	copy_error = p_error_copy (error);

	BOOST_CHECK (copy_error != NULL);
	BOOST_CHECK (p_error_get_code (copy_error) == 30);
	BOOST_CHECK (strcmp (p_error_get_message (copy_error), PERROR_TEST_MESSAGE) == 0);

	p_error_free (copy_error);
	p_error_free (error);

	/* Through the double pointer */
	error = NULL;
	p_error_set_error_p (&error, 10, PERROR_TEST_MESSAGE);

	BOOST_CHECK (p_error_get_code (error) == 10);
	BOOST_CHECK (strcmp (p_error_get_message (error), PERROR_TEST_MESSAGE) == 0);

	p_error_free (error);

	p_lib_shutdown ();
}

BOOST_AUTO_TEST_SUITE_END()
