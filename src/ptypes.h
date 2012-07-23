/* 
 * 25.08.2010
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

/**
 * @file ptypes.h
 * @brief Types definitions
 * @author Alexander Saprykin
 *
 * Types and swap routines definitions.
 */

#if !defined (__PLIB_H_INSIDE__) && !defined (PLIB_COMPILATION)
#  error "Header files shouldn't be included directly, consider using <plib.h> instead."
#endif

#ifndef __PTYPES_H__
#define __PTYPES_H__

#include <plibconfig.h>
#include <pmacros.h>

P_BEGIN_DECLS

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
/** Type for long */
typedef long		plong;
/** Type long */
typedef int		pint;

/** Type for unsigned char */
typedef unsigned char	puchar;
/** Type for unsigned short */
typedef unsigned short	pushort;
/** Type for unsigned long */
typedef unsigned long	pulong;
/** Type for unsigned int */
typedef unsigned int	puint;

/** Type for float */
typedef float		pfloat;
/** Type for double precision float */
typedef double		pdouble;

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
#define P_MININT64	((pint64)  0x8000000000000000)
/** Max value for 64-bit int */
#define P_MAXINT64	((pint64)  0x7FFFFFFFFFFFFFFF)
/** Max value for 64-bit unsigned int */
#define P_MAXUINT64	((puint64) 0xFFFFFFFFFFFFFFFFU)

/* Endian checks, see P_BYTE_ORDER in plibconfig.h */

/** Little endian mark */
#define P_LITTLE_ENDIAN	1234
/** Big endian mark */
#define P_BIG_ENDIAN	4321

/* Functions for bit swapping */

/**
 * @brief Swaps 16-bit unsigned int.
 * @param val Value to swap.
 * @return Swapped 16-bit unsigned int.
 * @since 0.0.1
 */
#define PUINT16_SWAP_BYTES(val)			\
	((puint16) (((puint16) (val)) >> 8) | ((puint16) (val)) << 8))

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
	(((puint64) (val))  >> 56)) |					\
	((((puint64) (val)) << 40) & ((puint64) 0x00FF000000000000U)) |	\
	((((puint64) (val)) << 24) & ((puint64) 0x0000FF0000000000U)) |	\
	((((puint64) (val)) <<  8) & ((puint64) 0x000000FF00000000U)) |	\
	((((puint64) (val)) >>  8) & ((puint64) 0x00000000FF000000U)) |	\
	((((puint64) (val)) >> 24) & ((puint64) 0x0000000000FF0000U)) |	\
	((((puint64) (val)) >> 40) & ((puint64) 0x000000000000FF00U)) |	\
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

P_END_DECLS

#endif /* __PTYPES_H__ */
