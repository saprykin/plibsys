/*
 * The MIT License
 *
 * Copyright (C) 2014-2018 Alexander Saprykin <saprykin.spb@gmail.com>
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

#include <stdlib.h>
#include <time.h>

P_TEST_MODULE_INIT ();

static P_GNUC_WARN_UNUSED_RESULT pint unused_result_test_func ()
{
	return 0;
}

P_LIB_INTERNAL_API int internal_api_test ()
{
	return 0;
}

P_LIB_GLOBAL_API int global_api_test ()
{
	return 0;
}

P_TEST_CASE_BEGIN (pmacros_general_test)
{
	p_libsys_init ();

	/* Test OS detection macros */
#if !defined (P_OS_DARWIN) && !defined (P_OS_MAC9)     && !defined (P_OS_BSD4)      && \
    !defined (P_OS_AIX)    && !defined (P_OS_HPUX)     && !defined (P_OS_SOLARIS)   && \
    !defined (P_OS_QNX)    && !defined (P_OS_QNX6)     && !defined (P_OS_UNIX)      && \
    !defined (P_OS_LINUX)  && !defined (P_OS_WIN)      && !defined (P_OS_CYGWIN)    && \
    !defined (P_OS_SCO)    && !defined (P_OS_UNIXWARE) && !defined (P_OS_VMS)       && \
    !defined (P_OS_IRIX)   && !defined (P_OS_MSYS)     && !defined (P_OS_DRAGONFLY) && \
    !defined (P_OS_HAIKU)  && !defined (P_OS_TRU64)    && !defined (P_OS_SYLLABLE)  && \
    !defined (P_OS_BEOS)   && !defined (P_OS_OS2)      && !defined (P_OS_AMIGA)
	P_TEST_CHECK (false);
#endif

	/* Test for Mac OS */
#if defined (P_OS_MAC9) && defined (P_OS_UNIX)
	P_TEST_CHECK (false);
#endif

#if defined (P_OS_MAC9) && defined (P_OS_MAC)
	P_TEST_CHECK (false);
#endif

#if defined (P_OS_MAC) && !defined (P_OS_MAC32) && !defined (P_OS_MAC64)
	P_TEST_CHECK (false);
#endif

#if defined (P_OS_MAC) && (!defined (P_OS_DARWIN) || !defined (P_OS_BSD4))
	P_TEST_CHECK (false);
#endif

#if defined (P_OS_MAC32) && !defined (P_OS_DARWIN32)
	P_TEST_CHECK (false);
#endif

#if defined (P_OS_MAC64) && !defined (P_OS_DARWIN64)
	P_TEST_CHECK (false);
#endif

#if defined (P_OS_MAC32) && defined (P_OS_MAC64)
	P_TEST_CHECK (false);
#endif

	/* Test for Windows */
#if defined (P_OS_WIN64) && !defined (P_OS_WIN)
	P_TEST_CHECK (false);
#endif

#if defined (P_OS_WIN) && defined (P_OS_UNIX)
	P_TEST_CHECK (false);
#endif

	/* Test for FreeBSD */
#if defined (P_OS_FREEBSD) && !defined (P_OS_BSD4)
	P_TEST_CHECK (false);
#endif

	/* Test for DragonFlyBSD */
#if defined (P_OS_DRAGONFLY) && !defined (P_OS_BSD4)
	P_TEST_CHECK (false);
#endif

	/* Test for NetBSD */
#if defined (P_OS_NETBSD) && !defined (P_OS_BSD4)
	P_TEST_CHECK (false);
#endif

	/* Test for OpenBSD */
#if defined (P_OS_OPENBSD) && !defined (P_OS_BSD4)
	P_TEST_CHECK (false);
#endif

	/* Test for Android */
#if defined (P_OS_ANDROID) && !defined (P_OS_LINUX)
	P_TEST_CHECK (false);
#endif

	/* Test for others */
#if defined (P_OS_HAIKU) || defined (P_OS_BEOS) || defined (P_OS_OS2) || defined (P_OS_VMS) || \
    defined (P_OS_AMIGA)
#  if defined (P_OS_UNIX)
	P_TEST_CHECK (false);
#  endif
#endif

	/* Test for compiler detection macros */
#if !defined (P_CC_MSVC)    && !defined (P_CC_GNU)   && !defined (P_CC_MINGW)  && \
    !defined (P_CC_INTEL)   && !defined (P_CC_CLANG) && !defined (P_CC_SUN)    && \
    !defined (P_CC_XLC)     && !defined (P_CC_HP)    && !defined (P_CC_WATCOM) && \
    !defined (P_CC_BORLAND) && !defined (P_CC_MIPS)  && !defined (P_CC_USLC)   && \
    !defined (P_CC_DEC)     && !defined (P_CC_PGI)   && !defined (P_CC_CRAY)
	P_TEST_CHECK (false);
#endif

#if defined (P_CC_MSVC)
#  if !defined (P_OS_WIN)
	P_TEST_CHECK (false);
#  endif
#endif

#if defined (P_CC_INTEL)
#  if !defined (P_OS_WIN)   && !defined (P_OS_MAC)     && \
      !defined (P_OS_LINUX) && !defined (P_OS_FREEBSD) && \
      !defined (P_OS_QNX6)
	P_TEST_CHECK (false);
#  endif
#endif

#if defined (P_CC_SUN)
#  if !defined (P_OS_SOLARIS) && !defined (P_OS_LINUX)
	P_TEST_CHECK (false);
#  endif
#endif

#if defined (P_CC_XLC)
#  if !defined (P_OS_AIX) && !defined (P_OS_LINUX)
	P_TEST_CHECK (false);
#  endif
#endif

#if defined (P_CC_HP)
#  if !defined (P_OS_HPUX)
	P_TEST_CHECK (false);
#  endif
#endif

#if defined (P_CC_WATCOM)
#  if !defined (P_OS_WIN) && !defined (P_OS_LINUX) && \
      !defined (P_OS_OS2) && !defined (P_OS_QNX)
	P_TEST_CHECK (false);
#  endif
#endif

#if defined (P_CC_BORLAND)
#  if !defined (P_OS_WIN)
	P_TEST_CHECK (false);
#  endif
#endif

#if defined (P_CC_MIPS)
#  if !defined (P_OS_IRIX)
	P_TEST_CHECK (false);
#  endif
#endif

#if defined (P_CC_USLC)
#  if !defined (P_OS_SCO) && !defined (P_OS_UNIXWARE)
	P_TEST_CHECK (false);
#  endif
#endif

#if defined (P_CC_DEC)
#  if !defined (P_OS_VMS) && !defined (P_OS_TRU64)
	P_TEST_CHECK (false);
#  endif
#endif

#if defined (P_CC_PGI)
#  if !defined (P_OS_WIN) && !defined (P_OS_MAC) && !defined (P_OS_LINUX)
	P_TEST_CHECK (false);
#  endif
#endif

#if defined (P_CC_CRAY)
#  if !defined (P_OS_LINUX)
	P_TEST_CHECK (false);
#  endif
#endif

	/* Test for CPU architecture detection macros */
#if defined (P_OS_VMS)
#  if !defined (P_CPU_ALPHA) && !defined (P_CPU_IA64)
	P_TEST_CHECK (false);
#  endif
#endif

#if defined (P_OS_AMIGA)
#  if !defined (P_CPU_POWER)
	P_TEST_CHECK (false);
#  endif
#endif

#if defined (P_OS_TRU64)
#  if !defined (P_CPU_ALPHA)
	P_TEST_CHECK (false);
#  endif
#endif

#if defined (P_OS_AIX)
#  if !defined (P_CPU_POWER)
	P_TEST_CHECK (false);
#  endif
#endif

#if defined (P_OS_HPUX)
#  if !defined (P_CPU_HPPA) && !defined (P_CPU_IA64)
	P_TEST_CHECK (false);
#  endif
#endif

#if defined (P_OS_SOLARIS)
#  if !defined (P_CPU_X86) && !defined (P_CPU_SPARC) && !defined (P_CPU_POWER)
	P_TEST_CHECK (false);
#  endif
#endif

#if defined (P_OS_QNX)
#  if !defined (P_CPU_X86)
	P_TEST_CHECK (false);
#  endif
#endif

#if defined (P_OS_QNX6)
#  if !defined (P_CPU_X86)  && !defined (P_CPU_ARM) && \
      !defined (P_CPU_MIPS) && !defined (P_CPU_POWER)
	P_TEST_CHECK (false);
#  endif
#endif

#if defined (P_OS_BB10)
#  if !defined(P_CPU_X86) && !defined (P_CPU_ARM)
	P_TEST_CHECK (false);
#  endif
#endif

#if defined (P_OS_SCO)
#  if !defined (P_CPU_X86)
	P_TEST_CHECK (false);
#  endif
#endif

#if defined (P_OS_UNIXWARE)
#  if !defined (P_CPU_X86)
	P_TEST_CHECK (false);
#  endif
#endif

#if defined (P_OS_IRIX)
#  if !defined (P_CPU_MIPS)
	P_TEST_CHECK (false);
#  endif
#endif

#if defined (P_OS_HAIKU)
#  if !defined (P_CPU_X86) && !defined (P_CPU_ARM)
	P_TEST_CHECK (false);
#  endif
#endif

#if defined (P_OS_SYLLABLE)
#  if !defined (P_CPU_X86)
	P_TEST_CHECK (false);
#  endif
#endif

#if defined (P_OS_BEOS)
#  if !defined (P_CPU_X86) && !defined (P_CPU_POWER)
	P_TEST_CHECK (false);
#  endif
#endif

#if defined (P_OS_OS2)
#  if !defined (P_CPU_X86)
	P_TEST_CHECK (false);
#  endif
#endif

#if defined (P_OS_MAC9)
#  if !defined (P_CPU_POWER)
	P_TEST_CHECK (false);
#  endif
#endif

#if defined (P_OS_MAC)
#  if !defined (P_CPU_X86) && !defined (P_CPU_POWER)
	P_TEST_CHECK (false);
#  endif
#endif

#if defined (P_OS_WIN)
#  if !defined (P_CPU_X86)  && !defined (P_CPU_ARM)   && !defined (P_CPU_IA64) && \
      !defined (P_CPU_MIPS) && !defined (P_CPU_POWER) && !defined (P_CPU_ALPHA)
	P_TEST_CHECK (false);
#  endif
#endif

#if defined (P_OS_ANDROID)
#  if !defined (P_CPU_X86) && !defined (P_CPU_ARM) && !defined (P_CPU_MIPS)
	P_TEST_CHECK (false);
#  endif
#endif

#if defined (P_CC_MSVC)
#  if !defined (P_CPU_X86)  && !defined (P_CPU_ARM)   && !defined (P_CPU_IA64) && \
      !defined (P_CPU_MIPS) && !defined (P_CPU_POWER) && !defined (P_CPU_ALPHA)
	P_TEST_CHECK (false);
#  endif
#endif

#if defined (P_CC_SUN)
#  if !defined (P_CPU_X86) && !defined (P_CPU_SPARC)
	P_TEST_CHECK (false);
#  endif
#endif

#if defined (P_CC_XLC)
#  if !defined (P_CPU_POWER)
	P_TEST_CHECK (false);
#  endif
#endif

#if defined (P_CC_HP)
#  if !defined (P_CPU_HPPA) && !defined (P_CPU_IA64)
	P_TEST_CHECK (false);
#  endif
#endif

#if defined (P_CC_DEC)
#  if !defined (P_CPU_ALPHA) && !defined (P_CPU_IA64)
	P_TEST_CHECK (false);
#  endif
#endif

#if defined (P_CC_MIPS)
#  if !defined (P_CPU_MIPS)
	P_TEST_CHECK (false);
#  endif
#endif

#if defined (P_CC_USLC)
#  if !defined (P_CPU_X86)
	P_TEST_CHECK (false);
#  endif
#endif

#if defined (P_CC_WATCOM)
#  if !defined (P_CPU_X86)
	P_TEST_CHECK (false);
#  endif
#endif

#if defined (P_CC_BORLAND)
#  if !defined (P_CPU_X86)
	P_TEST_CHECK (false);
#  endif
#endif

#if defined (P_CC_PGI)
#  if !defined (P_CPU_X86) && !defined (P_CPU_POWER)
	P_TEST_CHECK (false);
#  endif
#endif

#if defined (P_CPU_ARM)
#  if !defined (P_CPU_ARM_V2) && !defined (P_CPU_ARM_V3) && !defined (P_CPU_ARM_V4) && \
      !defined (P_CPU_ARM_V5) && !defined (P_CPU_ARM_V6) && !defined (P_CPU_ARM_V7) && \
      !defined (P_CPU_ARM_V8)
	P_TEST_CHECK (false);
#  endif
#endif

#if defined (P_CPU_ARM_V2)
#  if !defined (P_CPU_ARM) || !(P_CPU_ARM - 0 == 2)
	P_TEST_CHECK (false);
#  endif
#endif

#if defined (P_CPU_ARM_V3)
#  if !defined (P_CPU_ARM) || !(P_CPU_ARM - 0 == 3)
	P_TEST_CHECK (false);
#  endif
#endif

#if defined (P_CPU_ARM_V4)
#  if !defined (P_CPU_ARM) || !(P_CPU_ARM - 0 == 4)
	P_TEST_CHECK (false);
#  endif
#endif

#if defined (P_CPU_ARM_V5)
#  if !defined (P_CPU_ARM) || !(P_CPU_ARM - 0 == 5)
	P_TEST_CHECK (false);
#  endif
#endif

#if defined (P_CPU_ARM_V6)
#  if !defined (P_CPU_ARM) || !(P_CPU_ARM - 0 == 6)
	P_TEST_CHECK (false);
#  endif
#endif

#if defined (P_CPU_ARM_V7)
#  if !defined (P_CPU_ARM) || !(P_CPU_ARM - 0 == 7)
	P_TEST_CHECK (false);
#  endif
#endif

#if defined (P_CPU_ARM_V8)
#  if !defined (P_CPU_ARM) || !(P_CPU_ARM - 0 == 8)
	P_TEST_CHECK (false);
#  endif
#endif

#if defined (P_CPU_ARM) && !(P_CPU_ARM - 0 > 0)
	P_TEST_CHECK (false);
#endif

#if defined (P_CPU_ARM)
#  if !defined (P_CPU_ARM_32) && !defined (P_CPU_ARM_64)
	P_TEST_CHECK (false);
#  endif
#endif

#if defined (P_CPU_ARM_32) || defined (P_CPU_ARM_64)
#  if !defined (P_CPU_ARM)
	P_TEST_CHECK (false);
#  endif
#endif

#if defined (P_CPU_ARM_32)
#  ifdef P_CPU_ARM_64
	P_TEST_CHECK (false);
#  endif
#endif

#if defined (P_CPU_ARM_64)
#  ifdef P_CPU_ARM_32
	P_TEST_CHECK (false);
#  endif
#endif

#if defined (P_CPU_X86)
#  if !((P_CPU_X86 >= 3) && (P_CPU_X86 <= 6))
	P_TEST_CHECK (false);
#  endif
#  if !defined (P_CPU_X86_32) && !defined (P_CPU_X86_64)
	P_TEST_CHECK (false);
#  endif
#endif

#if defined (P_CPU_X86_32) || defined (P_CPU_X86_64)
#  if !defined (P_CPU_X86)
	P_TEST_CHECK (false);
#  endif
#endif

#if defined (P_CPU_X86_32) && defined (P_CPU_X86_64)
	P_TEST_CHECK (false);
#endif

#if defined (P_CPU_X86_64)
#  if !defined (P_CPU_X86) || !(P_CPU_X86 - 0 == 6)
	P_TEST_CHECK (false);
#  endif
#endif

#if defined (P_CPU_MIPS)
#  if !defined (P_CPU_MIPS_I)  && !defined (P_CPU_MIPS_II) && !defined (P_CPU_MIPS_III) && \
      !defined (P_CPU_MIPS_IV) && !defined (P_CPU_MIPS_V)  && !defined (P_CPU_MIPS_32)  && \
      !defined (P_CPU_MIPS_64)
	P_TEST_CHECK (false);
#  endif
#endif

#if defined (P_CPU_MIPS_I)  || defined (P_CPU_MIPS_II) || defined (P_CPU_MIPS_III) || \
    defined (P_CPU_MIPS_IV) || defined (P_CPU_MIPS_V)  || defined (P_CPU_MIPS_32)  || \
    defined (P_CPU_MIPS_64)
#  if !defined (P_CPU_MIPS)
	P_TEST_CHECK (false);
#  endif
#endif

#if defined (P_CPU_MIPS_II)
#  if !defined (P_CPU_MIPS_I)
	P_TEST_CHECK (false);
#  endif
#endif

#if defined (P_CPU_MIPS_III)
#  if !defined (P_CPU_MIPS_II)
	P_TEST_CHECK (false);
#  endif
#endif

#if defined (P_CPU_MIPS_IV)
#  if !defined (P_CPU_MIPS_III)
	P_TEST_CHECK (false);
#  endif
#endif

#if defined (P_CPU_MIPS_V)
#  if !defined (P_CPU_MIPS_IV)
	P_TEST_CHECK (false);
#  endif
#endif

#if defined (P_CPU_MIPS_32)
#  if !defined (P_CPU_MIPS_II)
	P_TEST_CHECK (false);
#  endif
#endif

#if defined (P_CPU_MIPS_64)
#  if !defined (P_CPU_MIPS_V)
	P_TEST_CHECK (false);
#  endif
#endif

#if defined (P_CPU_POWER)
#  if !defined (P_CPU_POWER_32) && !defined (P_CPU_POWER_64)
	P_TEST_CHECK (false);
#  endif
#endif

#if defined (P_CPU_POWER_32) || defined (P_CPU_POWER_64)
#  if !defined (P_CPU_POWER)
	P_TEST_CHECK (false);
#  endif
#endif

#if defined (P_CPU_POWER_32) && defined (P_CPU_POWER_64)
	P_TEST_CHECK (false);
#endif

#if defined (P_CPU_SPARC_V8) || defined (P_CPU_SPARC_V9)
#  if !defined (P_CPU_SPARC)
	P_TEST_CHECK (false);
#  endif
#endif

#if defined (P_CPU_SPARC_V8) && defined (P_CPU_SPARC_V9)
	P_TEST_CHECK (false);
#endif

#if defined (P_CPU_HPPA)
#  if !defined (P_CPU_HPPA_32) && !defined (P_CPU_HPPA_64)
	P_TEST_CHECK (false);
#  endif
#endif

#if defined (P_CPU_HPPA_32) || defined (P_CPU_HPPA_64)
#  if !defined (P_CPU_HPPA)
	P_TEST_CHECK (false);
#  endif
#endif

#if defined (P_CPU_HPPA_32) && defined (P_CPU_HPPA_64)
	P_TEST_CHECK (false);
#endif

	/* Test other macros */
	pint unused;
	P_UNUSED (unused);

	pint result = unused_result_test_func ();

	P_UNUSED (result);

	P_TEST_CHECK (internal_api_test () == 0);
	P_TEST_CHECK (global_api_test () == 0);

	P_WARNING ("Test warning output");
	P_ERROR ("Test error output");
	P_DEBUG ("Test debug output");

	srand ((unsigned int) time (NULL));

	pint rand_number = rand ();

	if (P_LIKELY (rand_number > 0))
		P_DEBUG ("Likely condition triggered");

	if (P_UNLIKELY (rand_number == 0))
		P_DEBUG ("Unlikely condition triggered");

	/* Test version macros */
	P_TEST_CHECK (PLIBSYS_VERSION_MAJOR >= 0);
	P_TEST_CHECK (PLIBSYS_VERSION_MINOR >= 0);
	P_TEST_CHECK (PLIBSYS_VERSION_PATCH >= 0);
	P_TEST_CHECK (PLIBSYS_VERSION >= 0);

#if !defined (PLIBSYS_VERSION_STR)
	P_TEST_CHECK (false);
#endif

	P_TEST_CHECK (PLIBSYS_VERSION >= PLIBSYS_VERSION_CHECK (0, 0, 1));

	p_libsys_shutdown ();
}
P_TEST_CASE_END ()

P_TEST_SUITE_BEGIN()
{
	P_TEST_SUITE_RUN_CASE (pmacros_general_test);
}
P_TEST_SUITE_END()
