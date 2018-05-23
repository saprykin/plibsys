/*
 * The MIT License
 *
 * Copyright (C) 2010-2017 Alexander Saprykin <saprykin.spb@gmail.com>
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
 * @file pmacros.h
 * @brief Miscellaneous macros
 * @author Alexander Saprykin
 *
 * All the macros are completely independent of any other platform-specific
 * headers, thus gurantee to work with any compiler under any operating system
 * in the same way as they are used within the library.
 *
 * This family of macros provides various additional capabilities (compiler
 * hints, attributes, version, etc.).
 */

#if !defined (PLIBSYS_H_INSIDE) && !defined (PLIBSYS_COMPILATION)
#  error "Header files shouldn't be included directly, consider using <plibsys.h> instead."
#endif

#ifndef PLIBSYS_HEADER_PMACROS_H
#define PLIBSYS_HEADER_PMACROS_H

#include <pmacroscompiler.h>
#include <pmacroscpu.h>
#include <pmacrosos.h>

#include <stdio.h>

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
 * @def P_LIB_INTERNAL_API
 * @brief Marks a symbol (variable, function) as local.
 * @since 0.0.4
 *
 * Local symbols are not exported during the linkage and are not available from
 * the outside of the module they are defined in. Use it for internal API.
 *
 * @note Some compilers allow to put this attribute at the beginning of the
 * symbol declaration, and some also at the end of the declaration. Thus it is
 * better to put it in the beginning for more portability.
 */

/**
 * @def P_LIB_GLOBAL_API
 * @brief Marks a symbol (variable, function) as global.
 * @since 0.0.4
 *
 * Global symbols are exported during the linkage and are available from the
 * outside of the module they are defined in. Use it for public API.
 *
 * @note Some compilers allow to put this attribute at the beginning of the
 * symbol declaration, and some also at the end of the declaration. Thus it is
 * better to put it in the beginning for more portability.
 */

/*
 * Oracle Solaris Studio since version 12 has visibility attribute for C
 * compiler, and since version 12.2 for C++ compiler, or since version 8.0
 * specific __global attribute which is the same.
 * IBM XL C has support for visibility attributes since version 13.1.
 * HP C/aC++ has support for visibility attributes since version A.06.15.
 */

#if defined(P_CC_MSVC) || defined(P_CC_BORLAND) || defined(P_CC_WATCOM) || \
    defined(P_OS_OS2)  || (defined(P_OS_BEOS)  && !defined(P_CC_GNU))   || \
    (defined(P_OS_WIN) && defined(P_CC_PGI)) || \
    ((defined(P_OS_WIN) || defined(P_OS_CYGWIN) || defined(P_OS_MSYS)) && defined(P_CC_GNU))
#  define P_LIB_GLOBAL_API __declspec(dllexport)
#  define P_LIB_INTERNAL_API
#elif ((__GNUC__ >= 4) && !defined(P_OS_SOLARIS) && !defined(P_OS_HPUX) && !defined(P_OS_AIX)) || \
      (defined(P_CC_SUN) && __SUNPRO_C  >= 0x590)  || \
      (defined(P_CC_SUN) && __SUNPRO_CC >= 0x5110) || \
      (defined(P_CC_XLC) && __xlC__ >= 0x0D01)     || \
      (defined(P_CC_HP)  && __HP_aCC >= 0x061500)  || \
      (defined(P_CC_HP)  && __HP_cc >= 0x061500)   || \
      __has_attribute(visibility)
#  define P_LIB_GLOBAL_API __attribute__ ((visibility ("default")))
#  define P_LIB_INTERNAL_API __attribute__ ((visibility ("hidden")))
#elif defined(__SUNPRO_C) && (__SUNPRO_C >= 0x550)
#  define P_LIB_GLOBAL_API __global
#  define P_LIB_INTERNAL_API __hidden
#else
#  define P_LIB_GLOBAL_API
#  define P_LIB_INTERNAL_API
#endif

/**
 * @def P_LIB_API
 * @brief Exports a symbol from a shared library.
 * @since 0.0.1
 */

#define P_LIB_API P_LIB_GLOBAL_API

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
