/* 
 * 23.08.2010
 * Copyright (C) 2010 Alexander Saprykin <xelfium@gmail.com>
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

#if !defined (__PLIB_H_INSIDE__) && !defined (PLIB_COMPILATION)
#  error "Header files shouldn't be included directly, consider using <plib.h> instead."
#endif

#ifndef __PMACROS_H__
#define __PMACROS_H__

#include <stdio.h>

/* Operating systems */

/* Microsoft Windows */
#if defined(__WIN32__) || defined(_WIN32) || defined(WIN32)
#  define P_OS_WIN
#elif defined(_WIN64) || defined(_M_X64) || defined(_M_AMD64)
#  define P_OS_WIN64
/* Linux */
#elif defined(__linux) || defined(__linux__)
#  define P_OS_LINUX
/* Sun Solaris */
#elif defined(__sun) || defined(sun)
#  define P_OS_SOLARIS
/* QNX6 */
#elif defined(__QNXNTO__)
#  define P_OS_QNX6
/* QNX */
#elif defined(__QNX__)
#  define P_OS_QNX
#endif

#ifdef P_OS_WIN64
#  define P_OS_WIN
#endif

/* Compilers */

/* Microsoft Visual C/C++ */
#if defined(_MSC_VER)
# define P_CC_MSVC
# if defined(__INTEL_COMPILER)
#   define P_CC_INTEL
# endif
/* GNU C */
#elif defined(__GNUC__)
#  define P_CC_GNU
#  if defined(__MINGW32__)
#    define P_CC_MINGW
#  endif
#  if defined(__INTEL_COMPILER)
#    define P_CC_INTEL
#  endif
/* Sun WorkShop/Studio */
#elif defined(__SUNPRO_C)
#  define P_CC_SUN
/* Watcom C++ */
#elif defined(__WATCOMC__)
#  define P_CC_WATCOM
/* Intel C/C++ */
#elif defined(__INTEL_COMPILER)
#  define P_CC_INTEL
#endif

/* Give warning if returned from function result is not used */
#if __GNUC__ > 3 || (__GNUC__ == 3 && __GNUC_MINOR__ >= 4)
#  define P_GNUC_WARN_UNUSED_RESULT __attribute__((warn_unused_result))
#else
#  define P_GNUC_WARN_UNUSED_RESULT
#endif

/* For proper library compilation on Windows */
#ifdef P_CC_MSVC
#  define P_LIB_API __declspec(dllexport)
#else
#  define P_LIB_API
#endif

/* Notify compiler that function will never return a value (e.g. abort ()) */
#ifdef P_CC_MSVC
#  define P_NO_RETURN __declspec(noreturn)
#elif defined(P_CC_GNU)
#  define P_NO_RETURN __attribute__((noreturn))
#else
#  define P_NO_RETURN
#endif

/* Convert pointer to int and vice versa, 32-bit only */
#define P_INT_TO_POINTER(i) ((void *) (long)(i))
#define P_POINTER_TO_INT(p) ((int) (long)(p))

/* Platform-independent MT-safe strtok ()*/
#ifdef P_OS_WIN
#  define P_STRTOK(str, delim, buf) strtok_s(str, delim, buf)
#else
#  define P_STRTOK(str, delim, buf) strtok_r(str, delim, buf)
#endif

#ifdef P_OS_WIN
#  define P_LOCALTIME_R(gt, lt) localtime_s(lt, gt)
#else
#  define P_LOCALTIME_R(gt, lt) localtime_r(gt, lt)
#endif

/* Printing macroses */
#define P_WARNING(msg) printf ("** Warning: %s **\n", msg)
#define P_ERROR(msg) printf ("** Error: %s **\n", msg)
#define P_DEBUG(msg) printf ("** Debug: %s **\n", msg)

/* For .h-file declarations */
#ifdef __cplusplus
#  define P_BEGIN_DECLS extern "C" {
#  define P_END_DECLS }
#else
#  define P_BEGIN_DECLS
#  define P_END_DECLS
#endif

#endif /* __PMACROS_H__ */

