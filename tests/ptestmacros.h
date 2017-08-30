/*
 * Copyright (C) 2017 Alexander Saprykin <xelfium@gmail.com>
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

/**
 * @file ptestmacros.h
 * @brief Macros for unit-tests
 * @author Alexander Saprykin
 */

#ifndef PLIBSYS_HEADER_PTESTMACROS_H
#define PLIBSYS_HEADER_PTESTMACROS_H

#include <stdlib.h>

#include <plibsys.h>

inline double p_test_safe_division (double f1, double f2)
{
	return (f2 < 1.0 && f1 > f2 * P_MAXDOUBLE) ? P_MAXDOUBLE :
	        (((f2 > 1.0 && f1 < f2 * P_MINDOUBLE) || f1 == 0) ? 0 : f1 / f2);
}

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

#define P_TEST_SUITE_PARAMS_BEGIN()							\
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
