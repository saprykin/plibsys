/*
 * Copyright (C) 2010-2017 Alexander Saprykin <xelfium@gmail.com>
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
 * @file pmacros.h
 * @brief Miscellaneous macros
 * @author Alexander Saprykin
 *
 * All the macros are completely independent of any other platform-specific
 * headers, thus gurantee to work with any compiler under any operating system
 * in the same way as they are used within the library.
 *
 * Provided macros can be divided into the three groups:
 * - CPU architecture detection (P_CPU_x);
 * - operating system detection (P_OS_x);
 * - compiler detection (P_CC_x);
 * - other general macros (compiler hints, attributes, version, etc.).
 *
 * Macros from the first three groups are defined only for the particularly
 * detected CPU architectures, operating systems and compilers. Macros from the
 * latter group are always defined.
 */

#if !defined (PLIBSYS_H_INSIDE) && !defined (PLIBSYS_COMPILATION)
#  error "Header files shouldn't be included directly, consider using <plibsys.h> instead."
#endif

#ifndef PLIBSYS_HEADER_PMACROS_H
#define PLIBSYS_HEADER_PMACROS_H

#include <stdio.h>

/*
 * List of supported operating systems (P_OS_x):
 *
 * DARWIN            - Any Darwin based system
 * DARWIN32          - Any 32-bit Darwin based system
 * DARWIN64          - Any 64-bit Darwin based system
 * BSD4              - Any BSD 4.x based system
 * FREEBSD           - FreeBSD
 * DRAGONFLY         - DragonFlyBSD
 * NETBSD            - NetBSD
 * OPENBSD           - OpenBSD
 * AIX               - IBM AIX
 * HPUX              - HP-UX
 * TRU64             - Tru64
 * SOLARIS           - Sun (Oracle) Solaris
 * QNX               - QNX 4.x
 * QNX6              - QNX Neutrino 6.x
 * SCO               - SCO OpenServer 5/6
 * UNIXWARE          - UnixWare 7
 * IRIX              - SGI IRIX
 * HAIKU             - Haiku
 * SYLLABLE          - Syllable
 * BEOS              - BeOS
 * OS2               - OS/2
 * VMS               - OpenVMS
 * UNIX              - Any UNIX BSD/SYSV based system
 * LINUX             - Linux
 * MAC9              - Mac OS 9 (Classic)
 * MAC               - Any macOS
 * MAC32             - 32-bit macOS
 * MAC64             - 64-bit macOS
 * CYGWIN            - Cygwin
 * WIN               - 32-bit Windows
 * WIN64             - 64-bit Windows
 */

/**
 * @def P_OS_DARWIN
 * @brief Darwin based operating system (i.e. macOS).
 * @since 0.0.1
 */

/**
 * @def P_OS_DARWIN32
 * @brief Darwin based 32-bit operating system.
 * @since 0.0.1
 */

/**
 * @def P_OS_DARWIN64
 * @brief Darwin based 64-bit operating system.
 * @since 0.0.1
 */

/**
 * @def P_OS_BSD4
 * @brief BSD 4.x based operating system.
 * @since 0.0.1
 */

/**
 * @def P_OS_FREEBSD
 * @brief FreeBSD operating system.
 * @since 0.0.1
 */

/**
 * @def P_OS_DRAGONFLY
 * @brief DragonFlyBSD operating system.
 * @since 0.0.1
 */

/**
 * @def P_OS_NETBSD
 * @brief NetBSD operating system.
 * @since 0.0.1
 */

/**
 * @def P_OS_OPENBSD
 * @brief OpenBSD operating system.
 * @since 0.0.1
 */

/**
 * @def P_OS_AIX
 * @brief IBM AIX operating system.
 * @since 0.0.1
 */

/**
 * @def P_OS_HPUX
 * @brief HP-UX operating system.
 * @since 0.0.1
 */

/**
 * @def P_OS_TRU64
 * @brief Tru64 operating system.
 * @since 0.0.2
 */

/**
 * @def P_OS_SOLARIS
 * @brief Sun (Oracle) Solaris operating system.
 * @since 0.0.1
 */

/**
 * @def P_OS_QNX
 * @brief QNX 4.x operating system.
 * @since 0.0.1
 */

/**
 * @def P_OS_QNX6
 * @brief QNX Neutrino 6.x operating system.
 * @since 0.0.1
 */

/**
 * @def P_OS_SCO
 * @brief SCO OpenServer operating system.
 * @since 0.0.1
 */

/**
 * @def P_OS_UNIXWARE
 * @brief UnixWare 7 operating system.
 * @since 0.0.1
 */

/**
 * @def P_OS_IRIX
 * @brief SGI's IRIX operating system.
 * @since 0.0.1
 */

/**
 * @def P_OS_HAIKU
 * @brief Haiku operating system.
 * @since 0.0.1
 */

/**
 * @def P_OS_SYLLABLE
 * @brief Syllable operating system.
 * @since 0.0.2
 */

/**
 * @def P_OS_BEOS
 * @brief BeOS operating system.
 * @since 0.0.3
 */

/**
 * @def P_OS_OS2
 * @brief OS/2 operating system.
 * @since 0.0.3
 */

/**
 * @def P_OS_VMS
 * @brief OpenVMS operating system.
 * @since 0.0.1
 */

/**
 * @def P_OS_UNIX
 * @brief UNIX based operating system.
 * @since 0.0.1
 */

/**
 * @def P_OS_LINUX
 * @brief Linux based operating system.
 * @since 0.0.1
 */

/**
 * @def P_OS_MAC9
 * @brief Apple's Mac OS 9 operating system.
 * @since 0.0.1
 */

/**
 * @def P_OS_MAC
 * @brief Apple's macOS operating system.
 * @since 0.0.1
 */

/**
 * @def P_OS_MAC32
 * @brief Apple's macOS 32-bit operating system.
 * @since 0.0.1
 */

/**
 * @def P_OS_MAC64
 * @brief Apple's macOS 64-bit operating system.
 * @since 0.0.1
 */

/**
 * @def P_OS_CYGWIN
 * @brief Microsoft Windows POSIX runtime environment.
 * @since 0.0.1
 */

/**
 * @def P_OS_MSYS
 * @brief Microsoft Windows POSIX development environment.
 * @since 0.0.1
 */

/**
 * @def P_OS_WIN
 * @brief Microsoft Windows 32-bit operating system.
 * @since 0.0.1
 */

/**
 * @def P_OS_WIN64
 * @brief Microsoft Windows 64-bit operating system.
 * @since 0.0.1
 */

/* We need this to disable Doxygen warnings */

#ifdef DOXYGEN
#  define P_OS_DARWIN
#  define P_OS_DARWIN32
#  define P_OS_DARWIN64
#  define P_OS_BSD4
#  define P_OS_FREEBSD
#  define P_OS_DRAGONFLY
#  define P_OS_NETBSD
#  define P_OS_OPENBSD
#  define P_OS_AIX
#  define P_OS_HPUX
#  define P_OS_TRU64
#  define P_OS_SOLARIS
#  define P_OS_QNX
#  define P_OS_QNX6
#  define P_OS_SCO
#  define P_OS_UNIXWARE
#  define P_OS_IRIX
#  define P_OS_HAIKU
#  define P_OS_SYLLABLE
#  define P_OS_BEOS
#  define P_OS_OS2
#  define P_OS_VMS
#  define P_OS_UNIX
#  define P_OS_LINUX
#  define P_OS_MAC9
#  define P_OS_MAC
#  define P_OS_MAC32
#  define P_OS_MAC64
#  define P_OS_CYGWIN
#  define P_OS_MSYS
#  define P_OS_WIN
#  define P_OS_WIN64
#endif

#if defined(__APPLE__) && (defined(__GNUC__) || defined(__xlC__) || defined(__xlc__))
#  define P_OS_DARWIN
#  define P_OS_BSD4
#  ifdef __LP64__
#    define P_OS_DARWIN64
#  else
#    define P_OS_DARWIN32
#  endif
# elif defined(Macintosh) || defined(macintosh)
#  define P_OS_MAC9
#elif defined(__MSYS__)
#  define P_OS_MSYS
#elif defined(__CYGWIN__)
#  define P_OS_CYGWIN
#elif defined(_WIN64) || defined(_M_X64) || defined(_M_AMD64)
#  define P_OS_WIN64
#elif defined(__WIN32__) || defined(_WIN32) || defined(WIN32)
#  define P_OS_WIN
#elif defined(__linux) || defined(__linux__)
#  define P_OS_LINUX
#elif defined(__FreeBSD__)
#  define P_OS_FREEBSD
#  define P_OS_BSD4
#elif defined(__DragonFly__)
#  define P_OS_DRAGONFLY
#  define P_OS_BSD4
#elif defined(__NetBSD__)
#  define P_OS_NETBSD
#  define P_OS_BSD4
#elif defined(__OpenBSD__)
#  define P_OS_OPENBSD
#  define P_OS_BSD4
#elif defined(_AIX)
#  define P_OS_AIX
#elif defined(hpux) || defined(__hpux)
#  define P_OS_HPUX
#elif defined(__osf__) || defined(__osf)
#  define P_OS_TRU64
#elif defined(__sun) || defined(sun)
#  define P_OS_SOLARIS
#elif defined(__QNXNTO__)
#  define P_OS_QNX6
#elif defined(__QNX__)
#  define P_OS_QNX
#elif defined(_SCO_DS)
#  define P_OS_SCO
#elif defined(__USLC__) || defined(__UNIXWARE__)
#  define P_OS_UNIXWARE
#elif defined(__svr4__) && defined(i386)
#  define P_OS_UNIXWARE
#elif defined(__sgi) || defined(sgi)
#  define P_OS_IRIX
#elif defined(__HAIKU__)
#  define P_OS_HAIKU
#elif defined(__SYLLABLE__)
#  define P_OS_SYLLABLE
#elif defined(__BEOS__)
#  define P_OS_BEOS
#elif defined(__OS2__)
#  define P_OS_OS2
#elif defined(VMS) || defined(__VMS)
#  define P_OS_VMS
#endif

#ifdef P_OS_WIN64
#  define P_OS_WIN
#endif

#if defined(P_OS_DARWIN)
#  define P_OS_MAC
#  if defined(P_OS_DARWIN64)
#     define P_OS_MAC64
#  elif defined(P_OS_DARWIN32)
#     define P_OS_MAC32
#  endif
#endif

#if defined(P_OS_WIN)  || defined(P_OS_MAC9) || defined(P_OS_HAIKU) || \
    defined(P_OS_BEOS) || defined(P_OS_OS2)  || defined(P_OS_VMS)
#  undef P_OS_UNIX
#elif !defined(P_OS_UNIX)
#  define P_OS_UNIX
#endif

/*
 * List of supported compilers (P_CC_x):
 *
 * MSVC            - Microsoft Visual C/C++
 * GNU             - GNU C/C++
 * MINGW           - MinGW C/C++
 * INTEL           - Intel C/C++
 * CLANG           - LLVM Clang C/C++
 * SUN             - Sun WorkShop/Studio C/C++
 * XLC             - IBM XL C/C++
 * HP              - HP C/aC++
 * DEC             - DEC C/C++
 * MIPS            - MIPSpro C/C++
 * USLC            - SCO OUDK and UDK C/C++
 * WATCOM          - Watcom C/C++
 * BORLAND         - Borland C/C++
 * PGI             - Portland Group C/C++
 */

/**
 * @def P_CC_MSVC
 * @brief Microsoft Visual C/C++ compiler.
 * @since 0.0.1
 */

/**
 * @def P_CC_GNU
 * @brief GNU C/C++ compiler.
 * @since 0.0.1
 */

/**
 * @def P_CC_MINGW
 * @brief MinGW C/C++ compiler.
 * @since 0.0.1
 */

/**
 * @def P_CC_INTEL
 * @brief Intel C/C++ compiler.
 * @since 0.0.1
 */

/**
 * @def P_CC_CLANG
 * @brief LLVM Clang C/C++ compiler.
 * @since 0.0.1
 */

/**
 * @def P_CC_SUN
 * @brief Sun WorkShop/Studio C/C++ compiler.
 * @since 0.0.1
 */

/**
 * @def P_CC_XLC
 * @brief IBM XL C/C++ compiler.
 * @since 0.0.1
 */

/**
 * @def P_CC_HP
 * @brief HP C/aC++ compiler.
 * @since 0.0.1
 */

/**
 * @def P_CC_DEC
 * @brief DEC C/C++ compiler.
 * @since 0.0.2
 */

/**
 * @def P_CC_MIPS
 * @brief MIPSpro C/C++ compiler.
 * @since 0.0.1
 */

/**
 * @def P_CC_USLC
 * @brief SCO OUDK and UDK C/C++ compiler.
 * @since 0.0.1
 */

/**
 * @def P_CC_WATCOM
 * @brief Watcom C/C++ compiler.
 * @since 0.0.1
 */

/**
 * @def P_CC_BORLAND
 * @brief Borland C/C++ compiler.
 * @since 0.0.1
 */

/**
 * @def P_CC_PGI
 * @brief Portland Group C/C++ compiler.
 * @since 0.0.3
 */

#ifdef DOXYGEN
#  define P_CC_MSVC
#  define P_CC_GNU
#  define P_CC_MINGW
#  define P_CC_INTEL
#  define P_CC_CLANG
#  define P_CC_SUN
#  define P_CC_XLC
#  define P_CC_HP
#  define P_CC_DEC
#  define P_CC_MIPS
#  define P_CC_USLC
#  define P_CC_WATCOM
#  define P_CC_BORLAND
#  define P_CC_PGI
#endif

#if defined(_MSC_VER)
#  define P_CC_MSVC
#  if defined(__INTEL_COMPILER)
#    define P_CC_INTEL
#  endif
#  if defined(__clang__)
#    define P_CC_CLANG
#  endif
#elif defined(__GNUC__)
#  define P_CC_GNU
#  if defined(__MINGW32__)
#    define P_CC_MINGW
#  endif
#  if defined(__INTEL_COMPILER)
#    define P_CC_INTEL
#  endif
#  if defined(__clang__)
#    define P_CC_CLANG
#  endif
#elif defined(__SUNPRO_C) || defined(__SUNPRO_CC)
#  define P_CC_SUN
#elif defined(__xlc__) || defined(__xlC__)
#  define P_CC_XLC
#elif defined(__HP_cc) || defined(__HP_aCC)
#  define P_CC_HP
#elif defined (__DECC) || defined(__DECCXX)
#  define P_CC_DEC
#elif (defined(__sgi) || defined(sgi)) && \
      (defined(_COMPILER_VERSION) || defined(_SGI_COMPILER_VERSION))
#  define P_CC_MIPS
#elif defined(__USLC__) && defined(__SCO_VERSION__)
#  define P_CC_USLC
#elif defined(__WATCOMC__)
#  define P_CC_WATCOM
#elif defined(__BORLANDC__)
#  define P_CC_BORLAND
#elif defined(__INTEL_COMPILER)
#  define P_CC_INTEL
#elif defined(__PGI)
#  define P_CC_PGI
#endif

/*
 * List of supported CPU architectures (P_CPU_x):
 *
 * ALPHA           - Alpha
 * ARM             - ARM architecture revision:
 *                   v2, v3, v4, v5, v6, v7, v8
 * ARM_32          - ARM 32-bit
 * ARM_64          - ARM 64-bit
 * ARM_V2          - ARMv2 instruction set
 * ARM_V3          - ARMv3 instruction set
 * ARM_V4          - ARMv4 instruction set
 * ARM_V5          - ARMv5 instruction set
 * ARM_V6          - ARMv6 instruction set
 * ARM_V7          - ARMv7 instruction set
 * ARM_V8          - ARMv8 instruction set
 * X86             - x86 architecture revision:
 *                   3, 4, 5, 6 (Intel P6 or better)
 * X86_32          - x86 32-bit
 * X86_64          - x86 64-bit
 * IA64            - Intel Itanium (IA-64)
 * MIPS            - MIPS
 * MIPS_I          - MIPS I
 * MIPS_II         - MIPS II
 * MIPS_III        - MIPS III
 * MIPS_IV         - MIPS IV
 * MIPS_32         - MIPS32
 * MIPS_64         - MIPS64
 * POWER           - PowerPC
 * POWER_32        - PowerPC 32-bit
 * POWER_64        - PowerPC 64-bit
 * SPARC           - Sparc
 * SPARC_V8        - Sparc V8
 * SPARC_V9        - Sparc V9
 * HPPA            - HPPA-RISC
 * HPPA_32         - HPPA-RISC 32-bit
 * HPPA_64         - HPPA-RISC 64-bit
 */

/**
 * @def P_CPU_ALPHA
 * @brief DEC Alpha architecture.
 * @since 0.0.3
 */

/**
 * @def P_CPU_ARM
 * @brief ARM architecture.
 * @since 0.0.3
 *
 * This macro is defined for any ARM target. It contains an architecture
 * revision number. One of the revision specific macros (P_CPU_ARM_Vx) is also
 * defined, as well as #P_CPU_ARM_32 or #P_CPU_ARM_64.
 */

/**
 * @def P_CPU_ARM_32
 * @brief ARM 32-bit architecture.
 * @since 0.0.3
 *
 * This macro is defined for ARM 32-bit target. One of the revision specific
 * macros (P_CPU_ARM_Vx) is also defined, as well as #P_CPU_ARM.
 */

/**
 * @def P_CPU_ARM_64
 * @brief ARM 64-bit architecture.
 * @since 0.0.3
 *
 * This macro is defined for ARM 64-bit target. One of the revision specific
 * macros (P_CPU_ARM_Vx) is also defined, as well as #P_CPU_ARM.
 */

/**
 * @def P_CPU_ARM_V2
 * @brief ARMv2 architecture revision.
 * @since 0.0.3
 *
 * This macro is defined for ARMv2 target. #P_CPU_ARM_32 and #P_CPU_ARM macros
 * are also defined.
 */

/**
 * @def P_CPU_ARM_V3
 * @brief ARMv3 architecture revision.
 * @since 0.0.3
 *
 * This macro is defined for ARMv3 target. #P_CPU_ARM_32 and #P_CPU_ARM macros
 * are also defined.
 */

/**
 * @def P_CPU_ARM_V4
 * @brief ARMv4 architecture revision.
 * @since 0.0.3
 *
 * This macro is defined for ARMv4 target. #P_CPU_ARM_32 and #P_CPU_ARM macros
 * are also defined.
 */

/**
 * @def P_CPU_ARM_V5
 * @brief ARMv5 architecture revision.
 * @since 0.0.3
 *
 * This macro is defined for ARMv5 target. #P_CPU_ARM_32 and #P_CPU_ARM macros
 * are also defined.
 */

/**
 * @def P_CPU_ARM_V6
 * @brief ARMv6 architecture revision.
 * @since 0.0.3
 *
 * This macro is defined for ARMv6 target. #P_CPU_ARM_32 and #P_CPU_ARM macros
 * are also defined.
 */

/**
 * @def P_CPU_ARM_V7
 * @brief ARMv7 architecture revision.
 * @since 0.0.3
 *
 * This macro is defined for ARMv7 target. #P_CPU_ARM_32 and #P_CPU_ARM macros
 * are also defined.
 */

/**
 * @def P_CPU_ARM_V8
 * @brief ARMv8 architecture revision.
 * @since 0.0.3
 *
 * This macro is defined for ARMv8 target. #P_CPU_ARM_32 or #P_CPU_ARM_64 macro
 * is defined, as well as #P_CPU_ARM.
 */

/**
 * @def P_CPU_X86
 * @brief Intel x86 architecture.
 * @since 0.0.3
 *
 * This macro is defined for any x86 target. It contains an architecture
 * revision number (3 for i386 and lower, 4 for i486, 5 for i586, 6 for i686 and
 * better). One of the architecture specific macros (P_CPU_X86_xx) is also
 * defined.
 */

/**
 * @def P_CPU_X86_32
 * @brief Intel x86 32-bit architecture.
 * @since 0.0.3
 *
 * This macro is defined for x86 32-bit target. #P_CPU_X86 macro is also
 * defined.
 */

/**
 * @def P_CPU_X86_64
 * @brief Intel x86 64-bit architecture.
 * @since 0.0.3
 *
 * This macro is defined for x86 64-bit target. #P_CPU_X86 macro is also
 * defined.
 */

/**
 * @def P_CPU_IA64
 * @brief Intel Itanium (IA-64) architecture.
 * @since 0.0.3
 *
 * This macro is defined for Intel Itanium (IA-64) target.
 */

/**
 * @def P_CPU_MIPS
 * @brief MIPS architecture.
 * @since 0.0.3
 *
 * This macro is defined for any MIPS target. Some other specific macros
 * (P_CPU_MIPS_xx) for different MIPS ISAs may be defined.
 */

/**
 * @def P_CPU_MIPS_I
 * @brief MIPS I ISA.
 * @since 0.0.3
 *
 * This macro is defined for MIPS I target. #P_CPU_MIPS is also defined, as well
 * as probably some other ISA macros (P_CPU_MIPS_xx).
 */

/**
 * @def P_CPU_MIPS_II
 * @brief MIPS II ISA.
 * @since 0.0.3
 *
 * This macro is defined for MIPS II target. #P_CPU_MIPS and #P_CPU_MIPS_I are
 * also defined, as well as probably some other ISA macros (P_CPU_MIPS_xx).
 */

/**
 * @def P_CPU_MIPS_III
 * @brief MIPS III ISA.
 * @since 0.0.3
 *
 * This macro is defined for MIPS III target. #P_CPU_MIPS, #P_CPU_MIPS_I and
 * #P_CPU_MIPS_II are also defined, as well as probably some other ISA macros
 * (P_CPU_MIPS_xx).
 */

/**
 * @def P_CPU_MIPS_IV
 * @brief MIPS IV ISA.
 * @since 0.0.3
 *
 * This macro is defined for MIPS IV target. #P_CPU_MIPS, #P_CPU_MIPS_I,
 * #P_CPU_MIPS_II and #P_CPU_MIPS_III are also defined, as well as probably some
 * other ISA macros (P_CPU_MIPS_xx).
 */

/**
 * @def P_CPU_MIPS_32
 * @brief MIPS32 ISA.
 * @since 0.0.3
 *
 * This macro is defined for MIPS32 target. #P_CPU_MIPS, #P_CPU_MIPS_I and
 * #P_CPU_MIPS_II.
 */

/**
 * @def P_CPU_MIPS_64
 * @brief MIPS64 ISA.
 * @since 0.0.3
 *
 * This macro is defined for MIPS64 target. #P_CPU_MIPS, #P_CPU_MIPS_I,
 * #P_CPU_MIPS_II, #P_CPU_MIPS_III, #P_CPU_MIPS_IV and are also defined.
 */

/**
 * @def P_CPU_POWER
 * @brief PowerPC architecture.
 * @since 0.0.3
 *
 * This macro is defined for any PowerPC target. One of the architecture
 * specific macros (P_CPU_POWER_xx) is also defined.
 */

/**
 * @def P_CPU_POWER_32
 * @brief PowerPC 32-bit architecture.
 * @since 0.0.3
 *
 * This macro is defined for PowerPC 32-bit target. #P_CPU_POWER macro is also
 * defined.
 */

/**
 * @def P_CPU_POWER_64
 * @brief PowerPC 64-bit architecture.
 * @since 0.0.3
 *
 * This macro is defined for PowerPC 64-bit target. #P_CPU_POWER macro is also
 * defined.
 */

/**
 * @def P_CPU_SPARC
 * @brief Sun SPARC architecture.
 * @since 0.0.3
 *
 * This macro is defined for any SPARC target. One of the architecture
 * specific macros (P_CPU_SPARC_xx) is also may be defined.
 */

/**
 * @def P_CPU_SPARC_V8
 * @brief Sun SPARC V8 architecture.
 * @since 0.0.3
 *
 * This macro is defined for SPARC V8 target. #P_CPU_SPARC macro is also
 * defined.
 */

/**
 * @def P_CPU_SPARC_V9
 * @brief Sun SPARC V9 architecture.
 * @since 0.0.3
 *
 * This macro is defined for SPARC V9 target. #P_CPU_SPARC macro is also
 * defined.
 */

/**
 * @def P_CPU_HPPA
 * @brief HP PA-RISC architecture.
 * @since 0.0.3
 *
 * This macro is defined for any PA-RISC target. One of the architecture
 * specific macros (P_CPU_HPPA_xx) is also defined.
 */

/**
 * @def P_CPU_HPPA_32
 * @brief HP PA-RISC 32-bit (1.0, 1.1) architecture.
 * @since 0.0.3
 *
 * This macro is defined for PA-RISC 32-bit target. #P_CPU_HPPA macro is also
 * defined.
 */

/**
 * @def P_CPU_HPPA_64
 * @brief HP PA-RISC 64-bit (2.0) architecture.
 * @since 0.0.3
 *
 * This macro is defined for PA-RISC 64-bit target. #P_CPU_HPPA macro is also
 * defined.
 */

#ifdef DOXYGEN
#  define P_CPU_ALPHA
#  define P_CPU_ARM
#  define P_CPU_ARM_32
#  define P_CPU_ARM_64
#  define P_CPU_ARM_V2
#  define P_CPU_ARM_V3
#  define P_CPU_ARM_V4
#  define P_CPU_ARM_V5
#  define P_CPU_ARM_V6
#  define P_CPU_ARM_V7
#  define P_CPU_ARM_V8
#  define P_CPU_X86
#  define P_CPU_X86_32
#  define P_CPU_X86_64
#  define P_CPU_IA64
#  define P_CPU_MIPS
#  define P_CPU_MIPS_I
#  define P_CPU_MIPS_II
#  define P_CPU_MIPS_III
#  define P_CPU_MIPS_IV
#  define P_CPU_MIPS_32
#  define P_CPU_MIPS_64
#  define P_CPU_POWER
#  define P_CPU_POWER_32
#  define P_CPU_POWER_64
#  define P_CPU_SPARC
#  define P_CPU_SPARC_V8
#  define P_CPU_SPARC_V9
#  define P_CPU_HPPA
#  define P_CPU_HPPA_32
#  define P_CPU_HPPA_64
#endif

#if defined(__alpha__) || defined(__alpha) || defined(_M_ALPHA)
#  define P_CPU_ALPHA
#elif defined(__arm__) || defined(__TARGET_ARCH_ARM) || defined(_ARM) || \
      defined(_M_ARM_) || defined(__arm) || defined(__aarch64__)
#  if defined(__aarch64__)
#    define P_CPU_ARM_64
#  else
#    define P_CPU_ARM_32
#  endif
#  if defined(__ARM_ARCH) && __ARM_ARCH > 1
#    define P_CPU_ARM __ARM_ARCH
#  elif defined(__TARGET_ARCH_ARM) && __TARGET_ARCH_ARM > 1
#    define P_CPU_ARM __TARGET_ARCH_ARM
#  elif defined(_M_ARM) && _M_ARM > 1
#    define P_CPU_ARM _M_ARM
#  elif defined(__ARM64_ARCH_8__)  || \
        defined(__aarch64__)       || \
        defined(__CORE_CORTEXAV8__)
#    define P_CPU_ARM 8
#    define P_CPU_ARM_V8
#  elif defined(__ARM_ARCH_7__)    || \
        defined(__ARM_ARCH_7A__)   || \
        defined(__ARM_ARCH_7R__)   || \
        defined(__ARM_ARCH_7M__)   || \
        defined(__ARM_ARCH_7S__)   || \
        defined(_ARM_ARCH_7)       || \
        defined(__CORE_CORTEXA__)
#    define P_CPU_ARM 7
#    define P_CPU_ARM_V7
#  elif defined(__ARM_ARCH_6__)    || \
        defined(__ARM_ARCH_6J__)   || \
        defined(__ARM_ARCH_6T2__)  || \
        defined(__ARM_ARCH_6Z__)   || \
        defined(__ARM_ARCH_6K__)   || \
        defined(__ARM_ARCH_6ZK__)  || \
        defined(__ARM_ARCH_6M__)
#    define P_CPU_ARM 6
#    define P_CPU_ARM_V6
#  elif defined(__ARM_ARCH_5__)    || \
        defined(__ARM_ARCH_5E__)   || \
        defined(__ARM_ARCH_5T__)   || \
        defined(__ARM_ARCH_5TE__)  || \
        defined(__ARM_ARCH_5TEJ__)
#    define P_CPU_ARM 5
#    define P_CPU_ARM_V5
#  elif defined(__ARM_ARCH_4__)    || \
        defined(__ARM_ARCH_4T__)
#    define P_CPU_ARM 4
#    define P_CPU_ARM_V4
#  elif defined(__ARM_ARCH_3__)    || \
        defined(__ARM_ARCH_3M__)
#    define P_CPU_ARM 3
#    define P_CPU_ARM_V3
#  elif defined(__ARM_ARCH_2__)
#    define P_CPU_ARM 2
#    define P_CPU_ARM_V2
#  endif
#elif defined(__i386__) || defined(__i386) || defined(_M_IX86)
#  define P_CPU_X86_32
#  if defined(_M_IX86)
#    define P_CPU_X86 (_M_IX86 / 100)
#  elif defined(__i686__) || defined(__athlon__) || defined(__SSE__) || defined(__pentiumpro__)
#    define P_CPU_X86 6
#  elif defined(__i586__) || defined(__k6__) || defined(__pentium__)
#    define P_CPU_X86 5
#  elif defined(__i486__) || defined(__80486__)
#    define P_CPU_X86 4
#  else
#    define P_CPU_X86 3
#  endif
#elif defined(__x86_64__) || defined(__x86_64) || \
      defined(__amd64__)  || defined(__amd64)  || \
      defined(_M_X64)     || defined(_M_AMD64)
#  define P_CPU_X86_64
#  define P_CPU_X86 6
#elif defined(__ia64__) || defined(__ia64) || defined(_M_IA64)
#  define P_CPU_IA64
#elif defined(__mips__) || defined(__mips) || defined(_M_MRX000)
#  define P_CPU_MIPS
#  if defined(_M_MRX000)
#    if (_M_MRX000 >= 10000)
#      define P_CPU_MIPS_IV
#    else
#      define P_CPU_MIPS_III
#    endif
#  endif
#  if defined(_MIPS_ARCH_MIPS64) || (defined(__mips) && __mips - 0 >= 64) || \
     (defined(_MIPS_ISA) && defined(_MIPS_ISA_MIPS64) && __MIPS_ISA - 0 >= _MIPS_ISA_MIPS64)
#    define P_CPU_MIPS_64
#  elif defined(_MIPS_ARCH_MIPS32) || (defined(__mips) && __mips - 0 >= 32) || \
       (defined(_MIPS_ISA) && defined(_MIPS_ISA_MIPS32) && __MIPS_ISA - 0 >= _MIPS_ISA_MIPS32)
#    define P_CPU_MIPS_32
#  elif defined(_MIPS_ARCH_MIPS4) || (defined(__mips) && __mips - 0 >= 4) || \
       (defined(_MIPS_ISA) && defined(_MIPS_ISA_MIPS4) && __MIPS_ISA - 0 >= _MIPS_ISA_MIPS4)
#    define P_CPU_MIPS_IV
#  elif defined(_MIPS_ARCH_MIPS3) || (defined(__mips) && __mips - 0 >= 3) || \
       (defined(_MIPS_ISA)&& defined(_MIPS_ISA_MIPS3) && __MIPS_ISA - 0 >= _MIPS_ISA_MIPS3)
#    define P_CPU_MIPS_III
#  elif defined(_MIPS_ARCH_MIPS2) || (defined(__mips) && __mips - 0 >= 2) || \
       (defined(_MIPS_ISA) && defined(_MIPS_ISA_MIPS2) && __MIPS_ISA - 0 >= _MIPS_ISA_MIPS2)
#    define P_CPU_MIPS_II
#  elif defined(_MIPS_ARCH_MIPS1) || (defined(__mips) && __mips - 0 >= 1) || \
     (defined(_MIPS_ISA) && defined(_MIPS_ISA_MIPS1) && __MIPS_ISA - 0 >= _MIPS_ISA_MIPS1)
#    define P_CPU_MIPS_I
#  endif
#  if defined(P_CPU_MIPS_64)
#    define P_CPU_MIPS_IV
#  endif
#  if defined(P_CPU_MIPS_IV)
#    define P_CPU_MIPS_III
#  endif
#  if defined(P_CPU_MIPS_32) || defined(P_CPU_MIPS_III)
#    define P_CPU_MIPS_II
#  endif
#  if defined(P_CPU_MIPS_II)
#    define P_CPU_MIPS_I
#  endif
#elif defined(__powerpc__) || defined(__powerpc) || defined(__ppc__)   || defined(__ppc) || \
      defined(_ARCH_PPC)   || defined(_ARCH_PWR) || defined(_ARCH_COM) || \
      defined(_M_PPC)      || defined(_M_MPPC)
#  define P_CPU_POWER
#  if defined(__powerpc64__) || defined(__powerpc64) || defined(__ppc64__) || defined(__ppc64) || \
      defined(__64BIT__)     || defined(__LP64__)    || defined(_LP64)
#    define P_CPU_POWER_64
#  else
#    define P_CPU_POWER_32
#  endif
#elif defined(__sparc__) || defined(__sparc)
#  define P_CPU_SPARC
#  if defined(__sparc_v9__) || defined(__sparcv9)
#    define P_CPU_SPARC_V9
#  elif defined(__sparc_v8__) || defined(__sparcv8)
#    define P_CPU_SPARC_V8
#  endif
#elif defined(__hppa__) || defined(__hppa)
#  define P_CPU_HPPA
#  if defined(_PA_RISC2_0) || defined(__RISC2_0__) || defined(__HPPA20__) || defined(__PA8000__)
#    define P_CPU_HPPA_64
#  else
#    define P_CPU_HPPA_32
#  endif
#endif

/* For Clang */
#ifndef __has_attribute
#  define __has_attribute(x) 0
#endif

#ifndef __has_builtin
#  define __has_builtin(x) 0
#endif

/**
 * @def P_GNUC_WARN_UNUSED_RESULT
 * @brief Gives a warning if the result returned from a function is not being
 * used.
 * @since 0.0.1
 */

#if (__GNUC__ > 3 || (__GNUC__ == 3 && __GNUC_MINOR__ >= 4)) || \
    __has_attribute(warn_unused_result)
#  define P_GNUC_WARN_UNUSED_RESULT __attribute__((warn_unused_result))
#else
#  define P_GNUC_WARN_UNUSED_RESULT
#endif

/**
 * @def P_LIB_API
 * @brief Exports a symbol from a shared library.
 * @since 0.0.1
 */

#if defined(P_CC_MSVC) || defined(P_CC_BORLAND) || defined(P_CC_WATCOM) || \
    defined(P_OS_OS2)  || (defined(P_OS_BEOS) && !defined(P_CC_GNU))
#  define P_LIB_API __declspec(dllexport)
#else
#  define P_LIB_API
#endif

/* Oracle Solaris Studio at least since 12.2 has ((noreturn)) attribute */

/**
 * @def P_NO_RETURN
 * @brief Notifies a compiler that a function will never return a value (i.e.
 * due to the abort () call).
 * @since 0.0.1
 */

#if defined (__STDC_VERSION__) && (__STDC_VERSION__ >= 201112L)
#  define P_NO_RETURN _Noreturn
#elif defined(P_CC_MSVC) || (defined(P_CC_BORLAND) && __BORLANDC__ >= 0x0550)
#  define P_NO_RETURN __declspec(noreturn)
#elif __has_attribute(noreturn) || \
      defined(P_CC_GNU) || \
     (defined(P_CC_SUN) && __SUNPRO_C >= 0x5110) || \
     (defined(P_CC_SUN) && __SUNPRO_CC >= 0x5110)
#  define P_NO_RETURN __attribute__((noreturn))
#else
#  define P_NO_RETURN
#endif

/**
 * @def P_LIKELY
 * @brief Hints a compiler that a condition is likely to be true so it can
 * perform code optimizations.
 * @since 0.0.1
 */

/**
 * @def P_UNLIKELY
 * @brief Hints a compiler that a condition is likely to be false so it can
 * perform code optimizations.
 * @since 0.0.1
 */

#if (defined(P_CC_GNU) && (__GNUC__ > 2 && __GNUC_MINOR__ > 0)) || \
    (defined(P_CC_INTEL) && __INTEL_COMPILER >= 800) || \
    (defined(P_CC_XLC) && __xlC__ >= 0x0900) || \
    __has_builtin(__builtin_expect)
#  define P_LIKELY(x) __builtin_expect(!!(x), 1)
#  define P_UNLIKELY(x) __builtin_expect(!!(x), 0)
#else
#  define P_LIKELY(x) (x)
#  define P_UNLIKELY(x) (x)
#endif

/**
 * @def P_UNUSED
 * @brief Macro to by-pass a compiler warning on unused variables.
 * @since 0.0.1
 */
#define P_UNUSED(a) ((void) a)

/**
 * @def P_WARNING
 * @brief Prints a warning message.
 * @param msg Message to print.
 * @since 0.0.1
 */
#define P_WARNING(msg) printf ("** Warning: %s **\n", msg)

/**
 * @def P_ERROR
 * @brief Prints an error message.
 * @param msg Message to print.
 * @since 0.0.1
 */
#define P_ERROR(msg) printf ("** Error: %s **\n", msg)

/**
 * @def P_DEBUG
 * @brief Prints a debug message.
 * @param msg Message to print.
 * @since 0.0.1
 */
#define P_DEBUG(msg) printf ("** Debug: %s **\n", msg)

#ifdef DOXYGEN
#  define PLIBSYS_VERSION_MAJOR
#  define PLIBSYS_VERSION_MINOR
#  define PLIBSYS_VERSION_PATCH
#  define PLIBSYS_VERSION_STR
#  define PLIBSYS_VERSION
#endif

/**
 * @def PLIBSYS_VERSION_MAJOR
 * @brief Library major version number.
 * @since 0.0.1
 * @note This is the version against which the application is compiled.
 */

/**
 * @def PLIBSYS_VERSION_MINOR
 * @brief Library minor version number.
 * @since 0.0.1
 * @note This is the version against which the application is compiled.
 */

/**
 * @def PLIBSYS_VERSION_PATCH
 * @brief Library patch version number.
 * @since 0.0.1
 * @note This is the version against which the application is compiled.
 */

/**
 * @def PLIBSYS_VERSION_STR
 * @brief Library full version in the string form, i.e. "0.0.1".
 * @since 0.0.1
 * @note This is the version against which the application is compiled.
 * @sa p_libsys_version()
 */

/**
 * @def PLIBSYS_VERSION
 * @brief Library full version in the form 0xMMNNPP (MM = major, NN = minor,
 * PP = patch), i.e. 0x000001.
 * @since 0.0.1
 * @note This is the version against which the application is compiled.
 * @sa p_libsys_version()
 */

/**
 * @def PLIBSYS_VERSION_CHECK
 * @brief Makes a library version number which can be used to check the library
 * version against which the application is compiled.
 * @param major Major version number to check.
 * @param minor Minor version number to check.
 * @param patch Minor version number to check.
 * @since 0.0.1
 * @sa p_libsys_version()
 *
 * @code
 * if (PLIBSYS_VERSION >= PLIBSYS_VERSION_CHECK (0, 0, 1))
 * ...
 * @endcode
 */
#define PLIBSYS_VERSION_CHECK(major, minor, patch) ((major << 16) | (minor << 8) | (patch))

/**
 * @def P_BEGIN_DECLS
 * @brief Starts .h file declarations to be exported as C functions.
 * @since 0.0.1
 */

/**
 * @def P_END_DECLS
 * @brief Closes .h file declarations to be exported as C functions, should be
 * always used after #P_BEGIN_DECLS.
 * @since 0.0.1
 */

#ifdef __cplusplus
#  define P_BEGIN_DECLS extern "C" {
#  define P_END_DECLS }
#else
#  define P_BEGIN_DECLS
#  define P_END_DECLS
#endif

#endif /* PLIBSYS_HEADER_PMACROS_H */
