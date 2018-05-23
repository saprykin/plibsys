/*
 * The MIT License
 *
 * Copyright (C) 2013-2016 Alexander Saprykin <saprykin.spb@gmail.com>
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
