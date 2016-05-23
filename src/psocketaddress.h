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

/**
 * @file psocketaddress.h
 * @brief Socket address wrapper
 * @author Alexander Saprykin
 *
 * Socket address is usually represented by a network address (IPv4 or IPv6) and
 * a port number (though some other naming schemes and parameters are possible).
 *
 * Socket address parameters are stored inside a special system (native)
 * structure in a binary (raw) form. Native structure varies with operating
 * system and network protocol. #PSocketAddress acts like a thin wrapper around
 * that native address structure and unifies manipulation of socket address
 * data.
 *
 * #PSocketAddress supports IPv4 and IPv6 addresses which consist of an IP
 * address and a port number. IPv6 support is system dependent and doesn't
 * provided for all platforms. Sometimes you may also need to enable IPv6
 * support in the system to make it working.
 *
 * Convenient methods to create special addresses are provided: for loopback
 * interface use p_socket_address_new_loopback(), for any-address interface
 * use p_socket_address_new_any().
 *
 * If you want to get an underlying native address structure for further usage
 * in system calls use p_socket_address_to_native(), and
 * p_socket_address_new_from_native() for a vice versa conversion.
 */

#if !defined (__PLIBSYS_H_INSIDE__) && !defined (PLIBSYS_COMPILATION)
#  error "Header files shouldn't be included directly, consider using <plibsys.h> instead."
#endif

#ifndef __PSOCKETADDRESS_H__
#define __PSOCKETADDRESS_H__

#include <pmacros.h>
#include <ptypes.h>

#ifdef P_OS_WIN
#  include <winsock2.h>
#  include <ws2tcpip.h>
#  include <windows.h>
#else
#  include <sys/types.h>
#  include <sys/socket.h>
#  include <netinet/in.h>
#endif

P_BEGIN_DECLS

/** Socket address family. */
typedef enum _PSocketFamily {
	P_SOCKET_FAMILY_UNKNOWN = 0,		/**< Unknown family.	*/
#ifdef AF_INET6
	P_SOCKET_FAMILY_INET	= AF_INET,	/**< IPv4 family.	*/
	P_SOCKET_FAMILY_INET6	= AF_INET6	/**< IPv6 family	*/
#else
	P_SOCKET_FAMILY_INET	= AF_INET	/**< IPv4 family.	*/
#endif
} PSocketFamily;

/** Socket address opaque structure. */
typedef struct _PSocketAddress PSocketAddress;

/**
 * @brief Creates new #PSocketAddress from native socket address raw data.
 * @param native Pointer to native socket address raw data.
 * @param len Raw data length, in bytes.
 * @return Pointer to #PSocketAddress in case of success, NULL otherwise.
 * @since 0.0.1
 */
P_LIB_API PSocketAddress *	p_socket_address_new_from_native	(pconstpointer		native,
									 psize			len);

/**
 * @brief Creates new #PSocketAddress.
 * @param address String representation of the address (i.e. "172.146.45.5").
 * @param port Port number.
 * @return Pointer to #PSocketAddress in case of success, NULL otherwise.
 * @since 0.0.1
 * @note It tries to automatically detect a socket family.
 */
P_LIB_API PSocketAddress *	p_socket_address_new			(const pchar		*address,
									 puint16		port);

/**
 * @brief Creates new #PSocketAddress for the any-address representation.
 * @param family Socket family.
 * @param port Port number.
 * @return Pointer to #PSocketAddress in case of success, NULL otherwise.
 * @since 0.0.1
 * @note This call creates a network address for the set of all possible
 * addresses, so you can't use it for receiving or sending data on a particular
 * network address. If you need to bind a socket to the specific address
 * (i.e. 127.0.0.1) use p_socket_address_new() instead.
 */
P_LIB_API PSocketAddress *	p_socket_address_new_any		(PSocketFamily		family,
									 puint16		port);

/**
 * @brief Creates new #PSocketAddress for the loopback interface.
 * @param family Socket family.
 * @param port Port number.
 * @return Pointer to #PSocketAddress in case of success, NULL otherwise.
 * @since 0.0.1
 * @note This call creates a network address for the entire loopback network
 * interface, so you can't use it for receiving or sending data on a particular
 * network address. If you need to bind a socket to the specific address
 * (i.e. 127.0.0.1) use p_socket_address_new() instead.
 */
P_LIB_API PSocketAddress *	p_socket_address_new_loopback		(PSocketFamily		family,
									 puint16		port);

/**
 * @brief Converts #PSocketAddress to native socket address raw data.
 * @param addr #PSocketAddress to convert.
 * @param[out] dest Output buffer for raw data.
 * @param destlen Length in bytes of the @a dest buffer.
 * @return TRUE in case of success, FALSE otherwise.
 * @since 0.0.1
 */
P_LIB_API pboolean		p_socket_address_to_native		(const PSocketAddress	*addr,
									 ppointer		dest,
									 psize			destlen);

/**
 * @brief Gets the size of native socket address raw data, in bytes.
 * @param addr #PSocketAddress to get the size of native address raw data for.
 * @return Size of native socket address raw data in case of success, 0
 * otherwise.
 * @since 0.0.1
 */
P_LIB_API psize			p_socket_address_get_native_size	(const PSocketAddress	*addr);

/**
 * @brief Gets the family of the socket address.
 * @param addr #PSocketAddress to get a family for.
 * @return #PSocketFamily of the socket address.
 * @since 0.0.1
 */
P_LIB_API PSocketFamily		p_socket_address_get_family		(const PSocketAddress	*addr);

/**
 * @brief Gets a socket address in the string representation, i.e.
 * "172.146.45.5".
 * @param addr #PSocketAddress to get address string for.
 * @return Pointer to the string representation of the socket address in case of
 * success, NULL otherwise. Caller takes ownership of the returned pointer.
 * @since 0.0.1
 */
P_LIB_API pchar *		p_socket_address_get_address		(const PSocketAddress	*addr);

/**
 * @brief Gets a port number of the socket address.
 * @param addr #PSocketAddress to get the port number for.
 * @return Port number in case of success, 0 otherwise.
 * @since 0.0.1
 */
P_LIB_API puint16		p_socket_address_get_port		(const PSocketAddress	*addr);

/**
 * @brief Gets IPv6 traffic class and flow information.
 * @param addr #PSocketAddress to get flow information for.
 * @return IPv6 traffic class and flow information.
 * @since 0.0.1
 * @note This call is valid only for an IPv6 address, otherwise 0 is returned.
 */
P_LIB_API puint32		p_socket_address_get_flow_info		(const PSocketAddress	*addr);

/**
 * @brief Gets an IPv6 set of interfaces for a scope.
 * @param addr #PSocketAddress to get a set of interfaces for.
 * @return Index that identifies a set of interfaces for a scope.
 * @since 0.0.1
 * @note This call is valid only for an IPv6 address, otherwise 0 is returned.
 */
P_LIB_API puint32		p_socket_address_get_scope_id		(const PSocketAddress	*addr);

/**
 * @brief Sets IPv6 traffic class and flow information.
 * @param addr #PSocketAddress to set flow information for.
 * @param flow_info Flow information to set.
 * @since 0.0.1
 * @note This call is valid only for an IPv6 address.
 */
P_LIB_API void			p_socket_address_set_flow_info		(PSocketAddress		*addr,
									 puint32		flowinfo);

/**
 * @brief Sets an IPv6 set of interfaces for a scope.
 * @param addr #PSocketAddress to set a set of interfaces for.
 * @param scope_id Index that identifies a set of interfaces for a scope.
 * @since 0.0.1
 * @note This call is valid only for an IPv6 address.
 */
P_LIB_API void			p_socket_address_set_scope_id		(PSocketAddress		*addr,
									 puint32		scope_id);

/**
 * @brief Checks whether a given socket address is the any-address
 * representation. Such address is a 0.0.0.0.
 * @param addr #PSocketAddress to check.
 * @return TRUE if @a addr is the any-address representation, FALSE otherwise.
 * @since 0.0.1
 * @sa p_socket_address_new_any()
 */
P_LIB_API pboolean		p_socket_address_is_any			(const PSocketAddress	*addr);

/**
 * @brief Checks whether a given socket address is for loopback interface. Such
 * address is a 127.x.x.x.
 * @param addr #PSocketAddress to check.
 * @return TRUE if @a addr is for loopback interface, FALSE otherwise.
 * @since 0.0.1
 * @sa p_socket_address_new_loopback()
 */
P_LIB_API pboolean		p_socket_address_is_loopback		(const PSocketAddress	*addr);

/**
 * @brief Frees socket address structure and its resources.
 * @param addr #PSocketAddress to free.
 * @since 0.0.1
 */
P_LIB_API void			p_socket_address_free			(PSocketAddress		*addr);

P_END_DECLS

#endif /* __PSOCKETADDRESS_H__ */
