/* 
 * 14.09.2010
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

#ifndef __PSOCKETADDRESS_H__
#define __PSOCKETADDRESS_H__

#include <pmacros.h>
#include <ptypes.h>

#ifdef P_OS_WIN
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#endif

typedef enum _PSocketFamily {
	P_SOCKET_FAMILY_UNKNOWN = 0,
#ifdef AF_INET6
	P_SOCKET_FAMILY_INET	= AF_INET,
	P_SOCKET_FAMILY_INET6	= AF_INET6
#else
	P_SOCKET_FAMILY_INET	= AF_INET
#endif
} PSocketFamily;

typedef struct _PSocketAddress {
	PSocketFamily	family;
	union _addr {
		struct in_addr sin_addr;
#ifdef AF_INET6
		struct in6_addr sin6_addr;
#endif
	} 		addr;
	puint16 	port;
} PSocketAddress;

P_LIB_API PSocketAddress *	p_socket_address_new_from_native	(ppointer	native,
									 psize		len);
P_LIB_API PSocketAddress *	p_socket_address_new			(pchar		*address,
									 puint16	port);
P_LIB_API PSocketAddress *	p_socket_address_new_any		(PSocketFamily	family,
									 puint16	port);
P_LIB_API PSocketAddress *	p_socket_address_new_loopback		(PSocketFamily	family,
									 puint16	port);
P_LIB_API pboolean		p_socket_address_to_native		(PSocketAddress	*addr,
									 ppointer	dest,
									 psize		len);
P_LIB_API psize			p_socket_address_get_native_size	(PSocketAddress *addr);
P_LIB_API PSocketFamily		p_socket_address_get_family		(PSocketAddress *addr);
P_LIB_API pchar *		p_socket_address_get_address		(PSocketAddress *addr);
P_LIB_API puint16		p_socket_address_get_port		(PSocketAddress *addr);
P_LIB_API pboolean		p_socket_address_is_any			(PSocketAddress *addr);
P_LIB_API pboolean		p_socket_address_is_loopback		(PSocketAddress *addr);

#endif /* __PSOCKETADDRESS_H__ */

