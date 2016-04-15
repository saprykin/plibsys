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

#define BOOST_TEST_MODULE patomic_test

#include "plib.h"

#ifdef PLIB_TESTS_STATIC
#  include <boost/test/included/unit_test.hpp>
#else
#  include <boost/test/unit_test.hpp>
#endif

/* Actually we couldn't test the work of the atomic operations across the
 * threads, but at least we can test the sanity of operations */

BOOST_AUTO_TEST_SUITE (BOOST_TEST_MODULE)

BOOST_AUTO_TEST_CASE (patomic_general_test)
{
	p_lib_init ();

	pint atomic_int = 0;
	p_atomic_int_set (&atomic_int, 10);

	BOOST_CHECK (p_atomic_int_add (&atomic_int, 5) == 10);
	BOOST_CHECK (p_atomic_int_get (&atomic_int) == 15);

	p_atomic_int_add (&atomic_int, -5);
	BOOST_CHECK (p_atomic_int_get (&atomic_int) == 10);

	p_atomic_int_inc (&atomic_int);
	BOOST_CHECK (p_atomic_int_get (&atomic_int) == 11);

	BOOST_CHECK (p_atomic_int_dec_and_test (&atomic_int) == FALSE);
	BOOST_CHECK (p_atomic_int_get (&atomic_int) == 10);

	BOOST_CHECK (p_atomic_int_compare_and_exchange (&atomic_int, 10, -10) == TRUE);
	BOOST_CHECK (p_atomic_int_get (&atomic_int) == -10);
	BOOST_CHECK (p_atomic_int_compare_and_exchange (&atomic_int, 10, 20) == FALSE);
	BOOST_CHECK (p_atomic_int_get (&atomic_int) == -10);

	p_atomic_int_inc (&atomic_int);
	BOOST_CHECK (p_atomic_int_get (&atomic_int) == -9);

	p_atomic_int_set (&atomic_int, 4);
	BOOST_CHECK (p_atomic_int_get (&atomic_int) == 4);

	BOOST_CHECK (p_atomic_int_xor (&atomic_int, (puint) 1) == 4);
	BOOST_CHECK (p_atomic_int_get (&atomic_int) == 5);

	BOOST_CHECK (p_atomic_int_or (&atomic_int, (puint) 2) == 5);
	BOOST_CHECK (p_atomic_int_get (&atomic_int) == 7);

	BOOST_CHECK (p_atomic_int_and (&atomic_int, (puint) 1) == 7);
	BOOST_CHECK (p_atomic_int_get (&atomic_int) == 1);

	p_atomic_int_set (&atomic_int, 51);
	BOOST_CHECK (p_atomic_int_get (&atomic_int) == 51);

	for (pint i = 51; i > 1; --i) {
		BOOST_CHECK (p_atomic_int_dec_and_test (&atomic_int) == FALSE);
		BOOST_CHECK (p_atomic_int_get (&atomic_int) == (i - 1));
	}

	BOOST_CHECK (p_atomic_int_dec_and_test (&atomic_int) == TRUE);
	BOOST_CHECK (p_atomic_int_get (&atomic_int) == 0);

	ppointer atomic_pointer = NULL;
	p_atomic_pointer_set (&atomic_pointer, PUINT_TO_POINTER (P_MAXSIZE));
	BOOST_CHECK (p_atomic_pointer_get (&atomic_pointer) == PUINT_TO_POINTER (P_MAXSIZE));

	p_atomic_pointer_set (&atomic_pointer, PUINT_TO_POINTER (100));
	BOOST_CHECK (p_atomic_pointer_get (&atomic_pointer) == PUINT_TO_POINTER (100));
	BOOST_CHECK (p_atomic_pointer_add (&atomic_pointer, (pssize) 100) == 100);
	BOOST_CHECK (p_atomic_pointer_get (&atomic_pointer) == PUINT_TO_POINTER (200));

	p_atomic_pointer_set (&atomic_pointer, PINT_TO_POINTER (4));
	BOOST_CHECK (p_atomic_pointer_get (&atomic_pointer) == PINT_TO_POINTER (4));

	BOOST_CHECK (p_atomic_pointer_xor (&atomic_pointer, (psize) 1) == 4);
	BOOST_CHECK (p_atomic_pointer_get (&atomic_pointer) == PINT_TO_POINTER (5));

	BOOST_CHECK (p_atomic_pointer_or (&atomic_pointer, (psize) 2) == 5);
	BOOST_CHECK (p_atomic_pointer_get (&atomic_pointer) == PINT_TO_POINTER (7));

	BOOST_CHECK (p_atomic_pointer_and (&atomic_pointer, (psize) 1) == 7);
	BOOST_CHECK (p_atomic_pointer_get (&atomic_pointer) == PINT_TO_POINTER (1));

	BOOST_CHECK (p_atomic_pointer_compare_and_exchange (&atomic_pointer, PUINT_TO_POINTER (1), NULL) == TRUE);
	BOOST_CHECK (p_atomic_pointer_get (&atomic_pointer) == NULL);

	p_lib_shutdown ();
}

BOOST_AUTO_TEST_SUITE_END()
