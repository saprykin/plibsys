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

#if !defined (__PLIB_H_INSIDE__) && !defined (PLIB_COMPILATION)
#  error "Header files shouldn't be included directly, consider using <plib.h> instead."
#endif

#ifndef __PTYPES_H__
#define __PTYPES_H__

#include <plibconfig.h>
#include <pmacros.h>

P_BEGIN_DECLS

typedef void *		ppointer;
typedef const void *	pconstpointer;

typedef signed int	pboolean;
typedef char		pchar;
typedef short		pshort;
typedef long		plong;
typedef int		pint;

typedef unsigned char	puchar;
typedef unsigned short	pushort;
typedef unsigned long	pulong;
typedef unsigned int	puint;

typedef float		pfloat;
typedef double		pdouble;

#define P_MININT8	((pint8)  0x80)
#define P_MAXINT8	((pint8)  0x7F)
#define P_MAXUINT8	((puint8) 0xFF)

#define P_MININT16	((pint16)  0x8000)
#define P_MAXINT16	((pint16)  0x7FFF)
#define P_MAXUINT16	((puint16) 0xFFFF)

#define P_MININT32	((pint32)  0x80000000)
#define P_MAXINT32	((pint32)  0x7FFFFFFF)
#define P_MAXUINT32	((puint32) 0xFFFFFFFF)

#define P_MININT64	((pint64)  0x8000000000000000)
#define P_MAXINT64	((pint64)  0x7FFFFFFFFFFFFFFF)
#define P_MAXUINT64	((puint64) 0xFFFFFFFFFFFFFFFFU)

/* Endian checks, see P_BYTE_ORDER in plibconfig.h */
#define P_LITTLE_ENDIAN	1234
#define P_BIG_ENDIAN	4321

/* Functions for bit swapping */
#define PUINT16_SWAP_BYTES(val)			\
	((puint16) (((puint16) (val)) >> 8) | ((puint16) (val)) << 8))

#define PUINT32_SWAP_BYTES(val) ((puint32) (			\
	(((puint32) (val)) >> 24) |				\
	((((puint32) (val)) << 8) & ((puint32) 0x00FF0000U)) |	\
	((((puint32) (val)) >> 8) & ((puint32) 0x0000FF00U)) |	\
	(((puint32) (val)) << 24)))

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
#define PINT16_FROM_LE(val)	(PINT16_TO_LE (val))
#define PUINT16_FROM_LE(val)	(PUINT16_TO_LE (val))
#define PINT16_FROM_BE(val)	(PINT16_TO_BE (val))
#define PUINT16_FROM_BE(val)	(PUINT16_TO_BE (val))
#define PINT32_FROM_LE(val)	(PINT32_TO_LE (val))
#define PUINT32_FROM_LE(val)	(PUINT32_TO_LE (val))
#define PINT32_FROM_BE(val)	(PINT32_TO_BE (val))
#define PUINT32_FROM_BE(val)	(PUINT32_TO_BE (val))

#define PINT64_FROM_LE(val)	(PINT64_TO_LE (val))
#define PUINT64_FROM_LE(val)	(PUINT64_TO_LE (val))
#define PINT64_FROM_BE(val)	(PINT64_TO_BE (val))
#define PUINT64_FROM_BE(val)	(PUINT64_TO_BE (val))

#define PLONG_FROM_LE(val)	(PLONG_TO_LE (val))
#define PULONG_FROM_LE(val)	(PULONG_TO_LE (val))
#define PLONG_FROM_BE(val)	(PLONG_TO_BE (val))
#define PULONG_FROM_BE(val)	(PULONG_TO_BE (val))

#define PINT_FROM_LE(val)	(PINT_TO_LE (val))
#define PUINT_FROM_LE(val)	(PUINT_TO_LE (val))
#define PINT_FROM_BE(val)	(PINT_TO_BE (val))
#define PUINT_FROM_BE(val)	(PUINT_TO_BE (val))

#define PSIZE_FROM_LE(val)	(PSIZE_TO_LE (val))
#define PSSIZE_FROM_LE(val)	(PSSIZE_TO_LE (val))
#define PSIZE_FROM_BE(val)	(PSIZE_TO_BE (val))
#define PSSIZE_FROM_BE(val)	(PSSIZE_TO_BE (val))

/* Host-network order functions */
#define p_ntohl(val) (PUINT32_FROM_BE (val))
#define p_ntohs(val) (PUINT16_FROM_BE (val))
#define p_htonl(val) (PUINT32_TO_BE (val))
#define p_htons(val) (PUINT16_TO_BE (val))

#ifndef FALSE
#  define FALSE (0)
#endif

#ifndef TRUE
#  define TRUE (!FALSE)
#endif

P_END_DECLS

#endif /* __PTYPES_H__ */

