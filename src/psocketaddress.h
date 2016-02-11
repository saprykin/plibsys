/*
 * Copyright (C) 2010-2013 Alexander Saprykin <xelfium@gmail.com>
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
 * @file psocketaddress.h
 * @brief Socket address wrapper
 * @author Alexander Saprykin
 *
 * #PSocketAddress is a socket address wrapper depending on the underlying system.
 * Currently it supports IPv4 and IPv6 addresses, which consist of IP address and
 * corresponding port number.
 * Also you can use convenient methods to create addresses for loopback interface
 * using p_socket_address_new_loopback() or to specify address to any interface
 * using p_socket_address_new_any(). If you want to get native socket address data
 * for further usage in system calls use p_socket_address_to_native() and vice versa.
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

P_BEGIN_DECLS

/** Socket family */
typedef enum _PSocketFamily {
	P_SOCKET_FAMILY_UNKNOWN = 0,		/**< Unknown family.	*/
#ifdef AF_INET6
	P_SOCKET_FAMILY_INET	= AF_INET,	/**< IPv4 family.	*/
	P_SOCKET_FAMILY_INET6	= AF_INET6	/**< IPv6 family	*/
#else
	P_SOCKET_FAMILY_INET	= AF_INET	/**< IPv4 family.	*/
#endif
} PSocketFamily;

/** Typedef for opaque socket address structure */
typedef struct _PSocketAddress PSocketAddress;

/**
 * @brief Creates new #PSocketAddress from native socket address raw data.
 * @param native Pointer to native socket address raw data.
 * @param len Raw data length, in bytes.
 * @return Pointer to #PSocketAddress in case of success, NULL otherwise.
 * @since 0.0.1
 */
P_LIB_API PSocketAddress *	p_socket_address_new_from_native	(pconstpointer	native,
									 psize		len);

/**
 * @brief Creates new #PSocketAddress. Detects family automatically.
 * @param address String representation of the address, eg. 172.146.45.5.
 * @param port Port number.
 * @return Pointer to #PSocketAddress in case of success, NULL otherwise.
 * @since 0.0.1
 */
P_LIB_API PSocketAddress *	p_socket_address_new			(const pchar	*address,
									 puint16	port);

/**
 * @brief Creates new #PSocketAddress for any address representation.
 * @param family Address family.
 * @param port Port number.
 * @return Pointer to #PSocketAddress in case of success, NULL otherwise.
 * @since 0.0.1
 * @note This call creates network address for a set of all possible addresses,
 * so you can't use it for receiving or sending data on a particular network
 * address. If you need to bind a socket to specific address (e.g. 127.0.0.1)
 * use p_socket_address_new() instead.
 */
P_LIB_API PSocketAddress *	p_socket_address_new_any		(PSocketFamily	family,
									 puint16	port);

/**
 * @brief Creates new #PSocketAddress for loopback interface.
 * @param family Address family.
 * @param port Port number.
 * @return Pointer to #PSocketAddress in case of success, NULL otherwise.
 * @since 0.0.1
 * @note This call creates network address for an entire loopback network interface,
 * so you can't use it for receiving or sending data on a particular network
 * address. If you need to bind a socket to specific address (e.g. 127.0.0.1)
 * use p_socket_address_new() instead.
 */
P_LIB_API PSocketAddress *	p_socket_address_new_loopback		(PSocketFamily	family,
									 puint16	port);

/**
 * @brief Converts #PSocketAddress to native socket address raw data.
 * @param addr #PSocketAddress to convert.
 * @param[out] dest Output buffer for raw data.
 * @param destlen Length of the @a dest buffer.
 * @return TRUE in case of success, FALSE otherwise.
 * @since 0.0.1
 */
P_LIB_API pboolean		p_socket_address_to_native		(const PSocketAddress	*addr,
									 ppointer		dest,
									 psize			destlen);

/**
 * @brief Gets size of the native socket address raw data, in bytes.
 * @param addr #PSocketAddress to get size of the native address raw data for.
 * @return Size of the native socket address raw data in case of success, 0 otherwise.
 * @since 0.0.1
 */
P_LIB_API psize			p_socket_address_get_native_size	(const PSocketAddress *addr);

/**
 * @brief Gets family of the socket address.
 * @param addr #PSocketAddress to get family for.
 * @return #PSocketFamily of the socket address.
 * @since 0.0.1
 */
P_LIB_API PSocketFamily		p_socket_address_get_family		(const PSocketAddress *addr);

/**
 * @brief Gets socket address in string representation, eg. 172.146.45.5.
 * @param addr #PSocketAddress to get address string for.
 * @return Pointer to string representation of the socket address in case of
 * success, NULL otherwise. Caller takes ownership for the returned pointer.
 * @since 0.0.1
 */
P_LIB_API pchar *		p_socket_address_get_address		(const PSocketAddress *addr);

/**
 * @brief Gets port number of the socket address.
 * @param addr #PSocketAddress to get port number for.
 * @return Port number in case of success, 0 otherwise.
 * @since 0.0.1
 */
P_LIB_API puint16		p_socket_address_get_port		(const PSocketAddress *addr);

/**
 * @brief Checks whether given socket address is for any interface representation.
 * Such address is a 0.0.0.0.
 * @param addr #PSocketAddress to check.
 * @return TRUE if @a addr is for any interface representation, FALSE otherwise.
 * @since 0.0.1
 * @sa p_socket_address_new_any()
 */
P_LIB_API pboolean		p_socket_address_is_any			(const PSocketAddress *addr);

/**
 * @brief Checks whether given socket address is for loopback interface.
 * Such address is a 127.x.x.x.
 * @param addr #PSocketAddress to check.
 * @return TRUE if @a addr is for loopback interface, FALSE otherwise.
 * @since 0.0.1
 * @sa p_socket_address_new_loopback()
 */
P_LIB_API pboolean		p_socket_address_is_loopback		(const PSocketAddress *addr);

/**
 * @brief Frees socket address structure and it's resources.
 * @param addr #PSocketAddress to free.
 * @since 0.0.1
 */
P_LIB_API void			p_socket_address_free			(PSocketAddress	*addr);

P_END_DECLS

#endif /* __PSOCKETADDRESS_H__ */
