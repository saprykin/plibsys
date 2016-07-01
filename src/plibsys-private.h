/*
 * Copyright (C) 2013-2016 Alexander Saprykin <xelfium@gmail.com>
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

#if !defined (PLIBSYS_H_INSIDE) && !defined (PLIBSYS_COMPILATION)
#  error "Header files shouldn't be included directly, consider using <plibsys.h> instead."
#endif

#ifndef PLIBSYS_HEADER_PLIBSYS_PRIVATE_H
#define PLIBSYS_HEADER_PLIBSYS_PRIVATE_H

#include "pmacros.h"
#include "ptypes.h"

P_BEGIN_DECLS

#ifndef PLIBSYS_HAS_SOCKLEN_T
#  ifdef P_OS_VMS
typedef unsigned int socklen_t;
#  else
typedef int socklen_t;
#  endif
#endif

#ifndef PLIBSYS_HAS_SOCKADDR_STORAGE
/* According to RFC 2553 */
#  define _PLIBSYS_SS_MAXSIZE	128
#  define _PLIBSYS_SS_ALIGNSIZE	(sizeof (pint64))

#  ifdef PLIBSYS_SOCKADDR_HAS_SA_LEN
#    define _PLIBSYS_SS_PAD1SIZE	(_PLIBSYS_SS_ALIGNSIZE - (sizeof (puchar) + sizeof (puchar)))
#  else
#    define _PLIBSYS_SS_PAD1SIZE	(_PLIBSYS_SS_ALIGNSIZE - sizeof (puchar))
#  endif

#  define _PLIBSYS_SS_PAD2SIZE	(_PLIBSYS_SS_MAXSIZE - (sizeof (puchar) + _PLIBSYS_SS_PAD1SIZE + _PLIBSYS_SS_ALIGNSIZE))

struct sockaddr_storage {
#  ifdef PLIBSYS_SOCKADDR_HAS_SA_LEN
	puchar		ss_len;
#  endif
#  ifdef PLIBSYS_SIZEOF_SAFAMILY_T
#    if (PLIBSYS_SIZEOF_SAFAMILY_T == 1)
	puchar		ss_family;
#    elif (PLIBSYS_SIZEOF_SAFAMILY_T == 2)
	pushort		ss_family;
#    else
	puint		ss_family;
#    endif
#  else
#    ifdef PLIBSYS_SOCKADDR_HAS_SA_LEN
	puchar		ss_family;
#    else
	pushort		ss_family;
#    endif
#  endif
	pchar		__ss_pad1[_PLIBSYS_SS_PAD1SIZE];
	pint64		__ss_align;
	pchar		__ss_pad2[_PLIBSYS_SS_PAD2SIZE];
};
#endif

P_END_DECLS

#endif /* PLIBSYS_HEADER_PLIBSYS_PRIVATE_H */
