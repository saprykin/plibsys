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

/* Actually we couldn't test the work of the atomic operations across the
 * threads, but at least we can test the sanity of operations */

P_TEST_MODULE_INIT ();

P_TEST_CASE_BEGIN (patomic_general_test)
{
	p_libsys_init ();

	(void) p_atomic_is_lock_free ();

	pint atomic_int = 0;
	p_atomic_int_set (&atomic_int, 10);

	P_TEST_CHECK (p_atomic_int_add (&atomic_int, 5) == 10);
	P_TEST_CHECK (p_atomic_int_get (&atomic_int) == 15);

	p_atomic_int_add (&atomic_int, -5);
	P_TEST_CHECK (p_atomic_int_get (&atomic_int) == 10);

	p_atomic_int_inc (&atomic_int);
	P_TEST_CHECK (p_atomic_int_get (&atomic_int) == 11);

	P_TEST_CHECK (p_atomic_int_dec_and_test (&atomic_int) == FALSE);
	P_TEST_CHECK (p_atomic_int_get (&atomic_int) == 10);

	P_TEST_CHECK (p_atomic_int_compare_and_exchange (&atomic_int, 10, -10) == TRUE);
	P_TEST_CHECK (p_atomic_int_get (&atomic_int) == -10);
	P_TEST_CHECK (p_atomic_int_compare_and_exchange (&atomic_int, 10, 20) == FALSE);
	P_TEST_CHECK (p_atomic_int_get (&atomic_int) == -10);

	p_atomic_int_inc (&atomic_int);
	P_TEST_CHECK (p_atomic_int_get (&atomic_int) == -9);

	p_atomic_int_set (&atomic_int, 4);
	P_TEST_CHECK (p_atomic_int_get (&atomic_int) == 4);

	P_TEST_CHECK (p_atomic_int_xor ((puint *) &atomic_int, (puint) 1) == 4);
	P_TEST_CHECK (p_atomic_int_get (&atomic_int) == 5);

	P_TEST_CHECK (p_atomic_int_or ((puint *) &atomic_int, (puint) 2) == 5);
	P_TEST_CHECK (p_atomic_int_get (&atomic_int) == 7);

	P_TEST_CHECK (p_atomic_int_and ((puint *) &atomic_int, (puint) 1) == 7);
	P_TEST_CHECK (p_atomic_int_get (&atomic_int) == 1);

	p_atomic_int_set (&atomic_int, 51);
	P_TEST_CHECK (p_atomic_int_get (&atomic_int) == 51);

	for (pint i = 51; i > 1; --i) {
		P_TEST_CHECK (p_atomic_int_dec_and_test (&atomic_int) == FALSE);
		P_TEST_CHECK (p_atomic_int_get (&atomic_int) == (i - 1));
	}

	P_TEST_CHECK (p_atomic_int_dec_and_test (&atomic_int) == TRUE);
	P_TEST_CHECK (p_atomic_int_get (&atomic_int) == 0);

	ppointer atomic_pointer = NULL;
	p_atomic_pointer_set (&atomic_pointer, PUINT_TO_POINTER (P_MAXSIZE));
	P_TEST_CHECK (p_atomic_pointer_get (&atomic_pointer) == PUINT_TO_POINTER (P_MAXSIZE));

	p_atomic_pointer_set (&atomic_pointer, PUINT_TO_POINTER (100));
	P_TEST_CHECK (p_atomic_pointer_get (&atomic_pointer) == PUINT_TO_POINTER (100));
	P_TEST_CHECK (p_atomic_pointer_add (&atomic_pointer, (pssize) 100) == 100);
	P_TEST_CHECK (p_atomic_pointer_get (&atomic_pointer) == PUINT_TO_POINTER (200));

	p_atomic_pointer_set (&atomic_pointer, PINT_TO_POINTER (4));
	P_TEST_CHECK (p_atomic_pointer_get (&atomic_pointer) == PINT_TO_POINTER (4));

	P_TEST_CHECK (p_atomic_pointer_xor (&atomic_pointer, (psize) 1) == 4);
	P_TEST_CHECK (p_atomic_pointer_get (&atomic_pointer) == PINT_TO_POINTER (5));

	P_TEST_CHECK (p_atomic_pointer_or (&atomic_pointer, (psize) 2) == 5);
	P_TEST_CHECK (p_atomic_pointer_get (&atomic_pointer) == PINT_TO_POINTER (7));

	P_TEST_CHECK (p_atomic_pointer_and (&atomic_pointer, (psize) 1) == 7);
	P_TEST_CHECK (p_atomic_pointer_get (&atomic_pointer) == PINT_TO_POINTER (1));

	P_TEST_CHECK (p_atomic_pointer_compare_and_exchange (&atomic_pointer, PUINT_TO_POINTER (1), NULL) == TRUE);
	P_TEST_CHECK (p_atomic_pointer_get (&atomic_pointer) == NULL);

	p_libsys_shutdown ();
}
P_TEST_CASE_END ()

P_TEST_SUITE_BEGIN()
{
	P_TEST_SUITE_RUN_CASE (patomic_general_test);
}
P_TEST_SUITE_END()
