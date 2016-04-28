/*
 * Copyright (C) 2010-2016 Alexander Saprykin <xelfium@gmail.com>
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

/**
 * @file ptypes.h
 * @brief Types definitions
 * @author Alexander Saprykin
 *
 * Types and swap routines definitions.
 */

#if !defined (__PLIBSYS_H_INSIDE__) && !defined (PLIBSYS_COMPILATION)
#  error "Header files shouldn't be included directly, consider using <plibsys.h> instead."
#endif

#ifndef __PTYPES_H__
#define __PTYPES_H__

#include <plibsysconfig.h>
#include <pmacros.h>

P_BEGIN_DECLS

/** Type for signed 8 bit */
typedef signed char		pint8;
/** Type for unsigned 8 bit */
typedef unsigned char		puint8;
/** Type for signed 16 bit */
typedef signed short		pint16;
/** Type for unsigned 16 bit */
typedef unsigned short		puint16;
/** Type for signed 32 bit */
typedef signed int		pint32;
/** Type for unsigned 32 bit */
typedef unsigned int		puint32;

/**
  * @var pint64
  * @brief Type for signed 64 bit
  */

/**
  * @var puint64
  * @brief Type for unsigned 64 bit
  */

#if defined (P_OS_WIN) && (defined (P_CC_MSVC) || defined (P_CC_BORLAND))
  typedef signed __int64	pint64;
  typedef unsigned __int64	puint64;
#else
#  if PLIBSYS_SIZEOF_LONG == 8
     typedef signed long	pint64;
     typedef unsigned long	puint64;
#  else
     typedef signed long long	pint64;
     typedef unsigned long long	puint64;
#  endif
#endif

/** Type for pointer */
typedef void *		ppointer;
/** Type for const pointer */
typedef const void *	pconstpointer;

/** Type for bool */
typedef signed int	pboolean;
/** Type for char */
typedef char		pchar;
/** Type for short */
typedef short		pshort;
/** Type for int */
typedef int		pint;
/** Type for long */
typedef long		plong;

/** Type for unsigned char */
typedef unsigned char	puchar;
/** Type for unsigned short */
typedef unsigned short	pushort;
/** Type for unsigned int */
typedef unsigned int	puint;
/** Type for unsigned long */
typedef unsigned long	pulong;

/** Type for float */
typedef float		pfloat;
/** Type for double precision float */
typedef double		pdouble;

/**
  * @var pssize
  * @brief Type for platform independent signed size_t
  */

/**
  * @var psize
  * @brief Type for platform independent size_t
  */

/**
  * @def PSIZE_MODIFIER
  * @brief Platform dependent length modifier for conversion specifiers
  * of #psize or #pssize type for printing and scanning values. It is
  * a string literal, but doesn't include percent sign so you can add
  * precision and length modifiers and append a conversion specifier.
  * @code
  * psize size_val = 256;
  * printf ("%#" PSIZE_MODIFIER "x", size_val);
  * @endcode
  */

/**
  * @def PSSIZE_FORMAT
  * @brief Platform dependent conversion specifier of #pssize type
  * for printing and scanning values.
  * @code
  * pssize size_val = 100;
  * printf ("%" PSSIZE_FORMAT, size_val);
  * @endcode
  */

/**
  * @def PSIZE_FORMAT
  * @brief Platform dependent conversion specifier of #psize type
  * for printing and scanning values.
  */

/**
  * @def P_MAXSIZE
  * @brief Maximum value of #psize type
  */

/**
  * @def P_MINSSIZE
  * @brief Minimum value of #pssize type
  */

/**
  * @def P_MAXSSIZE
  * @brief Maximum value of #pssize type
  */

#if PLIBSYS_SIZEOF_SIZE_T == 8
#  if defined (P_OS_WIN) && (defined (P_CC_MSVC) || defined (P_CC_BORLAND))
     typedef signed __int64	pssize;
     typedef unsigned __int64	psize;
     #define PSIZE_MODIFIER	"I64"
     #define PSSIZE_FORMAT	"I64d"
     #define PSIZE_FORMAT	"I64u"
     #define P_MAXSIZE		P_MAXUINT64
     #define P_MINSSIZE		P_MININT64
     #define P_MAXSSIZE		P_MAXINT64
#  else
#    if PLIBSYS_SIZEOF_LONG == 8
       typedef long			pssize;
       typedef unsigned long		psize;
       #define PSIZE_MODIFIER		"l"
       #define PSSIZE_FORMAT		"li"
       #define PSIZE_FORMAT		"lu"
       #define P_MAXSIZE		P_MAXULONG
       #define P_MINSSIZE		P_MINLONG
       #define P_MAXSSIZE		P_MAXLONG
#    else
       typedef long long		pssize;
       typedef unsigned long long	psize;
       #define PSIZE_MODIFIER		"ll"
       #define PSSIZE_FORMAT		"lli"
       #define PSIZE_FORMAT		"llu"
       #define P_MAXSIZE		P_MAXUINT64
       #define P_MINSSIZE		P_MININT64
       #define P_MAXSSIZE		P_MAXINT64
#    endif
#  endif
#else
  typedef signed int		pssize;
  typedef unsigned int		psize;
  #define PSIZE_MODIFIER	""
  #define PSSIZE_FORMAT		"i"
  #define PSIZE_FORMAT		"u"
  #define P_MAXSIZE		P_MAXUINT
  #define P_MINSSIZE		P_MININT
  #define P_MAXSSIZE		P_MAXINT
#endif

/**
  * @var pintptr
  * @brief Type for platform independent signed pointer represented by integer
  */

/**
  * @var puintptr
  * @brief Type for platform independent unsigned pointer represented by integer
  */

/**
  * @def PINTPTR_MODIFIER
  * @brief Platform dependent length modifier for conversion specifiers
  * of #pintptr or #puintptr type for printing and scanning values. It is
  * a string literal, but doesn't include percent sign so you can add
  * precision and length modifiers and append a conversion specifier.
  */

/**
  * @def PINTPTR_FORMAT
  * @brief Platform dependent conversion specifier of #pintptr type
  * for printing and scanning values.
  */

/**
  * @def PUINTPTR_FORMAT
  * @brief Platform dependent conversion specifier of #puintptr type
  * for printing and scanning values.
  */

#if PLIBSYS_SIZEOF_VOID_P == 8
#  if defined (P_OS_WIN) && (defined (P_CC_MSVC) || defined (P_CC_BORLAND))
     typedef signed __int64	pintptr;
     typedef unsigned __int64	puintptr;
     #define PINTPTR_MODIFIER	"I64"
     #define PINTPTR_FORMAT	"I64i"
     #define PUINTPTR_FORMAT	"I64u"
#  else
#    if PLIBSYS_SIZEOF_LONG == 8
       typedef long			pintptr;
       typedef unsigned long		puintptr;
       #define PINTPTR_MODIFIER		"l"
       #define PINTPTR_FORMAT		"li"
       #define PUINTPTR_FORMAT		"lu"
#    else
       typedef long long		pintptr;
       typedef unsigned long long	puintptr;
       #define PINTPTR_MODIFIER		"ll"
       #define PINTPTR_FORMAT		"lli"
       #define PUINTPTR_FORMAT		"llu"
#    endif
#  endif
#else
  typedef signed int		pintptr;
  typedef unsigned int		puintptr;
  #define PINTPTR_MODIFIER	""
  #define PINTPTR_FORMAT	"i"
  #define PUINTPTR_FORMAT	"u"
#endif

/** Platform independent offset_t definition */
typedef pint64 poffset;

#if PLIBSYS_SIZEOF_VOID_P == 8
#  define P_INT_TO_POINTER(i)	((void *)	(long long) (i))
#  define P_POINTER_TO_INT(p)	((int)		(long long) (p))
#  define PPOINTER_TO_INT(p)	((pint)		((pint64) (p)))
#  define PPOINTER_TO_UINT(p)	((puint)	((puint64) (p)))
#  define PINT_TO_POINTER(i)	((ppointer)	(pint64) (i))
#  define PUINT_TO_POINTER(u)	((ppointer)	(puint64) (u))
#else
#  define P_INT_TO_POINTER(i)	((void *)	(long) (i))
#  define P_POINTER_TO_INT(p)	((int)		(long) (p))
#  define PPOINTER_TO_INT(p)	((pint)		((plong) (p)))
#  define PPOINTER_TO_UINT(p)	((puint)	((pulong) (p)))
#  define PINT_TO_POINTER(i)	((ppointer)	(plong) (i))
#  define PUINT_TO_POINTER(u)	((ppointer)	(pulong) (u))
#endif

/**
 * @def P_INT_TO_POINTER
 * @brief Casts int to pointer
 * @param i Variable to cast.
 * @return Casted variable.
 * @since 0.0.1
 */

/**
 * @def P_POINTER_TO_INT
 * @brief Casts pointer to int
 * @param p Pointer to cast.
 * @return Casted pointer.
 * @since 0.0.1
 */

/**
 * @def PPOINTER_TO_INT
 * @brief Casts #ppointer to #pint value
 * @param p #ppointer to cast.
 * @return Casted #ppointer.
 * @since 0.0.1
 */

 /**
  * @def PPOINTER_TO_UINT
  * @brief Casts #ppointer to #pint value
  * @param p #ppointer to cast.
  * @return Casted #ppointer.
  * @since 0.0.1
  */

 /**
  * @def PINT_TO_POINTER
  * @brief Casts #pint value to #ppointer
  * @param i #pint to cast.
  * @return Casted #pint.
  * @since 0.0.1
  */

 /**
  * @def PUINT_TO_POINTER
  * @brief Casts #puint value to #ppointer
  * @param u #puint to cast.
  * @return Casted #puint.
  * @since 0.0.1
  */

/** Casts #psize value to #ppointer */
#define PSIZE_TO_POINTER(i)	((ppointer)  ((psize) (i)))
/** Casts #ppointer to #psize value */
#define PPOINTER_TO_PSIZE(p)	((psize)  (p))

/** Min value for 8-bit int */
#define P_MININT8	((pint8)  0x80)
/** Max value for 8-bit int */
#define P_MAXINT8	((pint8)  0x7F)
/** Max value for 8-bit unsigned int */
#define P_MAXUINT8	((puint8) 0xFF)

/** Min value for 16-bit int */
#define P_MININT16	((pint16)  0x8000)
/** Max value for 16-bit int */
#define P_MAXINT16	((pint16)  0x7FFF)
/** Max value for 16-bit unsigned int */
#define P_MAXUINT16	((puint16) 0xFFFF)

/** Min value for 32-bit int */
#define P_MININT32	((pint32)  0x80000000)
/** Max value for 32-bit int */
#define P_MAXINT32	((pint32)  0x7FFFFFFF)
/** Max value for 32-bit unsigned int */
#define P_MAXUINT32	((puint32) 0xFFFFFFFF)

/** Min value for 64-bit int */
#define P_MININT64	((pint64)  0x8000000000000000LL)
/** Max value for 64-bit int */
#define P_MAXINT64	((pint64)  0x7FFFFFFFFFFFFFFFLL)
/** Max value for 64-bit unsigned int */
#define P_MAXUINT64	((puint64) 0xFFFFFFFFFFFFFFFFULL)

/**
 * @def PINT16_MODIFIER
 * @brief Platform dependent length modifier for conversion specifiers
 * of #pint16 or #puint16 type for printing and scanning values. It is
 * a string literal, but doesn't include percent sign so you can add
 * precision and length modifiers and append a conversion specifier.
 */

/**
 * @def PINT16_FORMAT
 * @brief Platform dependent conversion specifier of #pint16 type
 * for printing and scanning values.
 */

/**
 * @def PUINT16_FORMAT
 * @brief Platform dependent conversion specifier of #puint16 type
 * for printing and scanning values.
 */

/**
 * @def PINT32_MODIFIER
 * @brief Platform dependent length modifier for conversion specifiers
 * of #pint32 or #puint32 type for printing and scanning values. It is
 * a string literal, but doesn't include percent sign so you can add
 * precision and length modifiers and append a conversion specifier.
 */

/**
 * @def PINT32_FORMAT
 * @brief Platform dependent conversion specifier of #pint32 type
 * for printing and scanning values.
 */

/**
 * @def PUINT32_FORMAT
 * @brief Platform dependent conversion specifier of #puint32 type
 * for printing and scanning values.
 */

/**
 * @def PINT64_MODIFIER
 * @brief Platform dependent length modifier for conversion specifiers
 * of #pint64 or #puint64 type for printing and scanning values. It is
 * a string literal, but doesn't include percent sign so you can add
 * precision and length modifiers and append a conversion specifier.
 */

/**
 * @def PINT64_FORMAT
 * @brief Platform dependent conversion specifier of #pint64 type
 * for printing and scanning values.
 */

/**
 * @def PUINT64_FORMAT
 * @brief Platform dependent conversion specifier of #puint64 type
 * for printing and scanning values.
 */

/**
 * @def POFFSET_MODIFIER
 * @brief Platform dependent length modifier for conversion specifiers
 * of #poffset type for printing and scanning values. It is
 * a string literal, but doesn't include percent sign so you can add
 * precision and length modifiers and append a conversion specifier.
 */

/**
 * @def POFFSET_FORMAT
 * @brief Platform dependent conversion specifier of #poffset type
 * for printing and scanning values.
 */

#if defined (P_OS_WIN) && (defined (P_CC_MSVC) || defined (P_CC_BORLAND))
  #define PINT16_MODIFIER	"h"
#else
  #define PINT16_MODIFIER	""
#endif

#define PINT16_FORMAT		"hi"
#define PUINT16_FORMAT		"hu"

#define PINT32_MODIFIER		""
#define PINT32_FORMAT		"i"
#define PUINT32_FORMAT		"u"

#if defined (P_OS_WIN) && (defined (P_CC_MSVC) || defined (P_CC_BORLAND))
  #define PINT64_MODIFIER	"I64"
  #define PINT64_FORMAT		"I64i"
  #define PUINT64_FORMAT	"I64u"
#else
#  if PLIBSYS_SIZEOF_LONG == 8
     #define PINT64_MODIFIER	"l"
     #define PINT64_FORMAT	"li"
     #define PUINT64_FORMAT	"lu"
#  else
     #define PINT64_MODIFIER	"ll"
     #define PINT64_FORMAT	"lli"
     #define PUINT64_FORMAT	"llu"
#  endif
#endif

#define POFFSET_MODIFIER	PINT64_MODIFIER
#define POFFSET_FORMAT		PINT64_FORMAT

/* Endian checks, see P_BYTE_ORDER in plibsysconfig.h */

/** Little endian mark */
#define P_LITTLE_ENDIAN	1234
/** Big endian mark */
#define P_BIG_ENDIAN	4321

/**
 * @def PINT16_TO_LE
 * @brief Swaps #pint16 variable from host to little endian order
 * @param val Value to swap.
 * @return Swapped value.
 * @since 0.0.1
 */

/**
 * @def PUINT16_TO_LE
 * @brief Swaps #puint16 variable from host to little endian order
 * @param val Value to swap.
 * @return Swapped value.
 * @since 0.0.1
 */

/**
 * @def PINT16_TO_BE
 * @brief Swaps #pint16 variable from host to big endian order
 * @param val Value to swap.
 * @return Swapped value.
 * @since 0.0.1
 */

/**
 * @def PUINT16_TO_BE
 * @brief Swaps #puint16 variable from host to big endian order
 * @param val Value to swap.
 * @return Swapped value.
 * @since 0.0.1
 */

/**
 * @def PINT32_TO_LE
 * @brief Swaps #pint32 variable from host to little endian order
 * @param val Value to swap.
 * @return Swapped value.
 * @since 0.0.1
 */

/**
 * @def PUINT32_TO_LE
 * @brief Swaps #puint32 variable from host to little endian order
 * @param val Value to swap.
 * @return Swapped value.
 * @since 0.0.1
 */

/**
 * @def PINT32_TO_BE
 * @brief Swaps #pint32 variable from host to big endian order
 * @param val Value to swap.
 * @return Swapped value.
 * @since 0.0.1
 */

/**
 * @def PUINT32_TO_BE
 * @brief Swaps #puint32 variable from host to big endian order
 * @param val Value to swap.
 * @return Swapped value.
 * @since 0.0.1
 */

/**
 * @def PINT64_TO_LE
 * @brief Swaps #pint64 variable from host to little endian order
 * @param val Value to swap.
 * @return Swapped value.
 * @since 0.0.1
 */

/**
 * @def PUINT64_TO_LE
 * @brief Swaps #puint64 variable from host to little endian order
 * @param val Value to swap.
 * @return Swapped value.
 * @since 0.0.1
 */

/**
 * @def PINT64_TO_BE
 * @brief Swaps #pint64 variable from host to big endian order
 * @param val Value to swap.
 * @return Swapped value.
 * @since 0.0.1
 */

/**
 * @def PUINT64_TO_BE
 * @brief Swaps #puint64 variable from host to big endian order
 * @param val Value to swap.
 * @return Swapped value.
 * @since 0.0.1
 */

/**
 * @def PLONG_TO_LE
 * @brief Swaps #plong variable from host to little endian order
 * @param val Value to swap.
 * @return Swapped value.
 * @since 0.0.1
 */

/**
 * @def PULONG_TO_LE
 * @brief Swaps #pulong variable from host to little endian order
 * @param val Value to swap.
 * @return Swapped value.
 * @since 0.0.1
 */

/**
 * @def PLONG_TO_BE
 * @brief Swaps #plong variable from host to big endian order
 * @param val Value to swap.
 * @return Swapped value.
 * @since 0.0.1
 */

/**
 * @def PULONG_TO_BE
 * @brief Swaps #pulong variable from host to big endian order
 * @param val Value to swap.
 * @return Swapped value.
 * @since 0.0.1
 */

/**
 * @def PSSIZE_TO_LE
 * @brief Swaps #pssize variable from host to little endian order
 * @param val Value to swap.
 * @return Swapped value.
 * @since 0.0.1
 */

/**
 * @def PSIZE_TO_LE
 * @brief Swaps #psize variable from host to little endian order
 * @param val Value to swap.
 * @return Swapped value.
 * @since 0.0.1
 */

/**
 * @def PSSIZE_TO_BE
 * @brief Swaps #pssize variable from host to big endian order
 * @param val Value to swap.
 * @return Swapped value.
 * @since 0.0.1
 */

/**
 * @def PSIZE_TO_BE
 * @brief Swaps #psize variable from host to big endian order
 * @param val Value to swap.
 * @return Swapped value.
 * @since 0.0.1
 */

/**
 * @def PINT_TO_LE
 * @brief Swaps #pint variable from host to little endian order
 * @param val Value to swap.
 * @return Swapped value.
 * @since 0.0.1
 */

/**
 * @def PUINT_TO_LE
 * @brief Swaps #puint variable from host to little endian order
 * @param val Value to swap.
 * @return Swapped value.
 * @since 0.0.1
 */

/**
 * @def PINT_TO_BE
 * @brief Swaps #pint variable from host to big endian order
 * @param val Value to swap.
 * @return Swapped value.
 * @since 0.0.1
 */

/**
 * @def PUINT_TO_BE
 * @brief Swaps #puint variable from host to big endian order
 * @param val Value to swap.
 * @return Swapped value.
 * @since 0.0.1
 */

#if P_BYTE_ORDER == P_LITTLE_ENDIAN
  #define PINT16_TO_LE(val)	((pint16) (val))
  #define PUINT16_TO_LE(val)	((puint16) (val))
  #define PINT16_TO_BE(val)	((pint16) PUINT16_SWAP_BYTES (val))
  #define PUINT16_TO_BE(val)	(PUINT16_SWAP_BYTES (val))
  #define PINT32_TO_LE(val)	((pint32) (val))
  #define PUINT32_TO_LE(val)	((puint32) (val))
  #define PINT32_TO_BE(val)	((pint32) PUINT32_SWAP_BYTES (val))
  #define PUINT32_TO_BE(val)	(PUINT32_SWAP_BYTES (val))
  #define PINT64_TO_LE(val)	((pint64) (val))
  #define PUINT64_TO_LE(val)	((puint64) (val))
  #define PINT64_TO_BE(val)	((pint64) PUINT64_SWAP_BYTES (val))
  #define PUINT64_TO_BE(val)	(PUINT64_SWAP_BYTES (val))
#  if PLIBSYS_SIZEOF_LONG == 8
     #define PLONG_TO_LE(val)	((plong) PINT64_TO_LE (val))
     #define PULONG_TO_LE(val)	((pulong) PUINT64_TO_LE (val))
     #define PLONG_TO_BE(val)	((plong) PINT64_TO_BE (val))
     #define PULONG_TO_BE(val)	((pulong) PUINT64_TO_BE (val))
#  else
     #define PLONG_TO_LE(val)	((plong) PINT32_TO_LE (val))
     #define PULONG_TO_LE(val)	((pulong) PUINT32_TO_LE (val))
     #define PLONG_TO_BE(val)	((plong) PINT32_TO_BE (val))
     #define PULONG_TO_BE(val)	((pulong) PUINT32_TO_BE (val))
#  endif
#  if PLIBSYS_SIZEOF_SIZE_T == 8
     #define PSIZE_TO_LE(val)	((psize) PUINT64_TO_LE (val))
     #define PSSIZE_TO_LE(val)	((pssize) PINT64_TO_LE (val))
     #define PSIZE_TO_BE(val)	((psize) PUINT64_TO_BE (val))
     #define PSSIZE_TO_BE(val)	((pssize) PINT64_TO_BE (val))
#  else
     #define PSIZE_TO_LE(val)	((psize) PUINT32_TO_LE (val))
     #define PSSIZE_TO_LE(val)	((pssize) PINT32_TO_LE (val))
     #define PSIZE_TO_BE(val)	((psize) PUINT32_TO_BE (val))
     #define PSSIZE_TO_BE(val)	((pssize) PINT32_TO_BE (val))
#  endif
  #define PINT_TO_LE(val)	((pint) PINT32_TO_LE (val))
  #define PUINT_TO_LE(val)	((puint) PUINT32_TO_LE (val))
  #define PINT_TO_BE(val)	((pint) PINT32_TO_BE (val))
  #define PUINT_TO_BE(val)	((puint) PUINT32_TO_BE (val))

#else
  #define PINT16_TO_LE(val)	((pint16) PUINT16_SWAP_BYTES (val))
  #define PUINT16_TO_LE(val)	(PUINT16_SWAP_BYTES (val))
  #define PINT16_TO_BE(val)	((pint16) (val))
  #define PUINT16_TO_BE(val)	((puint16) (val))
  #define PINT32_TO_LE(val)	((pint32) PUINT32_SWAP_BYTES (val))
  #define PUINT32_TO_LE(val)	(PUINT32_SWAP_BYTES (val))
  #define PINT32_TO_BE(val)	((pint32) (val))
  #define PUINT32_TO_BE(val)	((puint32) (val))
  #define PINT64_TO_LE(val)	((pint64) PUINT64_SWAP_BYTES (val))
  #define PUINT64_TO_LE(val)	(PUINT64_SWAP_BYTES (val))
  #define PINT64_TO_BE(val)	((pint64) (val))
  #define PUINT64_TO_BE(val)	((puint64) (val))
#  if PLIBSYS_SIZEOF_LONG == 8
     #define PLONG_TO_LE(val)	((plong) PINT64_TO_LE (val))
     #define PULONG_TO_LE(val)	((pulong) PUINT64_TO_LE (val))
     #define PLONG_TO_BE(val)	((plong) PINT64_TO_BE (val))
     #define PULONG_TO_BE(val)	((pulong) PUINT64_TO_BE (val))
#  else
     #define PLONG_TO_LE(val)	((plong) PINT32_TO_LE (val))
     #define PULONG_TO_LE(val)	((pulong) PUINT32_TO_LE (val))
     #define PLONG_TO_BE(val)	((plong) PINT32_TO_BE (val))
     #define PULONG_TO_BE(val)	((pulong) PUINT32_TO_BE (val))
#  endif
#  if PLIBSYS_SIZEOF_SIZE_T == 8
     #define PSIZE_TO_LE(val)	((psize) PUINT64_TO_LE (val))
     #define PSSIZE_TO_LE(val)	((pssize) PINT64_TO_LE (val))
     #define PSIZE_TO_BE(val)	((psize) PUINT64_TO_BE (val))
     #define PSSIZE_TO_BE(val)	((pssize) PINT64_TO_BE (val))
#  else
     #define PSIZE_TO_LE(val)	((psize) PUINT32_TO_LE (val))
     #define PSSIZE_TO_LE(val)	((pssize) PINT32_TO_LE (val))
     #define PSIZE_TO_BE(val)	((psize) PUINT32_TO_BE (val))
     #define PSSIZE_TO_BE(val)	((pssize) PINT32_TO_BE (val))
#  endif
  #define PINT_TO_LE(val)	((pint) PINT32_TO_LE (val))
  #define PUINT_TO_LE(val)	((puint) PUINT32_TO_LE (val))
  #define PINT_TO_BE(val)	((pint) PINT32_TO_BE (val))
  #define PUINT_TO_BE(val)	((puint) PUINT32_TO_BE (val))
#endif

/* Functions for bit swapping */

/**
 * @brief Swaps 16-bit unsigned int.
 * @param val Value to swap.
 * @return Swapped 16-bit unsigned int.
 * @since 0.0.1
 */
#define PUINT16_SWAP_BYTES(val)			\
	((puint16) (((puint16) (val)) >> 8 | ((puint16) (val)) << 8))

/**
 * @brief Swaps 32-bit unsigned int.
 * @param val Value to swap.
 * @return Swapped 32-bit unsigned int.
 * @since 0.0.1
 */
#define PUINT32_SWAP_BYTES(val) ((puint32) (			\
	(((puint32) (val)) >> 24) |				\
	((((puint32) (val)) << 8) & ((puint32) 0x00FF0000U)) |	\
	((((puint32) (val)) >> 8) & ((puint32) 0x0000FF00U)) |	\
	(((puint32) (val)) << 24)))

/**
 * @brief Swaps 64-bit unsigned int.
 * @param val Value to swap.
 * @return Swapped 64-bit unsigned int.
 * @since 0.0.1
 */
#define PUINT64_SWAP_BYTES(val) ((puint64) (				\
	(((puint64) (val))  >> 56) |					\
	((((puint64) (val)) << 40) & ((puint64) 0x00FF000000000000ULL)) |	\
	((((puint64) (val)) << 24) & ((puint64) 0x0000FF0000000000ULL)) |	\
	((((puint64) (val)) <<  8) & ((puint64) 0x000000FF00000000ULL)) |	\
	((((puint64) (val)) >>  8) & ((puint64) 0x00000000FF000000ULL)) |	\
	((((puint64) (val)) >> 24) & ((puint64) 0x0000000000FF0000ULL)) |	\
	((((puint64) (val)) >> 40) & ((puint64) 0x000000000000FF00ULL)) |	\
	(((puint64) (val))  << 56)))

/* Functions, similar to ?_TO_? functions */

/**
 * @brief Swaps 16-bit int from little endian byte order to host one.
 * @param val Value to swap.
 * @return Swapped 16-bit int.
 * @since 0.0.1
 */
#define PINT16_FROM_LE(val)	(PINT16_TO_LE (val))

/**
 * @brief Swaps 16-bit unsigned int from little endian byte order to host one.
 * @param val Value to swap.
 * @return Swapped 16-bit unsigned int.
 * @since 0.0.1
 */
#define PUINT16_FROM_LE(val)	(PUINT16_TO_LE (val))

/**
 * @brief Swaps 16-bit int from big endian byte order to host one.
 * @param val Value to swap.
 * @return Swapped 16-bit int.
 * @since 0.0.1
 */
#define PINT16_FROM_BE(val)	(PINT16_TO_BE (val))

/**
 * @brief Swaps 16-bit unsigned int from big endian byte order to host one.
 * @param val Value to swap.
 * @return Swapped 16-bit unsigned int.
 * @since 0.0.1
 */
#define PUINT16_FROM_BE(val)	(PUINT16_TO_BE (val))


/**
 * @brief Swaps 32-bit int from little endian byte order to host one.
 * @param val Value to swap.
 * @return Swapped 32-bit int.
 * @since 0.0.1
 */
#define PINT32_FROM_LE(val)	(PINT32_TO_LE (val))

/**
 * @brief Swaps 32-bit unsigned int from little endian byte order to host one.
 * @param val Value to swap.
 * @return Swapped 32-bit unsigned int.
 * @since 0.0.1
 */
#define PUINT32_FROM_LE(val)	(PUINT32_TO_LE (val))

/**
 * @brief Swaps 32-bit int from big endian byte order to host one.
 * @param val Value to swap.
 * @return Swapped 32-bit int.
 * @since 0.0.1
 */
#define PINT32_FROM_BE(val)	(PINT32_TO_BE (val))

/**
 * @brief Swaps 32-bit unsigned int from big endian byte order to host one.
 * @param val Value to swap.
 * @return Swapped 32-bit unsigned int.
 * @since 0.0.1
 */
#define PUINT32_FROM_BE(val)	(PUINT32_TO_BE (val))


/**
 * @brief Swaps 64-bit int from little endian byte order to host one.
 * @param val Value to swap.
 * @return Swapped 64-bit int.
 * @since 0.0.1
 */
#define PINT64_FROM_LE(val)	(PINT64_TO_LE (val))

/**
 * @brief Swaps 64-bit unsigned int from little endian byte order to host one.
 * @param val Value to swap.
 * @return Swapped 64-bit unsigned int.
 * @since 0.0.1
 */
#define PUINT64_FROM_LE(val)	(PUINT64_TO_LE (val))

/**
 * @brief Swaps 64-bit int from big endian byte order to host one.
 * @param val Value to swap.
 * @return Swapped 64-bit int.
 * @since 0.0.1
 */
#define PINT64_FROM_BE(val)	(PINT64_TO_BE (val))

/**
 * @brief Swaps 64-bit unsigned int from big endian byte order to host one.
 * @param val Value to swap.
 * @return Swapped 64-bit unsigned int.
 * @since 0.0.1
 */
#define PUINT64_FROM_BE(val)	(PUINT64_TO_BE (val))


/**
 * @brief Swaps long int from little endian byte order to host one.
 * @param val Value to swap.
 * @return Swapped long int.
 * @since 0.0.1
 */
#define PLONG_FROM_LE(val)	(PLONG_TO_LE (val))

/**
 * @brief Swaps unsigned long int from little endian byte order to host one.
 * @param val Value to swap.
 * @return Swapped unsigned long int.
 * @since 0.0.1
 */
#define PULONG_FROM_LE(val)	(PULONG_TO_LE (val))

/**
 * @brief Swaps long int from big endian byte order to host one.
 * @param val Value to swap.
 * @return Swapped long int.
 * @since 0.0.1
 */
#define PLONG_FROM_BE(val)	(PLONG_TO_BE (val))

/**
 * @brief Swaps unsigned long int from big endian byte order to host one.
 * @param val Value to swap.
 * @return Swapped unsigned long int.
 * @since 0.0.1
 */
#define PULONG_FROM_BE(val)	(PULONG_TO_BE (val))


/**
 * @brief Swaps #pint from little endian byte order to host one.
 * @param val Value to swap.
 * @return Swapped #pint.
 * @since 0.0.1
 */
#define PINT_FROM_LE(val)	(PINT_TO_LE (val))

/**
 * @brief Swaps #puint from little endian byte order to host one.
 * @param val Value to swap.
 * @return Swapped #puint.
 * @since 0.0.1
 */
#define PUINT_FROM_LE(val)	(PUINT_TO_LE (val))

/**
 * @brief Swaps #pint from big endian byte order to host one.
 * @param val Value to swap.
 * @return Swapped #pint.
 * @since 0.0.1
 */
#define PINT_FROM_BE(val)	(PINT_TO_BE (val))

/**
 * @brief Swaps #puint from big endian byte order to host one.
 * @param val Value to swap.
 * @return Swapped #puint.
 * @since 0.0.1
 */
#define PUINT_FROM_BE(val)	(PUINT_TO_BE (val))


/**
 * @brief Swaps #psize from little endian byte order to host one.
 * @param val Value to swap.
 * @return Swapped #psize.
 * @since 0.0.1
 */
#define PSIZE_FROM_LE(val)	(PSIZE_TO_LE (val))

/**
 * @brief Swaps #pssize from little endian byte order to host one.
 * @param val Value to swap.
 * @return Swapped #pssize.
 * @since 0.0.1
 */
#define PSSIZE_FROM_LE(val)	(PSSIZE_TO_LE (val))

/**
 * @brief Swaps #psize from big endian byte order to host one.
 * @param val Value to swap.
 * @return Swapped #psize.
 * @since 0.0.1
 */
#define PSIZE_FROM_BE(val)	(PSIZE_TO_BE (val))

/**
 * @brief Swaps #pssize from big endian byte order to host one.
 * @param val Value to swap.
 * @return Swapped #pssize.
 * @since 0.0.1
 */
#define PSSIZE_FROM_BE(val)	(PSSIZE_TO_BE (val))


/* Host-network order functions */

/**
 * @brief Swaps long int from network byte order to host one.
 * @param val Value to swap.
 * @return Swapped long int.
 * @since 0.0.1
 */
#define p_ntohl(val) (PUINT32_FROM_BE (val))

/**
 * @brief Swaps short int from network byte order to host one.
 * @param val Value to swap.
 * @return Swapped short int.
 * @since 0.0.1
 */
#define p_ntohs(val) (PUINT16_FROM_BE (val))

/**
 * @brief Swaps long int from host byte order to network one.
 * @param val Value to swap.
 * @return Swapped long int.
 * @since 0.0.1
 */
#define p_htonl(val) (PUINT32_TO_BE (val))

/**
 * @brief Swaps short int from host byte order to network one.
 * @param val Value to swap.
 * @return Swapped short int.
 * @since 0.0.1
 */
#define p_htons(val) (PUINT16_TO_BE (val))

#ifndef FALSE
/** Type definition for false boolean value */
#  define FALSE (0)
#endif

#ifndef TRUE
/** Type definition for true boolean value */
#  define TRUE (!FALSE)
#endif

/**
 * @brief Function to traverse through a key-value container.
 * @param key The key of the item.
 * @param value The value of the item.
 * @param user_data Data provided by user, maybe NULL.
 * @return FALSE to continue traversing, TRUE to stop it.
 * @since 0.0.1
 */
typedef pboolean (*PTraverseFunc) (ppointer key,
				   ppointer value,
				   ppointer user_data);

/**
 * @brief General purpose function.
 * @param data Main argument related to context value.
 * @param user_data Additional (maybe NULL) user-provided data.
 * @since 0.0.1
 */
typedef void (*PFunc) (ppointer data, ppointer user_data);

/**
 * @brief Object destroy notification function.
 * @param data Pointer to object to be destroyed.
 * @since 0.0.1
 */
typedef void (*PDestroyFunc) (ppointer data);

/**
 * @brief Compares two values.
 * @param a First value to compare.
 * @param b Second value to compare.
 * @return -1 if first value less then second, 1 if first value
 * greater then second, 0 otherwise.
 * @since 0.0.1
 */
typedef pint (*PCompareFunc) (pconstpointer a, pconstpointer b);

/**
 * @brief Compares two values with additional data.
 * @param a First value to compare.
 * @param b Second value to compare.
 * @param data Addition data, may be NULL.
 * @return -1 if first value less then second, 1 if first value
 * greater then second, 0 otherwise.
 * @since 0.0.1
 */
typedef pint (*PCompareDataFunc) (pconstpointer a, pconstpointer b, ppointer data);

P_END_DECLS

#endif /* __PTYPES_H__ */
