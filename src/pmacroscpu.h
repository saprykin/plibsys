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
 * @file pmacroscpu.h
 * @brief CPU detection macros
 * @author Alexander Saprykin
 *
 * All the macros are completely independent of any other platform-specific
 * headers, thus gurantee to work with any compiler under any operating system
 * in the same way as they are used within the library.
 *
 * This family of macros provides CPU detection and defines one or several of
 * P_CPU_x macros.
 */

#if !defined (PLIBSYS_H_INSIDE) && !defined (PLIBSYS_COMPILATION)
#  error "Header files shouldn't be included directly, consider using <plibsys.h> instead."
#endif

#ifndef PLIBSYS_HEADER_PMACROSCPU_H
#define PLIBSYS_HEADER_PMACROSCPU_H

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
#    if (_M_IX86 >= 300 &&_M_IX86 <= 600)
#      define P_CPU_X86 (_M_IX86 / 100)
#    else
#      define P_CPU_X86 6
#    endif
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

/* We need this to generate full Doxygen documentation */

#ifdef DOXYGEN
#  ifndef P_CPU_ALPHA
#    define P_CPU_ALPHA
#  endif
#  ifndef P_CPU_ARM
#    define P_CPU_ARM
#  endif
#  ifndef P_CPU_ARM_32
#    define P_CPU_ARM_32
#  endif
#  ifndef P_CPU_ARM_64
#    define P_CPU_ARM_64
#  endif
#  ifndef P_CPU_ARM_V2
#    define P_CPU_ARM_V2
#  endif
#  ifndef P_CPU_ARM_V3
#    define P_CPU_ARM_V3
#  endif
#  ifndef P_CPU_ARM_V4
#    define P_CPU_ARM_V4
#  endif
#  ifndef P_CPU_ARM_V5
#    define P_CPU_ARM_V5
#  endif
#  ifndef P_CPU_ARM_V6
#    define P_CPU_ARM_V6
#  endif
#  ifndef P_CPU_ARM_V7
#    define P_CPU_ARM_V7
#  endif
#  ifndef P_CPU_ARM_V8
#    define P_CPU_ARM_V8
#  endif
#  ifndef P_CPU_X86
#    define P_CPU_X86
#  endif
#  ifndef P_CPU_X86_32
#    define P_CPU_X86_32
#  endif
#  ifndef P_CPU_X86_64
#    define P_CPU_X86_64
#  endif
#  ifndef P_CPU_IA64
#    define P_CPU_IA64
#  endif
#  ifndef P_CPU_MIPS
#    define P_CPU_MIPS
#  endif
#  ifndef P_CPU_MIPS_I
#    define P_CPU_MIPS_I
#  endif
#  ifndef P_CPU_MIPS_II
#    define P_CPU_MIPS_II
#  endif
#  ifndef P_CPU_MIPS_III
#    define P_CPU_MIPS_III
#  endif
#  ifndef P_CPU_MIPS_IV
#    define P_CPU_MIPS_IV
#  endif
#  ifndef P_CPU_MIPS_32
#    define P_CPU_MIPS_32
#  endif
#  ifndef P_CPU_MIPS_64
#    define P_CPU_MIPS_64
#  endif
#  ifndef P_CPU_POWER
#    define P_CPU_POWER
#  endif
#  ifndef P_CPU_POWER_32
#    define P_CPU_POWER_32
#  endif
#  ifndef P_CPU_POWER_64
#    define P_CPU_POWER_64
#  endif
#  ifndef P_CPU_SPARC
#    define P_CPU_SPARC
#  endif
#  ifndef P_CPU_SPARC_V8
#    define P_CPU_SPARC_V8
#  endif
#  ifndef P_CPU_SPARC_V9
#    define P_CPU_SPARC_V9
#  endif
#  ifndef P_CPU_HPPA
#    define P_CPU_HPPA
#  endif
#  ifndef P_CPU_HPPA_32
#    define P_CPU_HPPA_32
#  endif
#  ifndef P_CPU_HPPA_64
#    define P_CPU_HPPA_64
#  endif
#endif

#endif /* PLIBSYS_HEADER_PMACROSCPU_H */
