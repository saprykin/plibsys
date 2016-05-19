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
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

#include "plibsys-private.h"
#include "pmem.h"
#include "psocketaddress.h"
#include "pstring.h"

#include <stdlib.h>
#include <string.h>

#ifndef P_OS_WIN
#  include <arpa/inet.h>
#endif

/* According to Open Group specifications */
#ifndef INET_ADDRSTRLEN
#  ifdef P_OS_WIN
     /* On Windows it includes port number  */
#    define INET_ADDRSTRLEN 22
#  else
#    define INET_ADDRSTRLEN 16
#  endif
#endif

#ifdef AF_INET6
#  ifndef INET6_ADDRSTRLEN
#    ifdef P_OS_WIN
       /* On Windows it includes port number */
#      define INET6_ADDRSTRLEN 65
#    else
#      define INET6_ADDRSTRLEN 46
#    endif
#  endif
#endif

struct _PSocketAddress {
	PSocketFamily	family;
	union _addr {
		struct in_addr sin_addr;
#ifdef AF_INET6
		struct in6_addr sin6_addr;
#endif
	} 		addr;
	puint16 	port;
};

P_LIB_API PSocketAddress *
p_socket_address_new_from_native (pconstpointer	native,
				  psize		len)
{
	PSocketAddress	*ret;
	puint16		family;

	if (P_UNLIKELY (native == NULL || len == 0))
		return NULL;

	if (P_UNLIKELY ((ret = p_malloc0 (sizeof (PSocketAddress))) == NULL))
		return NULL;

	family = ((struct sockaddr *) native)->sa_family;

	if (family == AF_INET) {
		if (len < sizeof (struct sockaddr_in)) {
			P_WARNING ("PSocketAddress: not enough space to convert from native");
			p_free (ret);
			return NULL;
		}

		memcpy (&ret->addr.sin_addr, &((struct sockaddr_in *) native)->sin_addr, sizeof (struct in_addr));
		ret->family = P_SOCKET_FAMILY_INET;
		ret->port = ntohs (((struct sockaddr_in *) native)->sin_port);
		return ret;
	}
#ifdef AF_INET6
	else if (family == AF_INET6) {
		if (len < sizeof (struct sockaddr_in6)) {
			P_WARNING ("PSocketAddress: not enough space to convert from native");
			p_free (ret);
			return NULL;
		}

		memcpy (&ret->addr.sin6_addr, &((struct sockaddr_in6 *) native)->sin6_addr, sizeof (struct in6_addr));
		ret->family = P_SOCKET_FAMILY_INET6;
		ret->port = ntohs (((struct sockaddr_in *) native)->sin_port);
		return ret;
	}
#endif
	else {
		p_free (ret);
		return NULL;
	}
}

P_LIB_API PSocketAddress *
p_socket_address_new (const pchar	*address,
		      puint16		port)
{
	PSocketAddress		*ret;
#ifdef P_OS_WIN
	struct sockaddr_storage	sa;
	struct sockaddr_in 	*sin = (struct sockaddr_in *) &sa;
#  ifdef AF_INET6
	struct sockaddr_in6 	*sin6 = (struct sockaddr_in6 *) &sa;
#  endif /* AF_INET6 */
	pint 			len;
#endif /* P_OS_WIN */

	if (P_UNLIKELY (address == NULL))
		return NULL;

	if (P_UNLIKELY ((ret = p_malloc0 (sizeof (PSocketAddress))) == NULL)) {
		P_ERROR ("PSocketAddress: failed to allocate memory");
		return NULL;
	}

	ret->port = port;

#ifdef P_OS_WIN
	memset (&sa, 0, sizeof (sa));
	len = sizeof (sa);
	sin->sin_family = AF_INET;

	if (WSAStringToAddressA ((LPSTR) address, AF_INET, NULL, (LPSOCKADDR) &sa, &len) == 0) {
		memcpy (&ret->addr.sin_addr, &sin->sin_addr, sizeof (struct in_addr));
		ret->family = P_SOCKET_FAMILY_INET;
		return ret;
	}
#  ifdef AF_INET6
	else {
		sin6->sin6_family = AF_INET6;

		if (WSAStringToAddressA ((LPSTR) address, AF_INET6, NULL, (LPSOCKADDR) &sa, &len) == 0) {
			memcpy (&ret->addr.sin6_addr, &sin6->sin6_addr, sizeof (struct in6_addr));
			ret->family = P_SOCKET_FAMILY_INET6;
			return ret;
		}
	}
#  endif /* AF_INET6 */
#else /* P_OS_WIN */
	if (inet_pton (AF_INET, address, &ret->addr.sin_addr) > 0) {
		ret->family = P_SOCKET_FAMILY_INET;
		return ret;
	}
#  ifdef AF_INET6
	else if (inet_pton (AF_INET6, address, &ret->addr.sin6_addr) > 0) {
		ret->family = P_SOCKET_FAMILY_INET6;
		return ret;
	}
#  endif /* AF_INET6 */
#endif /* P_OS_WIN */

	p_free (ret);
	return NULL;
}

P_LIB_API PSocketAddress *
p_socket_address_new_any (PSocketFamily	family,
			  puint16	port)
{
	PSocketAddress	*ret;
	puchar		any_addr[] = {0, 0, 0, 0};
#ifdef AF_INET6
	struct in6_addr	any6_addr = IN6ADDR_ANY_INIT;
#endif

	if (P_UNLIKELY ((ret = p_malloc0 (sizeof (PSocketAddress))) == NULL)) {
		P_ERROR ("PSocketAddress: failed to allocate memory");
		return NULL;
	}

	if (family == P_SOCKET_FAMILY_INET)
		memcpy (&ret->addr.sin_addr, any_addr, sizeof (any_addr));
#ifdef AF_INET6
	else if (family == P_SOCKET_FAMILY_INET6)
		memcpy (&ret->addr.sin6_addr, &any6_addr.s6_addr, sizeof (any6_addr.s6_addr));
#endif
	else {
		p_free (ret);
		return NULL;
	}

	ret->family = family;
	ret->port = port;

	return ret;
}

P_LIB_API PSocketAddress *
p_socket_address_new_loopback (PSocketFamily	family,
			       puint16		port)
{
	PSocketAddress	*ret;
	puchar		loop_addr[] = {127, 0, 0, 0};
#ifdef AF_INET6
	struct in6_addr	loop6_addr = IN6ADDR_LOOPBACK_INIT;
#endif

	if (P_UNLIKELY ((ret = p_malloc0 (sizeof (PSocketAddress))) == NULL)) {
		P_ERROR ("PSocketAddress: failed to allocate memory");
		return NULL;
	}

	if (family == P_SOCKET_FAMILY_INET)
		memcpy (&ret->addr.sin_addr, loop_addr, sizeof (loop_addr));
#ifdef AF_INET6
	else if (family == P_SOCKET_FAMILY_INET6)
		memcpy (&ret->addr.sin6_addr, &loop6_addr.s6_addr, sizeof (loop6_addr.s6_addr));
#endif
	else {
		p_free (ret);
		return NULL;
	}

	ret->family = family;
	ret->port = port;

	return ret;
}

P_LIB_API pboolean
p_socket_address_to_native (const PSocketAddress	*addr,
			    ppointer			dest,
			    psize			destlen)
{
	struct sockaddr_in	*sin;
#ifdef AF_INET6
	struct sockaddr_in6	*sin6;
#endif

	if (P_UNLIKELY (addr == NULL || dest == NULL || destlen == 0))
		return FALSE;

	sin = (struct sockaddr_in *) dest;
#ifdef AF_INET6
	sin6 = (struct sockaddr_in6 *) dest;
#endif

	if (addr->family == P_SOCKET_FAMILY_INET) {
		if (P_UNLIKELY (destlen < sizeof (struct sockaddr_in))) {
			P_WARNING ("PSocketAddress: not enough space to convert to native");
			return FALSE;
		}

		memcpy (&sin->sin_addr, &addr->addr.sin_addr, sizeof (struct in_addr));
		sin->sin_family = AF_INET;
		sin->sin_port = htons (addr->port);
		memset (sin->sin_zero, 0, sizeof (sin->sin_zero));
		return TRUE;
	}
#ifdef AF_INET6
	else if (addr->family == P_SOCKET_FAMILY_INET6) {
		if (P_UNLIKELY (destlen < sizeof (struct sockaddr_in6))) {
			P_ERROR ("PSocketAddress: not enough space to convert to native");
			return FALSE;
		}

		memcpy (&sin6->sin6_addr, &addr->addr.sin6_addr, sizeof (struct in6_addr));
		sin->sin_family = AF_INET6;
		sin->sin_port = htons (addr->port);
		return TRUE;
	}
#endif
	else {
		P_WARNING ("PSocketAddress: unsupported socket address");
		return FALSE;
	}
}

P_LIB_API psize
p_socket_address_get_native_size (const PSocketAddress *addr)
{
	if (P_UNLIKELY (addr == NULL))
		return 0;

	if (addr->family == P_SOCKET_FAMILY_INET)
		return sizeof (struct sockaddr_in);
#ifdef AF_INET6
	else if (addr->family == P_SOCKET_FAMILY_INET6)
		return sizeof (struct sockaddr_in6);
#endif
	else {
		P_WARNING ("PSocketAddress: can't get native size for unsupported family");
		return 0;
	}
}

P_LIB_API PSocketFamily
p_socket_address_get_family (const PSocketAddress *addr)
{
	if (P_UNLIKELY (addr == NULL))
		return P_SOCKET_FAMILY_UNKNOWN;

	return addr->family;
}

P_LIB_API pchar *
p_socket_address_get_address (const PSocketAddress *addr)
{
#ifdef AF_INET6
	pchar			buffer[INET6_ADDRSTRLEN];
#else
	pchar			buffer[INET_ADDRSTRLEN];
#endif

#ifdef P_OS_WIN
	DWORD			buflen = sizeof (buffer);
	DWORD			addrlen;
	struct sockaddr_storage	sa;
	struct sockaddr_in	*sin;
#  ifdef AF_INET6
	struct sockaddr_in6	*sin6;
#  endif /* AF_INET6 */
#endif /* P_OS_WIN */

	if (P_UNLIKELY (addr == NULL || addr->family == P_SOCKET_FAMILY_UNKNOWN))
		return NULL;
#ifdef P_OS_WIN
	sin = (struct sockaddr_in *) &sa;
#  ifdef AF_INET6
	sin6 = (struct sockaddr_in6 *) &sa;
#  endif /* AF_INET6 */
#endif /* P_OS_WIN */

#ifdef P_OS_WIN
	memset (&sa, 0, sizeof (sa));
#endif

#ifdef P_OS_WIN
	sa.ss_family = addr->family;

	if (addr->family == P_SOCKET_FAMILY_INET) {
		addrlen = sizeof (struct sockaddr_in);
		memcpy (&sin->sin_addr, &addr->addr.sin_addr, sizeof (struct in_addr));
	}
#  ifdef AF_INET6
	else {
		addrlen = sizeof (struct sockaddr_in6);
		memcpy (&sin6->sin6_addr, &addr->addr.sin6_addr, sizeof (struct in6_addr));
	}
#  endif /* AF_INET6 */

	if (P_UNLIKELY (WSAAddressToStringA ((LPSOCKADDR) &sa,
					     addrlen,
					     NULL,
					     (LPSTR) buffer,
					     &buflen) != 0))
		return NULL;
#else /* !P_OS_WIN */
	if (addr->family == P_SOCKET_FAMILY_INET)
		inet_ntop (AF_INET, &addr->addr.sin_addr, buffer, sizeof (buffer));
#  ifdef AF_INET6
	else
		inet_ntop (AF_INET6, &addr->addr.sin6_addr, buffer, sizeof (buffer));
#  endif /* AF_INET6 */
#endif /* P_OS_WIN */

	return p_strdup (buffer);
}

P_LIB_API puint16
p_socket_address_get_port (const PSocketAddress *addr)
{
	if (P_UNLIKELY (addr == NULL))
		return 0;

	return addr->port;
}

P_LIB_API pboolean
p_socket_address_is_any (const PSocketAddress *addr)
{
	puint32 addr4;

	if (P_UNLIKELY (addr == NULL || addr->family == P_SOCKET_FAMILY_UNKNOWN))
		return FALSE;

	if (addr->family == P_SOCKET_FAMILY_INET) {
		addr4 = ntohl (* ((puint32 *) &addr->addr.sin_addr));

		return (addr4 == INADDR_ANY);
	}
#ifdef AF_INET6
	else
		return IN6_IS_ADDR_UNSPECIFIED (&addr->addr.sin6_addr);
#else
	else
		return FALSE;
#endif
}

P_LIB_API pboolean
p_socket_address_is_loopback (const PSocketAddress *addr)
{
	puint32 addr4;

	if (P_UNLIKELY (addr == NULL || addr->family == P_SOCKET_FAMILY_UNKNOWN))
		return FALSE;

	if (addr->family == P_SOCKET_FAMILY_INET) {
		addr4 = ntohl (* ((puint32 *) &addr->addr.sin_addr));

		/* 127.0.0.0/8 */
		return ((addr4 & 0xff000000) == 0x7f000000);
	}
#ifdef AF_INET6
	else
		return IN6_IS_ADDR_LOOPBACK (&addr->addr.sin6_addr);
#else
	else
		return FALSE;
#endif
}

P_LIB_API void
p_socket_address_free (PSocketAddress *addr)
{
	if (P_UNLIKELY (addr == NULL))
		return;

	p_free (addr);
}
