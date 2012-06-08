/* 
 * 17.09.2010
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
 * @file psocket.h
 * @brief Socket implementation
 * @author Alexander Saprykin
 *
 * #PSocket is a socket implementation which supports INET and INET6 families,
 * TCP and UDP protocols. It's fast and non-blocking by default. Note that before
 * using #PSocket API you must call p_lib_init() in order to initialize system
 * resources (on UNIX this will do nothing, but on Windows this routine is required),
 * and p_lib_shutdown() after #PSocket API usage. Commonly first routine called on
 * the program's start, and second one is before application termination. Here is an
 * example of #PSocket usage:
 * @code
 * PSocketAddress *addr;
 * PSocket	  *sock;
 * 
 * p_lib_init();
 * ...
 * if ((addr = p_socket_address_new("127.0.0.1", 5432)) == NULL) {
 *	...
 * }
 * 
 * if ((sock = p_socket_new(P_SOCKET_FAMILY_INET, P_SOCKET_TYPE_DATAGRAM, P_SOCKET_PROTOCOL_UDP)) == NULL) {
 *	p_socket_address_free(addr);
 *	...
 * }
 *
 * if (!p_socket_bind (sock, addr, FALSE)) {
 *	p_socket_address_free(addr);
 *	p_socket_free(sock);
 *	...
 * }
 *
 * ...
 * p_socket_address_free(addr);
 * p_socket_close(sock);
 * p_socket_free(sock);
 * p_lib_shutdown();
 * @endcode
 * Here was created UDP socket, which then was binded to localhost address and port 5432.
 * Do not forget to close socket and free memory after its usage.
 */

#if !defined (__PLIB_H_INSIDE__) && !defined (PLIB_COMPILATION)
#  error "Header files shouldn't be included directly, consider using <plib.h> instead."
#endif

#ifndef __PSOCKET_H__
#define __PSOCKET_H__

#include <pmacros.h>
#include <psocketaddress.h>

P_BEGIN_DECLS

/** Socket protocols specified by IANA  */
typedef enum _PSocketProtocol {
	P_SOCKET_PROTOCOL_UNKNOWN		= -1,	/**< Unknown protocol.	*/
	P_SOCKET_PROTOCOL_DEFAULT		= 0,	/**< Default protocol.	*/
	P_SOCKET_PROTOCOL_TCP			= 6,	/**< TCP protocol.	*/
	P_SOCKET_PROTOCOL_UDP			= 17	/**< UDP protocol.	*/
} PSocketProtocol;

/** Socket errors */
typedef enum _PSocketError {
	P_SOCKET_ERROR_NONE			= 0,	/**< No error.					*/
	P_SOCKET_ERROR_NO_RESOURCES		= 1,	/**< OS hasn't enough resources.		*/
	P_SOCKET_ERROR_NOT_AVAILABLE		= 2,	/**< Resource isn't available.			*/
	P_SOCKET_ERROR_ACCESS_DENIED		= 3,	/**< Access denied.				*/
	P_SOCKET_ERROR_CONNECTED		= 4,	/**< Already connected.				*/
	P_SOCKET_ERROR_CONNECTING		= 5,	/**< Connection in progress.			*/
	P_SOCKET_ERROR_ABORTED			= 6,	/**< Operation aborted.				*/
	P_SOCKET_ERROR_INVALID_ARGUMENT		= 7,	/**< Invalid argument specified.		*/
	P_SOCKET_ERROR_NOT_SUPPORTED		= 8,	/**< Operation not supported.			*/
	P_SOCKET_ERROR_TIMED_OUT		= 9,	/**< Operation timed out.			*/
	P_SOCKET_ERROR_WOULD_BLOCK		= 10,	/**< Operation cannot be completed immediatly.	*/
	P_SOCKET_ERROR_ADDRESS_IN_USE		= 11,	/**< Address is already under usage.		*/
	P_SOCKET_ERROR_CONNECTION_REFUSED	= 12,	/**< Connection refused.			*/
	P_SOCKET_ERROR_NOT_CONNECTED		= 13,	/**< Connection required first.			*/
	P_SOCKET_ERROR_FAILED			= 14	/**< General error.				*/
} PSocketError;

/** Socket types */
typedef enum _PSocketType {
	P_SOCKET_TYPE_UNKNOWN			= 0,	/**< Unknown type.	*/
	P_SOCKET_TYPE_STREAM			= 1,	/**< Stream type.	*/
	P_SOCKET_TYPE_DATAGRAM			= 2,	/**< Datagram type.	*/
	P_SOCKET_TYPE_SEQPACKET			= 3	/**< SeqPacket type.	*/
} PSocketType;

/** Socket direction for data operations */
typedef enum _PSocketDirection {
	P_SOCKET_DIRECTION_SND			= 0,	/**< Send direction.	*/
	P_SOCKET_DIRECTION_RCV			= 1	/**< Receive direction.	*/
} PSocketDirection;

typedef struct _PSocket PSocket;

/**
 * @brief Creates new #PSocket from file descriptor.
 * @param fd File descriptor to create socket from.
 * @return Pointer to #PSocket in case of success, NULL otherwise.
 * @since 0.0.1
 */
P_LIB_API PSocket *		p_socket_new_from_fd		(pint 			fd);

/**
 * @brief Creates new #PSocket object.
 * @param family Socket family.
 * @param type Socket type.
 * @param protocol Socket data transfer protocol.
 * @return Pointer to #PSocket in case of success, NULL otherwise.
 * @since 0.0.1
 * @note If all given parameters are not compatible with each other, then function
 * will fail.
 * @sa #PSocketFamily, #PSocketType, #PSocketProtocol
 */
P_LIB_API PSocket *		p_socket_new 			(PSocketFamily		family,
								 PSocketType		type,
								 PSocketProtocol	protocol);

/**
 * @brief Gets underlying file descriptor of the socket.
 * @param socket #PSocket to get file descriptor for.
 * @return File descriptor in case of success, NULL otherwise.
 * @since 0.0.1
 */
P_LIB_API pint			p_socket_get_fd 		(PSocket 		*socket);

/**
 * @brief Gets socket's family.
 * @param socket #PSocket to get family for.
 * @return #PSocketFamily in case of success, #P_SOCKET_FAMILY_UNKNOWN otherwise.
 * @since 0.0.1
 */
P_LIB_API PSocketFamily		p_socket_get_family 		(PSocket 		*socket);

/**
 * @brief Gets socket's type.
 * @param socket #PSocket to get type for.
 * @return #PSocketType in case of success, #P_SOCKET_TYPE_UNKNOWN otherwise.
 * @since 0.0.1
 */
P_LIB_API PSocketType		p_socket_get_type 		(PSocket 		*socket);

/**
 * @brief Gets socket's protocol.
 * @param socket #PSocket to get protocol for.
 * @return #PSocketProtocol in case of success, #P_SOCKET_PROTOCOL_UNKNOWN otherwise.
 * @since 0.0.1
 */
P_LIB_API PSocketProtocol	p_socket_get_protocol		(PSocket 		*socket);

/**
 * @brief Checks whether keep alive flag is enabled.
 * @param socket #PSocket to check keep alive flag for.
 * @return TRUE if keep alive flag is enabled, FALSE otherwise.
 * @since 0.0.1
 */
P_LIB_API pboolean		p_socket_get_keepalive		(PSocket 		*socket);

/**
 * @brief Checks whether socket is blocking.
 * @param socket #PSocket to check blocking for.
 * @return TRUE if @a socket is blocking, FALSE otherwise.
 * @note Blocking socket will wait for I/O operation to be completed before
 * returning to caller function.
 * @since 0.0.1
 */
P_LIB_API pboolean		p_socket_get_blocking		(PSocket 		*socket);

/**
 * @brief Gets socket's listen backlog parameter.
 * @param socket #PSocket to get listen backlog parameter for.
 * @return Listen backlog parameter in case of success, -1 otherwise.
 * @since 0.0.1
 */
P_LIB_API pint			p_socket_get_listen_backlog	(PSocket 		*socket);

/**
 * @brief Gets socket's local address.
 * @param socket #PSocket to get local address for.
 * @return #PSocketAddress with socket's local address in case of success,
 * NULL otherwise.
 * @since 0.0.1
 */
P_LIB_API PSocketAddress *	p_socket_get_local_address	(PSocket 		*socket);

/**
 * @brief Gets socket's remote address.
 * @param socket #PSocket to get remote address for.
 * @return #PSocketAddress with socket's remote address in case of success,
 * NULL otherwise.
 * @since 0.0.1
 */
P_LIB_API PSocketAddress *	p_socket_get_remote_address	(PSocket 		*socket);

/**
 * @brief Checks whether socket is connected.
 * @param socket #PSocket to check connection for.
 * @return TRUE if @a socket is connected, FALSE otherwise.
 * @since 0.0.1
 */
P_LIB_API pboolean		p_socket_is_connected		(PSocket 		*socket);

/**
 * @brief Sets socket's keep alive flag.
 * @param socket #PSocket to set keep alive flag for.
 * @param keepalive Value for keep alive flag.
 * @since 0.0.1
 */
P_LIB_API void			p_socket_set_keepalive		(PSocket 		*socket,
								 pboolean		keepalive);

/**
 * @brief Sets socket's blocking mode.
 * @param socket #PSocket to set blocking mode for.
 * @param blocking Whether to set @a socket to blocking.
 * @note Blocking socket will wait for I/O operation to be completed before
 * returning to caller function.
 * @since 0.0.1
 */
P_LIB_API void			p_socket_set_blocking		(PSocket 		*socket,
								 pboolean		blocking);

/**
 * @brief Sets socket's listen backlog parameter.
 * @param socket #PSocket to set listen backlog parameter for.
 * @param backlog Value for listen backlog parameter.
 * @since 0.0.1
 */
P_LIB_API void			p_socket_set_listen_backlog	(PSocket		*socket,
								 pint			backlog);

/**
 * @brief Binds socket to given address.
 * @param socket #PSocket to bind.
 * @param address #PSocketAddress to bind to given @a socket.
 * @param allow_reuse Whether to allow socket's address reusing.
 * @return TRUE in case of success, FALSE otherwise.
 * @since 0.0.1
 */
P_LIB_API pboolean		p_socket_bind			(PSocket 		*socket,
								 PSocketAddress		*address,
								 pboolean		allow_reuse);

/**
 * @brief Connects socket to given remote address.
 * @param socket #PSocket to connect.
 * @param address #PSocketAddress to connect @a socket to.
 * @return TRUE in case of success, FALSE otherwise.
 * @since 0.0.1
 */
P_LIB_API pboolean		p_socket_connect		(PSocket		*socket,
								 PSocketAddress		*address);

/**
 * @brief Checks socket's connection state.
 * @param socket #PSocket to check connection for.
 * @return TRUE if @a socket is connected, FALSE otherwise.
 * @since 0.0.1
 */
P_LIB_API pboolean		p_socket_check_connect_result	(PSocket 		*socket);

/**
 * @brief Puts socket in listen state.
 * @param socket #PSocket to start listening.
 * @return TRUE in case of success, FALSE otherwise.
 * @since 0.0.1
 */
P_LIB_API pboolean		p_socket_listen			(PSocket 		*socket);

/**
 * @brief Accepts socket's connection.
 * @param socket #PSocket to accept connection from.
 * @return #PSocket with accepted connection in case of success, NULL otherwise.
 * @since 0.0.1
 */
P_LIB_API PSocket *		p_socket_accept			(PSocket		*socket);

/**
 * @brief Receives data from given socket.
 * @param socket #PSocket to receive data from.
 * @param buffer Buffer to write received data in.
 * @param buflen Length of @a buffer.
 * @return Size of written data in case of success, -1 otherwise.
 * @note If @a socket is in blocking mode, then the caller will be blocked
 * until data arrived.
 * @since 0.0.1
 */
P_LIB_API pssize		p_socket_receive		(PSocket		*socket,
								 pchar			*buffer,
								 psize			buflen);

/**
 * @brief Receives data from given socket and saves remote address.
 * @param socket #PSocket to receive data from.
 * @param address Pointer to store remote address in case of success,
 * caller is responsible to free it after using.
 * @param buffer Buffer to write received data in.
 * @param buflen Length of @a buffer.
 * @return Size of written data in case of success, -1 otherwise.
 * @note If @a socket is in blocking mode, then the caller will be blocked
 * until data arrived.
 * @since 0.0.1
 */
P_LIB_API pssize		p_socket_receive_from		(PSocket 		*socket,
								 PSocketAddress		**address,
								 pchar 			*buffer,
								 psize			buflen);

/**
 * @brief Sends data through given socket.
 * @param socket #PSocket to send data through.
 * @param buffer Buffer with data to send.
 * @param buflen Length of @a buffer.
 * @return Size of sent data.
 * @note If @a socket is in blocking mode, then the caller will be blocked
 * until data sent.
 * @since 0.0.1
 */
P_LIB_API pssize		p_socket_send			(PSocket		*socket,
								 const pchar		*buffer,
								 psize			buflen);

/**
 * @brief Sends data through given socket to given address.
 * @param socket #PSocket to send data through.
 * @param address #PSocketAddress to send data to.
 * @param buffer Buffer with data to send.
 * @param buflen Length of @a buffer.
 * @return Size of sent data.
 * @note If @a socket is in blocking mode, then the caller will be blocked
 * until data sent.
 * @since 0.0.1
 */
P_LIB_API pssize		p_socket_send_to		(PSocket		*socket,
								 PSocketAddress		*address,
								 const pchar		*buffer,
								 psize			buflen);

/**
 * @brief Closes socket.
 * @param socket #PSocket to close.
 * @return TRUE in case of success, false otherwise.
 * @since 0.0.1
 */
P_LIB_API pboolean		p_socket_close			(PSocket		*socket);

/**
 * @brief Shutdowns socket's data transfer directions.
 * @param shutdown_read Whether to shutdown read data transfer direction.
 * @param shutdown_write Whether to shutdown write data transfer direction.
 * @return TRUE in case of success, FALSE otherwise.
 * @since 0.0.1
 */
P_LIB_API pboolean		p_socket_shutdown		(PSocket		*socket,
								 pboolean		shutdown_read,
								 pboolean		shutdown_write);

/**
 * @brief Frees socket's resources (and closes it).
 * @param socket #PSocket to close.
 * @since 0.0.1
 */
P_LIB_API void			p_socket_free			(PSocket 		*socket);

/**
 * @brief Gets last socket's error.
 * @param socket #PSocket to get error for.
 * @return Last error occurred.
 * @since 0.0.1
 */
P_LIB_API PSocketError		p_socket_get_last_error		(PSocket		*socket);

/**
 * @brief Sets socket's buffer for given data transfer direction.
 * @param socket #PSocket to set buffer for.
 * @param dir Direction to set buffer on.
 * @param size Size of buffer to set.
 * @return TRUE in case of success, FALSE otherwise.
 * @since 0.0.1
 */
P_LIB_API pboolean		p_socket_set_buffer_size	(PSocket		*socket,
								 PSocketDirection	dir,
								 psize			size);

P_END_DECLS

#endif /* __PSOCKET_H__ */

