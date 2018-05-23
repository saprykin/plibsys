/*
 * The MIT License
 *
 * Copyright (C) 2017 Alexander Saprykin <saprykin.spb@gmail.com>
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

/**
 * @file ptestmacros.h
 * @brief Macros for unit-tests
 * @author Alexander Saprykin
 */

#ifndef PLIBSYS_HEADER_PTESTMACROS_H
#define PLIBSYS_HEADER_PTESTMACROS_H

#include <stdlib.h>

#include <plibsys.h>

#if defined (P_CC_MSVC)
#  pragma warning (push)
#  pragma warning (disable : 4723)
#elif defined (P_CC_BORLAND)
#  pragma option -w-8008
#  pragma option -w-8066
#elif defined (P_CC_WATCOM)
#  pragma disable_message (13)
#  pragma disable_message (367)
#  pragma disable_message (368)
#endif

inline double p_test_safe_division (double f1, double f2)
{
	return (f2 < 1.0 && f1 > f2 * P_MAXDOUBLE) ? P_MAXDOUBLE :
	        (((f2 > 1.0 && f1 < f2 * P_MINDOUBLE) || f1 == 0) ? 0 : f1 / f2);
}

#ifdef P_CC_MSVC
#  pragma warning (pop)
#endif

#define P_TEST_MODULE_FAIL_COUNTER p_test_module_fail_counter
#define P_TEST_SUITE_FAIL_COUNTER p_test_suite_fail_counter

#define P_TEST_MODULE_INIT() static pint P_TEST_MODULE_FAIL_COUNTER = 0

#define P_TEST_CASE_BEGIN(test_case_name)						\
	pint p_test_case_##test_case_name (void)					\
	{										\
		P_TEST_MODULE_FAIL_COUNTER = 0;

#define P_TEST_CASE_END()								\
		return (P_TEST_MODULE_FAIL_COUNTER == 0) ? 0 : -1;			\
	}

#define P_TEST_CASE_RETURN() return 0

#define P_TEST_SUITE_BEGIN()								\
	int main (void)									\
	{										\
		pint P_TEST_SUITE_FAIL_COUNTER = 0;

#define P_TEST_SUITE_ARGS_BEGIN()							\
	int main (int argc, char *argv[])						\
	{										\
		pint P_TEST_SUITE_FAIL_COUNTER = 0;

#define P_TEST_SUITE_END()								\
		if (P_TEST_SUITE_FAIL_COUNTER == 0)					\
			printf ("Test passed\n");					\
		else									\
			printf ("Test failed\n");					\
											\
		return P_TEST_SUITE_FAIL_COUNTER == 0 ? 0 : -1;				\
	}

#define P_TEST_SUITE_RUN_CASE(a) 							\
	printf ("Running test case: %s\n", #a);						\
	P_TEST_SUITE_FAIL_COUNTER += (p_test_case_##a)()

#define P_TEST_CHECK(a)									\
	do {										\
		if (!(a)) {								\
			printf ("%s:%d: check failed\n", __FILE__, __LINE__);		\
			p_atomic_int_inc (&P_TEST_MODULE_FAIL_COUNTER);			\
		}									\
	} while (0)

#define P_TEST_CHECK_CLOSE(a, b, eps)							\
	do {										\
		double p_test_eps_diff = (a) > (b) ? (a) - (b) : (b) - (a);		\
		double p_test_d1 = p_test_safe_division (p_test_eps_diff,		\
							 ((a) < 0.0 ? (-(a)) : (a)));	\
		double p_test_d2 = p_test_safe_division (p_test_eps_diff,		\
							 ((b) < 0.0 ? (-(b)) : (b)));	\
		double p_test_tol = (eps) * 0.01;					\
											\
		if (!(p_test_d1 <= p_test_tol && p_test_d2 <= p_test_tol)) {		\
			printf ("%s:%d: check failed\n", __FILE__, __LINE__);		\
			p_atomic_int_inc (&P_TEST_MODULE_FAIL_COUNTER);			\
		}									\
	} while (0)

#define P_TEST_REQUIRE(a)								\
	do {										\
		if (!(a)) {								\
			printf ("%s:%d: required check failed\n", __FILE__, __LINE__);	\
			exit (-1);							\
		}									\
	} while (0)

#endif /* PLIBSYS_HEADER_PTESTMACROS_H */
