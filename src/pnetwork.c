/* 
 * 07.10.2010
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
 *
 *
 * Copyright (c) 1996,1999 by Internet Software Consortium.
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND INTERNET SOFTWARE CONSORTIUM DISCLAIMS
 * ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL INTERNET SOFTWARE
 * CONSORTIUM BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
 * PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS
 * ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 */

/* Only for systems without inet_pton () and inet_ntop () functions, e.g. old Solaris 2.5.1,
 * otherwise use system implementations */

#include "pnetwork.h"

#include <errno.h>
#include <string.h>

#define NS_INT16SZ	 2
#define NS_INADDRSZ	 4
#define NS_IN6ADDRSZ	16

/* WARNING: Don't even consider trying to compile this on a system where
 * sizeof(int) < 4, sizeof(int) > 4 is fine; all the world's not a VAX */

static const char * inet_ntop4 (const unsigned char *src, char *dst, socklen_t size);
static int inet_pton4 (const char *src, unsigned char *dst);

#ifdef AF_INET6
static const char *inet_ntop6(const unsigned char *src, char *dst, socklen_t size);
static int inet_pton6 (const char *src, unsigned char *dst);
#endif

const char *
inet_ntop (int			af,
	   const void		*src,
	   char			*dst,
	   socklen_t		size)
{
	switch (af) {
	case AF_INET:
		return (inet_ntop4 (src, dst, size));
#ifdef AF_INET6
	case AF_INET6:
		return (inet_ntop6 (src, dst, size));
#endif
	default:
		errno = EAFNOSUPPORT;
		return (NULL);
	}
	/* NOTREACHED */
}

static const char *
inet_ntop4 (const unsigned char	*src,
	    char		*dst,
	    socklen_t		size)
{
	static const char	*fmt = "%u.%u.%u.%u";
	char			tmp[sizeof "255.255.255.255"];
	size_t			len;

	len = sprintf (tmp, fmt, src[0], src[1], src[2], src[3]);
	if (len >= size) {
		errno = ENOSPC;
		return (NULL);
	}
	memcpy (dst, tmp, len + 1);

	return (dst);
}

#ifdef AF_INET6
static const char *
inet_ntop6 (const unsigned char	*src,
	    char		*dst,
	    socklen_t		size)
{
	/* Note that int32_t and int16_t need only be "at least" large enough
	 * to contain a value of the specified size.  On some systems, like
	 * Crays, there is no such thing as an integer variable with 16 bits.
	 * Keep this in mind if you think this function should have been coded
	 * to use pointer overlays.  All the world's not a VAX. */
	char		tmp[sizeof "ffff:ffff:ffff:ffff:ffff:ffff:255.255.255.255"], *tp;
	struct {
		int base, len;
	}		best, cur;
	
	unsigned int	words[NS_IN6ADDRSZ / NS_INT16SZ];
	int	i, inc;

	/* Preprocess:
	 *	Copy the input (bytewise) array into a wordwise array.
	 *	Find the longest run of 0x00's in src[] for :: shorthanding. */
	memset (words, '\0', sizeof words);

	for (i = 0; i < NS_IN6ADDRSZ; i++)
		words[i / 2] |= (src[i] << ((1 - (i % 2)) << 3));
	
	best.base = -1;
	best.len = 0;
	cur.base = -1;
	cur.len = 0;

	for (i = 0; i < (NS_IN6ADDRSZ / NS_INT16SZ); i++) {
		if (words[i] == 0) {
			if (cur.base == -1)
				cur.base = i, cur.len = 1;
			else
				cur.len++;
		} else {
			if (cur.base != -1) {
				if (best.base == -1 || cur.len > best.len)
					best = cur;
				cur.base = -1;
			}
		}
	}
	if (cur.base != -1) {
		if (best.base == -1 || cur.len > best.len)
			best = cur;
	}

	if (best.base != -1 && best.len < 2)
		best.base = -1;

	/* Format the result */
	tp = tmp;
	for (i = 0; i < (NS_IN6ADDRSZ / NS_INT16SZ); i++) {
		/* Are we inside the best run of 0x00's? */
		if (best.base != -1 && i >= best.base &&
		    i < (best.base + best.len)) {
			if (i == best.base)
				*tp++ = ':';
			continue;
		}
		/* Are we following an initial run of 0x00s or any real hex? */
		if (i != 0)
			*tp++ = ':';
		/* Is this address an encapsulated IPv4? */
		if (i == 6 && best.base == 0 &&
		    (best.len == 6 || (best.len == 5 && words[5] == 0xffff))) {
			if (!inet_ntop4 (src + 12, tp, sizeof tmp - (tp - tmp)))
				return (NULL);
			tp += strlen (tp);
			break;
		}
		inc = sprintf (tp, "%x", words[i]);
		if (inc >= 5) {
			abort();
		}
		tp += inc;
	}
	/* Was it a trailing run of 0x00's? */
	if (best.base != -1 && (best.base + best.len) ==
	    (NS_IN6ADDRSZ / NS_INT16SZ))
		*tp++ = ':';
	*tp++ = '\0';

	/* Check for overflow, copy, and we're done */
	if ((size_t) (tp - tmp) > size) {
		errno = ENOSPC;
		return (NULL);
	}

	memcpy (dst, tmp, tp - tmp);
	return (dst);
}
#endif /* AF_INET6 */

int
inet_pton (int		af,
	   const char	*src,
	   void		*dst)
{
	switch (af) {
	case AF_INET:
		return (inet_pton4 (src, dst));
#ifdef AF_INET6
	case AF_INET6:
		return (inet_pton6 (src, dst));
#endif
	default:
		errno = EAFNOSUPPORT;
		return (-1);
	}
	/* NOTREACHED */
}

static int
inet_pton4 (const char		*src,
	    unsigned char	*dst)
{
	const char		digits[] = "0123456789";
	int			saw_digit, octets, ch;
	unsigned char		tmp[NS_INADDRSZ], *tp;

	saw_digit = 0;
	octets = 0;
	*(tp = tmp) = 0;

	while ((ch = *src++) != '\0') {
		char *pch;

		if ((pch = strchr ((const char *) digits, ch)) != NULL) {
			unsigned int new_s;

			new_s = *tp * 10 + (pch - digits);

			if (new_s > 255)
				return (0);
			*tp = new_s;
			if (! saw_digit) {
				if (++octets > 4)
					return (0);
				saw_digit = 1;
			}
		} else if (ch == '.' && saw_digit) {
			if (octets == 4)
				return (0);
			*++tp = 0;
			saw_digit = 0;
		} else
			return (0);
	}

	if (octets < 4)
		return (0);

	memcpy (dst, tmp, NS_INADDRSZ);
	return (1);
}

#ifdef AF_INET6
static int
inet_pton6 (const char		*src,
	    unsigned char	*dst)
{
	const char		xdigits_l[] = "0123456789abcdef",
				xdigits_u[] = "0123456789ABCDEF";
	unsigned char		tmp[NS_IN6ADDRSZ], *tp, *endp, *colonp;
	const char		*xdigits, *curtok;
	int			ch, saw_xdigit;
	unsigned int		val;

	memset ((tp = tmp), '\0', NS_IN6ADDRSZ);
	endp = tp + NS_IN6ADDRSZ;
	colonp = NULL;
	/* Leading :: requires some special handling */
	if (*src == ':')
		if (*++src != ':')
			return (0);
	curtok = src;
	saw_xdigit = 0;
	val = 0;

	while ((ch = *src++) != '\0') {
		const char *pch;

		if ((pch = strchr ((xdigits = xdigits_l), ch)) == NULL)
			pch = strchr ((xdigits = xdigits_u), ch);
		if (pch != NULL) {
			val <<= 4;
			val |= (pch - xdigits);
			if (val > 0xffff)
				return (0);
			saw_xdigit = 1;
			continue;
		}
		if (ch == ':') {
			curtok = src;
			if (!saw_xdigit) {
				if (colonp)
					return (0);
				colonp = tp;
				continue;
			}
			if (tp + NS_INT16SZ > endp)
				return (0);
			*tp++ = (unsigned char) (val >> 8) & 0xff;
			*tp++ = (unsigned char) val & 0xff;
			saw_xdigit = 0;
			val = 0;
			continue;
		}
		if (ch == '.' && ((tp + NS_INADDRSZ) <= endp) &&
		    inet_pton4 (curtok, tp) > 0) {
			tp += NS_INADDRSZ;
			saw_xdigit = 0;
			break;	/* '\0' was seen by inet_pton4 () */
		}
		return (0);
	}
	if (saw_xdigit) {
		if (tp + NS_INT16SZ > endp)
			return (0);
		*tp++ = (unsigned char) (val >> 8) & 0xff;
		*tp++ = (unsigned char) val & 0xff;
	}
	if (colonp != NULL) {
		/* Since some memmove()'s erroneously fail to handle
		 * overlapping regions, we'll do the shift by hand */
		const int n = tp - colonp;
		pint i;

		for (i = 1; i <= n; i++) {
			endp[- i] = colonp[n - i];
			colonp[n - i] = 0;
		}
		tp = endp;
	}
	if (tp != endp)
		return (0);
	memcpy (dst, tmp, NS_IN6ADDRSZ);
	return (1);
}
#endif

