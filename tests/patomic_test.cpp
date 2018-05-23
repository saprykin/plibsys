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
