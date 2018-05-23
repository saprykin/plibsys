/*
 * The MIT License
 *
 * Copyright (C) 2017 Jean-Damien Durand <jeandamiendurand@gmail.com>
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

#if defined(P_CC_WATCOM)
#  pragma disable_message (7)
#endif

P_TEST_MODULE_INIT ();

static void variadic_function (pint unused, ...);
static void variadic_function_copy_all (pint unused, p_va_list ap);
static void variadic_function_copy_trail (pint unused, p_va_list ap);

/* The "runtime" thingies here are just to avoid compiler warnings */
/*                                                 In head,          In trail */
static const pint8    pint8_var[2]    = {        P_MININT8,         P_MAXINT8 };
static const puint8   puint8_var[2]   = {                0,        P_MAXUINT8 };
static const pint16   pint16_var[2]   = {       P_MININT16,        P_MAXINT16 };
static const puint16  puint16_var[2]  = {                0,       P_MAXUINT16 };
static       pint32   pint32_var[2]   = {  0 /* runtime */,   0 /* runtime */ };
static       puint32  puint32_var[2]  = {                0,   0 /* runtime */ };
static       pint64   pint64_var[2]   = {  0 /* runtime */,   0 /* runtime */ };
static       puint64  puint64_var[2]  = {                0,   0 /* runtime */ };
static const pint     pint_var[2]     = {         P_MININT,          P_MAXINT };
static const puint    puint_var[2]    = {                0,         P_MAXUINT };
static const pshort   pshort_var[2]   = {       P_MINSHORT,        P_MAXSHORT };
static const plong    plong_var[2]    = {        P_MINLONG,         P_MAXLONG };
static const pchar    pchar_var[2]    = {             '\0',               'z' };
static const ppointer ppointer_var[2] = {             NULL, (ppointer) p_libsys_init };
static const pfloat   pfloat_var[2]   = {          -1.234f,            1.234f };
static const pdouble  pdouble_var[2]  = {           -1.567,             1.567 };

/* Macros are used for testing because these tests MUST play with the CURRENT stack. */

#define P_TEST_VA_ARG(ap, type, wantedvalue) do {		\
	P_DEBUG ("Unstacking a " #type);			\
	P_TEST_CHECK (type##_va_arg(ap) == wantedvalue);	\
  } while (0)

#define P_TEST_VA_ARG_HEAD(ap) do {				\
	P_TEST_VA_ARG(ap, pint8, pint8_var[0]);			\
	P_TEST_VA_ARG(ap, puint8, puint8_var[0]);		\
	P_TEST_VA_ARG(ap, pint16, pint16_var[0]);		\
	P_TEST_VA_ARG(ap, puint16, puint16_var[0]);		\
	P_TEST_VA_ARG(ap, pint32, pint32_var[0]);		\
	P_TEST_VA_ARG(ap, puint32, puint32_var[0]);		\
	P_TEST_VA_ARG(ap, pint64, pint64_var[0]);		\
	P_TEST_VA_ARG(ap, puint64, puint64_var[0]);		\
	P_TEST_VA_ARG(ap, pint, pint_var[0]);			\
	P_TEST_VA_ARG(ap, puint, puint_var[0]);			\
	P_TEST_VA_ARG(ap, pshort, pshort_var[0]);		\
	P_TEST_VA_ARG(ap, plong, plong_var[0]);			\
	P_TEST_VA_ARG(ap, pchar, pchar_var[0]);			\
	P_TEST_VA_ARG(ap, ppointer, ppointer_var[0]);		\
	P_TEST_VA_ARG(ap, pfloat, pfloat_var[0]);		\
	P_TEST_VA_ARG(ap, pdouble, pdouble_var[0]);		\
  } while (0)

#define P_TEST_VA_ARG_TRAIL(ap) do {				\
	P_TEST_VA_ARG(ap, pdouble, pdouble_var[1]);		\
	P_TEST_VA_ARG(ap, pfloat, pfloat_var[1]);		\
	P_TEST_VA_ARG(ap, ppointer, ppointer_var[1]);		\
	P_TEST_VA_ARG(ap, pchar, pchar_var[1]);			\
	P_TEST_VA_ARG(ap, plong, plong_var[1]);			\
	P_TEST_VA_ARG(ap, pshort, pshort_var[1]);		\
	P_TEST_VA_ARG(ap, puint, puint_var[1]);			\
	P_TEST_VA_ARG(ap, pint, pint_var[1]);			\
	P_TEST_VA_ARG(ap, puint64, puint64_var[1]);		\
	P_TEST_VA_ARG(ap, pint64, pint64_var[1]);		\
	P_TEST_VA_ARG(ap, puint32, puint32_var[1]);		\
	P_TEST_VA_ARG(ap, pint32, pint32_var[1]);		\
	P_TEST_VA_ARG(ap, puint16, puint16_var[1]);		\
	P_TEST_VA_ARG(ap, pint16, pint16_var[1]);		\
	P_TEST_VA_ARG(ap, puint8, puint8_var[1]);		\
	P_TEST_VA_ARG(ap, pint8, pint8_var[1]);			\
  } while (0)

static void variadic_function_copy_all (pint unused, p_va_list ap)
{
	P_UNUSED (unused);
	P_DEBUG ("Unstacking a copy of all the arguments");
	P_TEST_VA_ARG_HEAD (ap);
	P_TEST_VA_ARG_TRAIL (ap);
}

static void variadic_function_copy_trail (pint unused, p_va_list ap)
{
	P_UNUSED (unused);
	P_DEBUG ("Unstacking second part of the arguments");
	P_TEST_VA_ARG_TRAIL (ap);
}

static void variadic_function (pint unused, ...)
{
	p_va_list ap;
	p_va_list ap2;

	p_va_start (ap, unused);

	P_DEBUG ("Copy of arguments");
	p_va_copy (ap2, ap);
	variadic_function_copy_all (unused, ap2);
	p_va_end (ap2);

	P_DEBUG ("Unstacking first part of arguments");
	P_TEST_VA_ARG_HEAD (ap);

	P_DEBUG ("Copy of arguments at current unstack state, i.e. in the middle");
	p_va_copy (ap2, ap);
	variadic_function_copy_trail (unused, ap2);
	p_va_end (ap2);

	P_DEBUG ("Unstacking second part of arguments");
	P_TEST_VA_ARG_TRAIL (ap);

	p_va_end (ap);
}

P_TEST_CASE_BEGIN (pstdarg_general_test)
{
	p_libsys_init ();

	pint32_var[0] = P_MININT16;
	pint32_var[0] <<= 16;
	pint32_var[1] = P_MAXINT16;
	pint32_var[1] <<= 16;
	puint32_var[1] = P_MAXUINT16;
	puint32_var[1] <<= 16;

	pint64_var[0] = pint32_var[0];
	pint64_var[0] <<= 32;
	pint64_var[1] = pint32_var[1];
	pint64_var[1] <<= 32;
	puint64_var[1] = puint32_var[1];
	puint64_var[1] <<= 32;

	variadic_function (0,
			   pint8_var[0],
			   puint8_var[0],
			   pint16_var[0],
			   puint16_var[0],
			   pint32_var[0],
			   puint32_var[0],
			   pint64_var[0],
			   puint64_var[0],
			   pint_var[0],
			   puint_var[0],
			   pshort_var[0],
			   plong_var[0],
			   pchar_var[0],
			   ppointer_var[0],
			   pfloat_var[0],
			   pdouble_var[0],
			   /* Cut is here when testing p_va_copy, we stack in reverse order */
			   pdouble_var[1],
			   pfloat_var[1],
			   ppointer_var[1],
			   pchar_var[1],
			   plong_var[1],
			   pshort_var[1],
			   puint_var[1],
			   pint_var[1],
			   puint64_var[1],
			   pint64_var[1],
			   puint32_var[1],
			   pint32_var[1],
			   puint16_var[1],
			   pint16_var[1],
			   puint8_var[1],
			   pint8_var[1]);

	p_libsys_shutdown ();
}
P_TEST_CASE_END ()

P_TEST_SUITE_BEGIN()
{
	P_TEST_SUITE_RUN_CASE (pstdarg_general_test);
}
P_TEST_SUITE_END()
