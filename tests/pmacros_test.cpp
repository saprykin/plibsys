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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA.
 */

#ifndef PLIB_TESTS_STATIC
#  define BOOST_TEST_DYN_LINK
#endif

#define BOOST_TEST_MODULE pmacros_test

#include "plib.h"

#ifdef PLIB_TESTS_STATIC
#  include <boost/test/included/unit_test.hpp>
#else
#  include <boost/test/unit_test.hpp>
#endif

static P_GNUC_WARN_UNUSED_RESULT pint unused_result_test_func ()
{
	return 0;
}

BOOST_AUTO_TEST_SUITE (BOOST_TEST_MODULE)

BOOST_AUTO_TEST_CASE (pmacros_general_test)
{
	p_lib_init ();

	/* Test OS detection macros */
#if !defined (P_OS_DARWIN) && !defined (P_OS_MAC9) && !defined (P_OS_BSD4) && \
    !defined (P_OS_AIX) && !defined (P_OS_HPUX) && !defined (P_OS_SOLARIS) && \
    !defined (P_OS_QNX) && !defined (P_OS_QNX6) && !defined (P_OS_UNIX) && \
    !defined (P_OS_LINUX) && !defined (P_OS_WIN) && !defined (P_OS_CYGWIN)
	BOOST_CHECK (false);
#endif

	/* Test for Mac OS */
#if defined (P_OS_MAC9) && defined (P_OS_UNIX)
	BOOST_CHECK (false);
#endif

#if defined (P_OS_MAC9) && defined (P_OS_MAC)
	BOOST_CHECK (false);
#endif

#if defined (P_OS_MAC) && !defined (P_OS_MAC32) && !defined (P_OS_MAC64)
	BOOST_CHECK (false);
#endif

#if defined (P_OS_MAC) && (!defined (P_OS_DARWIN) || !defined (P_OS_BSD4))
	BOOST_CHECK (false);
#endif

#if defined (P_OS_MAC32) && !defined (P_OS_DARWIN32)
	BOOST_CHECK (false);
#endif

#if defined (P_OS_MAC64) && !defined (P_OS_DARWIN64)
	BOOST_CHECK (false);
#endif

#if defined (P_OS_MAC32) && defined (P_OS_MAC64)
	BOOST_CHECK (false);
#endif

	/* Test for Windows */
#if defined (P_OS_WIN64) && !defined (P_OS_WIN)
	BOOST_CHECK (false);
#endif

#if defined (P_OS_WIN) && defined (P_OS_UNIX)
	BOOST_CHECK (false);
#endif

	/* Test for FreeBSD */
#if defined (P_OS_FREEBSD) && !defined (P_OS_BSD4)
	BOOST_CHECK (false);
#endif

	/* Test for NetBSD */
#if defined (P_OS_NETBSD) && !defined (P_OS_BSD4)
	BOOST_CHECK (false);
#endif

	/* Test for OpenBSD */
#if defined (P_OS_OPENBSD) && !defined (P_OS_BSD4)
	BOOST_CHECK (false);
#endif

	/* Test for compiler detection macros */
#if !defined (P_CC_MSVC) && !defined (P_CC_GNU) && !defined (P_CC_MINGW) && \
    !defined (P_CC_INTEL) && !defined (P_CC_CLANG) && !defined (P_CC_SUN) && \
    !defined (P_CC_XLC) && !defined (P_CC_HP) && !defined (P_CC_WATCOM)
	BOOST_CHECK (false);
#endif

	/* Test other macros */
	pint unused = 8;
	P_UNUSED (unused);

	pint result = unused_result_test_func ();

	P_WARNING ("Test warning output");
	P_ERROR ("Test error output");
	P_DEBUG ("Test debug output");

	p_lib_shutdown ();
}

BOOST_AUTO_TEST_SUITE_END()
