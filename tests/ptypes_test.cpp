/*
 * Copyright (C) 2014-2016 Alexander Saprykin <xelfium@gmail.com>
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
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

#ifndef PLIBSYS_TESTS_STATIC
#  define BOOST_TEST_DYN_LINK
#endif

#define BOOST_TEST_MODULE ptypes_test

#include "plibsys.h"

#ifdef PLIBSYS_TESTS_STATIC
#  include <boost/test/included/unit_test.hpp>
#else
#  include <boost/test/unit_test.hpp>
#endif

BOOST_AUTO_TEST_SUITE (BOOST_TEST_MODULE)

BOOST_AUTO_TEST_CASE (ptypes_general_test)
{
	p_libsys_init ();

	BOOST_CHECK (P_BYTE_ORDER == P_LITTLE_ENDIAN ||
		     P_BYTE_ORDER == P_BIG_ENDIAN);

	BOOST_CHECK (sizeof (pint8) == 1);
	BOOST_CHECK (sizeof (puint8) == 1);
	BOOST_CHECK (sizeof (pint16) == 2);
	BOOST_CHECK (sizeof (puint16) == 2);
	BOOST_CHECK (sizeof (pint32) == 4);
	BOOST_CHECK (sizeof (puint32) == 4);
	BOOST_CHECK (sizeof (pint64) == 8);
	BOOST_CHECK (sizeof (puint64) == 8);
	BOOST_CHECK (sizeof (void *) == sizeof (ppointer));
	BOOST_CHECK (sizeof (const void *) == sizeof (pconstpointer));
	BOOST_CHECK (sizeof (int) == sizeof (pboolean));
	BOOST_CHECK (sizeof (char) == sizeof (pchar));
	BOOST_CHECK (sizeof (short) == sizeof (pshort));
	BOOST_CHECK (sizeof (int) == sizeof (pint));
	BOOST_CHECK (sizeof (long) == sizeof (plong));
	BOOST_CHECK (sizeof (unsigned char) == sizeof (puchar));
	BOOST_CHECK (sizeof (unsigned short) == sizeof (pushort));
	BOOST_CHECK (sizeof (unsigned int) == sizeof (puint));
	BOOST_CHECK (sizeof (unsigned long) == sizeof (pulong));
	BOOST_CHECK (sizeof (float) == sizeof (pfloat));
	BOOST_CHECK (sizeof (double) == sizeof (pdouble));
	BOOST_CHECK (sizeof (pintptr) == PLIBSYS_SIZEOF_VOID_P);
	BOOST_CHECK (sizeof (puintptr) == PLIBSYS_SIZEOF_VOID_P);
	BOOST_CHECK (sizeof (psize) == PLIBSYS_SIZEOF_SIZE_T);
	BOOST_CHECK (sizeof (pssize) == PLIBSYS_SIZEOF_SIZE_T);
	BOOST_CHECK (sizeof (plong) == PLIBSYS_SIZEOF_LONG);
	BOOST_CHECK (sizeof (pulong) == PLIBSYS_SIZEOF_LONG);
	BOOST_CHECK (sizeof (poffset) == 8);

	p_libsys_shutdown ();
}

BOOST_AUTO_TEST_CASE (ptypes_pointers_convert_test)
{
	p_libsys_init ();

	ppointer pointer = P_INT_TO_POINTER (128);
	BOOST_CHECK (P_POINTER_TO_INT (pointer) == 128);

	pint pint_val = -64;
	pointer = PINT_TO_POINTER (pint_val);
	BOOST_CHECK (PPOINTER_TO_INT (pointer) == -64);

	puint puint_val = 64;
	pointer = PUINT_TO_POINTER (puint_val);
	BOOST_CHECK (PPOINTER_TO_UINT (pointer) == 64);

	psize psize_val = 1024;
	pointer = PSIZE_TO_POINTER (psize_val);
	BOOST_CHECK (PPOINTER_TO_PSIZE (psize_val) == 1024);

	p_libsys_shutdown ();
}

BOOST_AUTO_TEST_CASE (ptypes_min_max_test)
{
	p_libsys_init ();

	BOOST_CHECK (P_MININT8 == (pint8) 0x80);
	BOOST_CHECK (P_MAXINT8 == (pint8) 0x7F);
	BOOST_CHECK (P_MAXUINT8 == (puint8) 0xFF);
	BOOST_CHECK (P_MININT16 == (pint16) 0x8000);
	BOOST_CHECK (P_MAXINT16 == (pint16) 0x7FFF);
	BOOST_CHECK (P_MAXUINT16 == (puint16) 0xFFFF);
	BOOST_CHECK (P_MININT32 == (pint32) 0x80000000);
	BOOST_CHECK (P_MAXINT32 == (pint32) 0x7FFFFFFF);
	BOOST_CHECK (P_MAXUINT32 == (puint32) 0xFFFFFFFF);
	BOOST_CHECK (P_MININT64 == (pint64) 0x8000000000000000LL);
	BOOST_CHECK (P_MAXINT64 == (pint64) 0x7FFFFFFFFFFFFFFFLL);
	BOOST_CHECK (P_MAXUINT64 == (puint64) 0xFFFFFFFFFFFFFFFFULL);

	if (PLIBSYS_SIZEOF_SIZE_T == 8) {
		BOOST_CHECK (P_MINSSIZE == P_MININT64);
		BOOST_CHECK (P_MAXSSIZE == P_MAXINT64);
		BOOST_CHECK (P_MAXSIZE == P_MAXUINT64);

		if (PLIBSYS_SIZEOF_LONG == 8) {
			BOOST_CHECK (P_MINSSIZE == P_MINLONG);
			BOOST_CHECK (P_MAXSSIZE == P_MAXLONG);
			BOOST_CHECK (P_MAXSIZE == P_MAXULONG);
		}
	} else {
		BOOST_CHECK (P_MINSSIZE == P_MININT32);
		BOOST_CHECK (P_MAXSSIZE == P_MAXINT32);
		BOOST_CHECK (P_MAXSIZE == P_MAXUINT32);

		if (PLIBSYS_SIZEOF_LONG == 4) {
			BOOST_CHECK (P_MINSSIZE == P_MINLONG);
			BOOST_CHECK (P_MAXSSIZE == P_MAXLONG);
			BOOST_CHECK (P_MAXSIZE == P_MAXULONG);
		}
	}

	p_libsys_shutdown ();
}

BOOST_AUTO_TEST_CASE (ptypes_modifiers_test)
{
	p_libsys_init ();

	psize size_val = 256;
	printf ("%#" PSIZE_MODIFIER "x\n", size_val);
	pssize ssize_val = -256;
	printf ("%#" PSIZE_MODIFIER "x\n", ssize_val);

	puintptr puintptr_val = 512;
	printf ("%#" PINTPTR_MODIFIER "x\n", puintptr_val);
	pintptr pintptr_val = -512;
	printf ("%#" PINTPTR_MODIFIER "x\n", pintptr_val);

	puint16 puint16_val = 1024;
	printf ("%#" PINT16_MODIFIER "x\n", puint16_val);
	pint16 pint16_val = -1024;
	printf ("%#" PINT16_MODIFIER "x\n", pint16_val);

	puint32 puint32_val = 2048;
	printf ("%#" PINT32_MODIFIER "x\n", puint32_val);
	pint32 pint32_val = -2048;
	printf ("%#" PINT32_MODIFIER "x\n", pint32_val);

	puint64 puint64_val = 4096;
	printf ("%#" PINT64_MODIFIER "x\n", puint64_val);
	pint64 pint64_val = -4096;
	printf ("%#" PINT64_MODIFIER "x\n", pint64_val);

	poffset poffset_val = 8192;
	printf ("%#" POFFSET_MODIFIER "x\n", poffset_val);

	p_libsys_shutdown ();
}

BOOST_AUTO_TEST_CASE (ptypes_formats_test)
{
	p_libsys_init ();

	pssize ssize_val = -256;
	printf ("%" PSSIZE_FORMAT "\n", ssize_val);
	psize size_val = 256;
	printf ("%" PSIZE_FORMAT "\n", size_val);

	puintptr puintptr_val = 512;
	printf ("%" PUINTPTR_FORMAT "\n", puintptr_val);
	pintptr pintptr_val = -512;
	printf ("%" PINTPTR_FORMAT "\n", pintptr_val);

	puint16 puint16_val = 1024;
	printf ("%" PUINT16_FORMAT "\n", puint16_val);
	pint16 pint16_val = -1024;
	printf ("%" PINT16_FORMAT "\n", pint16_val);

	puint32 puint32_val = 2048;
	printf ("%" PUINT32_FORMAT "\n", puint32_val);
	pint32 pint32_val = -2048;
	printf ("%" PINT32_FORMAT "\n", pint32_val);

	puint64 puint64_val = 4096;
	printf ("%" PUINT64_FORMAT "\n", puint64_val);
	pint64 pint64_val = -4096;
	printf ("%" PINT64_FORMAT "\n", pint64_val);

	poffset poffset_val = 8192;
	printf ("%" POFFSET_FORMAT "\n", poffset_val);

	p_libsys_shutdown ();
}

BOOST_AUTO_TEST_CASE (ptypes_host_network_test)
{
	p_libsys_init ();

	if (P_BYTE_ORDER == P_LITTLE_ENDIAN) {
		pint16 pint16_val = PINT16_TO_BE (0xFFE0);
		BOOST_CHECK (pint16_val == (pint16) 0xE0FF);
		BOOST_CHECK (PINT16_FROM_BE (pint16_val) == (pint16) 0xFFE0);
		BOOST_CHECK (PINT16_TO_LE (pint16_val) == (pint16) 0xE0FF);
		BOOST_CHECK (PINT16_FROM_LE (pint16_val) == (pint16) 0xE0FF);

		puint16 puint16_val = PUINT16_TO_BE (0x0020);
		BOOST_CHECK (puint16_val == (puint16) 0x2000);
		BOOST_CHECK (PUINT16_FROM_BE (puint16_val) == (puint16) 0x0020);
		BOOST_CHECK (PUINT16_TO_LE (puint16_val) == (puint16) 0x2000);
		BOOST_CHECK (PUINT16_FROM_LE (puint16_val) == (puint16) 0x2000);

		pint32 pint32_val = PINT32_TO_BE (0xFFFFFFC0);
		BOOST_CHECK (pint32_val == (pint32) 0xC0FFFFFF);
		BOOST_CHECK (PINT32_FROM_BE (pint32_val) == (pint32) 0xFFFFFFC0);
		BOOST_CHECK (PINT32_TO_LE (pint32_val) == (pint32) 0xC0FFFFFF);
		BOOST_CHECK (PINT32_FROM_LE (pint32_val) == (pint32) 0xC0FFFFFF);

		puint32 puint32_val = PUINT32_TO_BE (0x00000040);
		BOOST_CHECK (puint32_val == (puint32) 0x40000000);
		BOOST_CHECK (PUINT32_FROM_BE (puint32_val) == (puint32) 0x00000040);
		BOOST_CHECK (PUINT32_TO_LE (puint32_val) == (puint32) 0x40000000);
		BOOST_CHECK (PUINT32_FROM_LE (puint32_val) == (puint32) 0x40000000);

		pint64 pint64_val = PINT64_TO_BE (0xFFFFFFFFFFFFFF80LL);
		BOOST_CHECK (pint64_val == (pint64) 0x80FFFFFFFFFFFFFFLL);
		BOOST_CHECK (PINT64_FROM_BE (pint64_val) == (pint64) 0xFFFFFFFFFFFFFF80LL);
		BOOST_CHECK (PINT64_TO_LE (pint64_val) == (pint64) 0x80FFFFFFFFFFFFFFLL);
		BOOST_CHECK (PINT64_FROM_LE (pint64_val) == (pint64) 0x80FFFFFFFFFFFFFFLL);

		puint64 puint64_val = PUINT64_TO_BE (0x0000000000000080ULL);
		BOOST_CHECK (puint64_val == (puint64) 0x8000000000000000ULL);
		BOOST_CHECK (PUINT64_FROM_BE (puint64_val) == (puint64) 0x0000000000000080ULL);
		BOOST_CHECK (PUINT64_TO_LE (puint64_val) == (puint64) 0x8000000000000000ULL);
		BOOST_CHECK (PUINT64_FROM_LE (puint64_val) == (puint64) 0x8000000000000000ULL);

		pint pint_val = PINT_TO_BE (0xFFFFFC00);
		BOOST_CHECK (pint_val == (pint) 0x00FCFFFF);
		BOOST_CHECK (PINT_FROM_BE (pint_val) == (pint) 0xFFFFFC00);
		BOOST_CHECK (PINT_TO_LE (pint_val) == (pint) 0x00FCFFFF);
		BOOST_CHECK (PINT_FROM_LE (pint_val) == (pint) 0x00FCFFFF);

		puint puint_val = PUINT_TO_BE (0x00000400);
		BOOST_CHECK (puint_val == (puint) 0x00040000);
		BOOST_CHECK (PUINT_FROM_BE (puint_val) == (puint) 0x00000400);
		BOOST_CHECK (PUINT_TO_LE (puint_val) == (puint) 0x00040000);
		BOOST_CHECK (PUINT_FROM_LE (puint_val) == (puint) 0x00040000);

		if (PLIBSYS_SIZEOF_LONG == 8) {
			plong plong_val = PLONG_TO_BE (0xFFFFFFFFFFFFF800LL);
			BOOST_CHECK (plong_val == (plong) 0x00F8FFFFFFFFFFFFLL);
			BOOST_CHECK (PLONG_FROM_BE (plong_val) == (plong) 0xFFFFFFFFFFFFF800LL);
			BOOST_CHECK (PLONG_TO_LE (plong_val) == (plong) 0x00F8FFFFFFFFFFFFLL);
			BOOST_CHECK (PLONG_FROM_LE (plong_val) == (plong) 0x00F8FFFFFFFFFFFFLL);

			pulong pulong_val = PULONG_TO_BE (0x0000000000000800ULL);
			BOOST_CHECK (pulong_val == (pulong) 0x0008000000000000ULL);
			BOOST_CHECK (PULONG_FROM_BE (pulong_val) == (pulong) 0x0000000000000800ULL);
			BOOST_CHECK (PULONG_TO_LE (pulong_val) == (pulong) 0x0008000000000000ULL);
			BOOST_CHECK (PULONG_FROM_LE (pulong_val) == (pulong) 0x0008000000000000ULL);
		} else {
			plong plong_val = PLONG_TO_BE (0xFFFFF800);
			BOOST_CHECK (plong_val == (plong) 0x00F8FFFF);
			BOOST_CHECK (PLONG_FROM_BE (plong_val) == (plong) 0xFFFFF800);
			BOOST_CHECK (PLONG_TO_LE (plong_val) == (plong) 0x00F8FFFF);
			BOOST_CHECK (PLONG_FROM_LE (plong_val) == (plong) 0x00F8FFFF);

			pulong pulong_val = PULONG_TO_BE (0x00000800);
			BOOST_CHECK (pulong_val == (pulong) 0x00080000);
			BOOST_CHECK (PULONG_FROM_BE (pulong_val) == (pulong) 0x00000800);
			BOOST_CHECK (PULONG_TO_LE (pulong_val) == (pulong) 0x00080000);
			BOOST_CHECK (PULONG_FROM_LE (pulong_val) == (pulong) 0x00080000);
		}

		if (PLIBSYS_SIZEOF_SIZE_T == 8) {
			psize psize_val = PSIZE_TO_BE (0x0000000000001000ULL);
			BOOST_CHECK (psize_val == (psize) 0x0010000000000000ULL);
			BOOST_CHECK (PSIZE_FROM_BE (psize_val) == (psize) 0x0000000000001000ULL);
			BOOST_CHECK (PSIZE_TO_LE (psize_val) == (psize) 0x0010000000000000ULL);
			BOOST_CHECK (PSIZE_FROM_LE (psize_val) == (psize) 0x0010000000000000ULL);

			pssize pssize_val = PSSIZE_TO_BE (0x000000000000F000LL);
			BOOST_CHECK (pssize_val == (pssize) 0x00F0000000000000LL);
			BOOST_CHECK (PSSIZE_FROM_BE (pssize_val) == (pssize) 0x000000000000F000LL);
			BOOST_CHECK (PSSIZE_TO_LE (pssize_val) == (pssize) 0x00F0000000000000LL);
			BOOST_CHECK (PSSIZE_FROM_LE (pssize_val) == (pssize) 0x00F0000000000000LL);
		} else {
			psize psize_val = PSIZE_TO_BE (0x00001000);
			BOOST_CHECK (psize_val == (psize) 0x00100000);
			BOOST_CHECK (PSIZE_FROM_BE (psize_val) == (psize) 0x00001000);
			BOOST_CHECK (PSIZE_TO_LE (psize_val) == (psize) 0x00100000);
			BOOST_CHECK (PSIZE_FROM_LE (psize_val) == (psize) 0x00100000);

			pssize pssize_val = PSSIZE_TO_BE (0x0000F000);
			BOOST_CHECK (pssize_val == (pssize) 0x00F00000);
			BOOST_CHECK (PSSIZE_FROM_BE (pssize_val) == (pssize) 0x0000F000);
			BOOST_CHECK (PSSIZE_TO_LE (pssize_val) == (pssize) 0x00F00000);
			BOOST_CHECK (PSSIZE_FROM_LE (pssize_val) == (pssize) 0x00F00000);
		}

		puint16_val = p_htons (0x0020);
		BOOST_CHECK (puint16_val == (puint16) 0x2000);
		BOOST_CHECK (p_ntohs (puint16_val) == (puint16) 0x0020);

		puint32_val = p_htonl (0x00000040);
		BOOST_CHECK (puint32_val == (puint32) 0x40000000);
		BOOST_CHECK (p_ntohl (puint32_val) == (puint32) 0x00000040);
	} else {
		pint16 pint16_val = PINT16_TO_LE (0xFFE0);
		BOOST_CHECK (pint16_val == (pint16) 0xE0FF);
		BOOST_CHECK (PINT16_FROM_LE (pint16_val) == (pint16) 0xFFE0);
		BOOST_CHECK (PINT16_TO_BE (pint16_val) == (pint16) 0xE0FF);
		BOOST_CHECK (PINT16_FROM_BE (pint16_val) == (pint16) 0xE0FF);

		puint16 puint16_val = PUINT16_TO_LE (0x0020);
		BOOST_CHECK (puint16_val == (puint16) 0x2000);
		BOOST_CHECK (PUINT16_FROM_LE (puint16_val) == (puint16) 0x0020);
		BOOST_CHECK (PUINT16_TO_BE (puint16_val) == (puint16) 0x2000);
		BOOST_CHECK (PUINT16_FROM_BE (puint16_val) == (puint16) 0x2000);

		pint32 pint32_val = PINT32_TO_LE (0xFFFFFFC0);
		BOOST_CHECK (pint32_val == (pint32) 0xC0FFFFFF);
		BOOST_CHECK (PINT32_FROM_LE (pint32_val) == (pint32) 0xFFFFFFC0);
		BOOST_CHECK (PINT32_TO_BE (pint32_val) == (pint32) 0xC0FFFFFF);
		BOOST_CHECK (PINT32_FROM_BE (pint32_val) == (pint32) 0xC0FFFFFF);

		puint32 puint32_val = PUINT32_TO_LE (0x00000040);
		BOOST_CHECK (puint32_val == (puint32) 0x40000000);
		BOOST_CHECK (PUINT32_FROM_LE (puint32_val) == (puint32) 0x00000040);
		BOOST_CHECK (PUINT32_TO_BE (puint32_val) == (puint32) 0x40000000);
		BOOST_CHECK (PUINT32_FROM_BE (puint32_val) == (puint32) 0x40000000);

		pint64 pint64_val = PINT64_TO_LE (0xFFFFFFFFFFFFFF80LL);
		BOOST_CHECK (pint64_val == (pint64) 0x80FFFFFFFFFFFFFFLL);
		BOOST_CHECK (PINT64_FROM_LE (pint64_val) == (pint64) 0xFFFFFFFFFFFFFF80LL);
		BOOST_CHECK (PINT64_TO_BE (pint64_val) == (pint64) 0x80FFFFFFFFFFFFFFLL);
		BOOST_CHECK (PINT64_FROM_BE (pint64_val) == (pint64) 0x80FFFFFFFFFFFFFFLL);

		puint64 puint64_val = PUINT64_TO_LE (0x0000000000000080ULL);
		BOOST_CHECK (puint64_val == (puint64) 0x8000000000000000ULL);
		BOOST_CHECK (PUINT64_FROM_LE (puint64_val) == (puint64) 0x0000000000000080ULL);
		BOOST_CHECK (PUINT64_TO_BE (puint64_val) == (puint64) 0x8000000000000000ULL);
		BOOST_CHECK (PUINT64_FROM_BE (puint64_val) == (puint64) 0x8000000000000000ULL);

		pint pint_val = PINT_TO_LE (0xFFFFFC00);
		BOOST_CHECK (pint_val == (pint) 0x00FCFFFF);
		BOOST_CHECK (PINT_FROM_LE (pint_val) == (pint) 0xFFFFFC00);
		BOOST_CHECK (PINT_TO_BE (pint_val) == (pint) 0x00FCFFFF);
		BOOST_CHECK (PINT_FROM_BE (pint_val) == (pint) 0x00FCFFFF);

		puint puint_val = PUINT_TO_LE (0x00000400);
		BOOST_CHECK (puint_val == (puint) 0x00040000);
		BOOST_CHECK (PUINT_FROM_LE (puint_val) == (puint) 0x00000400);
		BOOST_CHECK (PUINT_TO_BE (puint_val) == (puint) 0x00040000);
		BOOST_CHECK (PUINT_FROM_BE (puint_val) == (puint) 0x00040000);

		if (PLIBSYS_SIZEOF_LONG == 8) {
			plong plong_val = PLONG_TO_LE (0xFFFFFFFFFFFFF800LL);
			BOOST_CHECK (plong_val == (plong) 0x00F8FFFFFFFFFFFFLL);
			BOOST_CHECK (PLONG_FROM_LE (plong_val) == (plong) 0xFFFFFFFFFFFFF800LL);
			BOOST_CHECK (PLONG_TO_BE (plong_val) == (plong) 0x00F8FFFFFFFFFFFFLL);
			BOOST_CHECK (PLONG_FROM_BE (plong_val) == (plong) 0x00F8FFFFFFFFFFFFLL);

			pulong pulong_val = PULONG_TO_LE (0x0000000000000800ULL);
			BOOST_CHECK (pulong_val == (pulong) 0x0008000000000000ULL);
			BOOST_CHECK (PULONG_FROM_LE (pulong_val) == (pulong) 0x0000000000000800ULL);
			BOOST_CHECK (PULONG_TO_BE (pulong_val) == (pulong) 0x0008000000000000ULL);
			BOOST_CHECK (PULONG_FROM_BE (pulong_val) == (pulong) 0x0008000000000000ULL);
		} else {
			plong plong_val = PLONG_TO_LE (0xFFFFF800);
			BOOST_CHECK (plong_val == (plong) 0x00F8FFFF);
			BOOST_CHECK (PLONG_FROM_LE (plong_val) == (plong) 0xFFFFF800);
			BOOST_CHECK (PLONG_TO_BE (plong_val) == (plong) 0x00F8FFFF);
			BOOST_CHECK (PLONG_FROM_BE (plong_val) == (plong) 0x00F8FFFF);

			pulong pulong_val = PULONG_TO_LE (0x00000800);
			BOOST_CHECK (pulong_val == (pulong) 0x00080000);
			BOOST_CHECK (PULONG_FROM_LE (pulong_val) == (pulong) 0x00000800);
			BOOST_CHECK (PULONG_TO_BE (pulong_val) == (pulong) 0x00080000);
			BOOST_CHECK (PULONG_FROM_BE (pulong_val) == (pulong) 0x00080000);
		}

		if (PLIBSYS_SIZEOF_SIZE_T == 8) {
			psize psize_val = PSIZE_TO_LE (0x0000000000001000ULL);
			BOOST_CHECK (psize_val == (psize) 0x0010000000000000ULL);
			BOOST_CHECK (PSIZE_FROM_LE (psize_val) == (psize) 0x0000000000001000ULL);
			BOOST_CHECK (PSIZE_TO_BE (psize_val) == (psize) 0x0010000000000000ULL);
			BOOST_CHECK (PSIZE_FROM_BE (psize_val) == (psize) 0x0010000000000000ULL);

			pssize pssize_val = PSSIZE_TO_LE (0x000000000000F000LL);
			BOOST_CHECK (pssize_val == (pssize) 0x00F0000000000000LL);
			BOOST_CHECK (PSSIZE_FROM_LE (pssize_val) == (pssize) 0x000000000000F000LL);
			BOOST_CHECK (PSSIZE_TO_BE (pssize_val) == (pssize) 0x00F0000000000000LL);
			BOOST_CHECK (PSSIZE_FROM_BE (pssize_val) == (pssize) 0x00F0000000000000LL);
		} else {
			psize psize_val = PSIZE_TO_LE (0x00001000);
			BOOST_CHECK (psize_val == (psize) 0x00100000);
			BOOST_CHECK (PSIZE_FROM_LE (psize_val) == (psize) 0x00001000);
			BOOST_CHECK (PSIZE_TO_BE (psize_val) == (psize) 0x00100000);
			BOOST_CHECK (PSIZE_FROM_BE (psize_val) == (psize) 0x00100000);

			pssize pssize_val = PSSIZE_TO_LE (0x0000F000);
			BOOST_CHECK (pssize_val == (pssize) 0x00F00000);
			BOOST_CHECK (PSSIZE_FROM_LE (pssize_val) == (pssize) 0x0000F000);
			BOOST_CHECK (PSSIZE_TO_BE (pssize_val) == (pssize) 0x00F00000);
			BOOST_CHECK (PSSIZE_FROM_BE (pssize_val) == (pssize) 0x00F00000);
		}

		puint16_val = p_htons (0x0020);
		BOOST_CHECK (puint16_val == (puint16) 0x0020);
		BOOST_CHECK (p_ntohs (puint16_val) == (puint16) 0x0020);

		puint32_val = p_htonl (0x00000040);
		BOOST_CHECK (puint32_val == (puint32) 0x00000040);
		BOOST_CHECK (p_ntohl (puint32_val) == (puint32) 0x00000040);
	}

	puint16 puint16_val = PUINT16_SWAP_BYTES (0x0020);
	BOOST_CHECK (puint16_val == (puint16) 0x2000);
	BOOST_CHECK (PUINT16_SWAP_BYTES (puint16_val) == (puint16) 0x0020);

	puint32 puint32_val = PUINT32_SWAP_BYTES (0x00000040);
	BOOST_CHECK (puint32_val == (puint32) 0x40000000);
	BOOST_CHECK (PUINT32_SWAP_BYTES (puint32_val) == (puint32) 0x00000040);

	puint64 puint64_val = PUINT64_SWAP_BYTES (0x0000000000000080ULL);
	BOOST_CHECK (puint64_val == (puint64) 0x8000000000000000ULL);
	BOOST_CHECK (PUINT64_SWAP_BYTES (puint64_val) == (puint64) 0x0000000000000080ULL);

	p_libsys_shutdown ();
}

BOOST_AUTO_TEST_SUITE_END()
