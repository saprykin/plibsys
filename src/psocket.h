/* 
 * Copyright (C) 2010-2014 Alexander Saprykin <xelfium@gmail.com>
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
 * Socket is a communication primitive, usually working over network.
 * There are two kind of sockets:
 * - connection oriented (or stream sockets, i.e. TCP);
 * - connectionless (or datagram sockets, i.e. UDP).
 * Connection oriented sockets work with the data in a stream, connectionless sockets
 * work with the data in independent packets (datagrams). Former guarantees delivery,
 * while latter doesn't.
 * #PSocket supports INET and INET6 address families which specifies network communication
 * addresses used by created sockets: IPv4 and IPv6 correspondingly. INET6 family
 * is not supported on all platforms, read documentation for target platform.
 * #PSocket supports different underlying data transfer protocols: TCP, UDP and
 * others. Note that not all protocols can be used with any socket type, i.e.
 * you can use TCP protocol with stream socket, but you can't use UDP protocol with
 * stream socket. You can specify #P_SOCKET_PROTOCOL_DEFAULT protocol when creating socket
 * and appropriate best matching socket type will be selected.
 *
 * In common socket communication scheme server and client sides are involved.
 * Depending on whether sockets are connection oriented, there are slightly different
 * usage patterns.
 * For connection oriented sockets server side acts as following:
 * - creates socket using p_socket_new();
 * - binds socket to particular local address using p_socket_bind();
 * - starts to listen incoming connections using p_socket_listen();
 * - takes incoming connection from the internal queue using p_socket_accept().
 *
 * Client side acts as following:
 * - creates socket using p_socket_new();
 * - binds socket to particular local address using p_socket_bind();
 * - connects to server using p_socket_connect();
 * After connection was successfully established, both sides can send and receive data
 * to each other using p_socket_send() and p_socket_receive().
 * When using connectionless sockets, all is a bit simpler. There is no server side
 * or client side - anyone can send and receive data without establishing connection.
 * Just create a socket, bind it to local address and send/receive data using
 * p_socket_send_to() and p_socket_receive(). You can also call p_socket_connect() on
 * connectionless socket to prevent passing target address each time when sending data
 * and use p_socket_send() instead of p_socket_send_to().
 *
 * #PSocket can operate in blocking and non-blocking (async) modes. By default it is in
 * blocking mode. When using #PSocket in blocking mode each non-immediate call on it
 * will block caller thread until operation will be completed. For example, p_socket_accept()
 * call can wait for incoming connection for some time, and calling it on blocking socket
 * will prevent caller thread from further execution until it receives new incoming connection.
 * In non-blocking mode any call will return immediatly and you must check it's result.
 * You can change socket mode using p_socket_set_blocking().
 *
 * Note that before using #PSocket API you must call p_lib_init() in order to initialize system
 * resources (on UNIX this will do nothing, but on Windows this routine is required),
 * and p_lib_shutdown() after #PSocket API usage is over. Commonly first routine called on
 * the program's start, and the second one before application termination.
 *
 * Here is an example of #PSocket usage:
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
 * Here UDP socket was created, binded to localhost address and port 5432. Do not forget to close
 * socket and free memory after its usage.
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

/** Socket IO waiting (polling) conditions */
typedef enum _PSocketIOCondition {
	P_SOCKET_IO_CONDITION_POLLIN		= 1,	/**< Ready to read.	*/
	P_SOCKET_IO_CONDITION_POLLOUT		= 2	/**< Ready to write.	*/
} PSocketIOCondition;

/** Typedef for opaque #PSocket structure */
typedef struct _PSocket PSocket;

/**
 * @brief Creates new #PSocket from file descriptor.
 * @param fd File descriptor to create socket from.
 * @return Pointer to #PSocket in case of success, NULL otherwise.
 * @since 0.0.1
 * @sa p_socket_get_fd()
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
 * will fail. Use #P_SOCKET_PROTOCOL_DEFAULT for automatical matching depending on
 * specified @a type.
 * @sa #PSocketFamily, #PSocketType, #PSocketProtocol
 */
P_LIB_API PSocket *		p_socket_new 			(PSocketFamily		family,
								 PSocketType		type,
								 PSocketProtocol	protocol);

/**
 * @brief Gets underlying file descriptor of the socket.
 * @param socket #PSocket to get file descriptor for.
 * @return File descriptor in case of success, -1 otherwise.
 * @since 0.0.1
 * @sa p_socket_new_from_fd()
 */
P_LIB_API pint			p_socket_get_fd 		(const PSocket		*socket);

/**
 * @brief Gets socket's family.
 * @param socket #PSocket to get family for.
 * @return #PSocketFamily in case of success, #P_SOCKET_FAMILY_UNKNOWN otherwise.
 * @since 0.0.1
 */
P_LIB_API PSocketFamily		p_socket_get_family 		(const PSocket		*socket);

/**
 * @brief Gets socket's type.
 * @param socket #PSocket to get type for.
 * @return #PSocketType in case of success, #P_SOCKET_TYPE_UNKNOWN otherwise.
 * @since 0.0.1
 */
P_LIB_API PSocketType		p_socket_get_type 		(const PSocket		*socket);

/**
 * @brief Gets socket's data transfer protocol.
 * @param socket #PSocket to get data transfer protocol for.
 * @return #PSocketProtocol in case of success, #P_SOCKET_PROTOCOL_UNKNOWN otherwise.
 * @since 0.0.1
 */
P_LIB_API PSocketProtocol	p_socket_get_protocol		(const PSocket		*socket);

/**
 * @brief Checks whether keep alive flag is enabled.
 * @param socket #PSocket to check keep alive flag for.
 * @return TRUE if keep alive flag is enabled, FALSE otherwise.
 * @since 0.0.1
 * @sa p_socket_set_keepalive(), p_socket_check_connect_result()
 *
 * This option is only has effect for connection oriented (i.e. TCP) sockets.
 * After connection has been established between sockets, they periodically
 * send ping packets to each other to make sure that connection is alive. The time
 * interval between alive packets is system dependent and varies from several
 * minutes to several hours.
 *
 * The main usage of this option is to detect dead clients on the server side
 * and close such a broken sockets to free resources for actual clients which may
 * want to connect to server. Some servers may perform long time waiting for the data
 * from the clients, so such an option helps to detect died clients faster.
 * It's some kind of garbage collecting.
 *
 * Use p_socket_check_connect_result() to check for last connection errors.
 */
P_LIB_API pboolean		p_socket_get_keepalive		(const PSocket		*socket);

/**
 * @brief Checks whether socket is used in blocking mode.
 * @param socket #PSocket to check blocking mode for.
 * @return TRUE if @a socket is in blocking mode, FALSE otherwise.
 * @note Blocking socket will wait for I/O operation to be completed before
 * returning to caller function.
 * @since 0.0.1
 * @sa p_socket_set_blocking()
 */
P_LIB_API pboolean		p_socket_get_blocking		(PSocket 		*socket);

/**
 * @brief Gets socket's listen backlog parameter.
 * @param socket #PSocket to get listen backlog parameter for.
 * @return Listen backlog parameter in case of success, -1 otherwise.
 * @since 0.0.1
 * @sa p_socket_set_listen_backlog(), p_socket_listen()
 *
 * This parameter only has meaning for connection oriented (i.e. TCP) sockets.
 * Backlog parameter specifies how much pending connections from other clients
 * can be stored in internal (system) queue. If socket has already number of pending
 * connections equal to backlog parameter, and another client attempts to connect,
 * it will either get #P_SOCKET_ERROR_CONNECTION_REFUSED error, or it will be
 * connected while some of the pending connections will be dropped. The behavior is
 * system dependent.
 *
 * This parameter is system dependent, some systems may not allow to set it to
 * some values. By default #PSocket attempts to set it to 5.
 */
P_LIB_API pint			p_socket_get_listen_backlog	(const PSocket 		*socket);

/**
 * @brief Gets @a socket timeout for blocking IO operations.
 * @param socket #PSocket to get timeout for.
 * @return Timeout for blocking IO operations in milliseconds, -1 in case of fail.
 * @since 0.0.1
 * @sa p_socket_set_timeout(), p_socket_io_condition_wait()
 *
 * For blocking socket timeout value means maximum amount of time for which blocking
 * call will be waiting until newtwork IO operation completed. If operation is not finished
 * after timeout, blocking call returns with error set to #P_SOCKET_ERROR_TIMED_OUT.
 *
 * For non-blocking socket timeout affects only on p_socket_io_condition_wait() maximum
 * waiting time.
 *
 * Zero timeout means that operation which requires time to complete network IO will be blocked
 * until operation finished of error occured.
 */
P_LIB_API pint			p_socket_get_timeout		(const PSocket		*socket);

/**
 * @brief Gets socket's local (binded) address.
 * @param socket #PSocket to get local address for.
 * @return #PSocketAddress with socket's local address in case of success,
 * NULL otherwise.
 * @since 0.0.1
 * @sa p_socket_bind()
 */
P_LIB_API PSocketAddress *	p_socket_get_local_address	(PSocket 		*socket);

/**
 * @brief Gets socket's remote (connected) address.
 * @param socket #PSocket to get remote address for.
 * @return #PSocketAddress with socket's remote address in case of success,
 * NULL otherwise.
 * @since 0.0.1
 * @sa p_socket_connect()
 */
P_LIB_API PSocketAddress *	p_socket_get_remote_address	(PSocket 		*socket);

/**
 * @brief Checks whether socket is connected.
 * @param socket #PSocket to check connection for.
 * @return TRUE if @a socket is connected, FALSE otherwise.
 * @since 0.0.1
 * @sa p_socket_connect(), p_socket_check_connect_result()
 *
 * This function doesn't check if socket is still connected, it only checks whether
 * a p_socket_connect() call was successfully performed on the @a socket.
 */
P_LIB_API pboolean		p_socket_is_connected		(const PSocket		*socket);

/**
 * @brief Checks connection state after calling p_socket_connect().
 * @param socket #PSocket to check connection state for.
 * @return TRUE if was no error, FALSE otherwise.
 * @since 0.0.1
 * @sa p_socket_io_condition_wait()
 *
 * Usually this call is used after calling p_socket_connect() on the socket in
 * non-blocking mode to check connection state. If call returns FALSE result then
 * connection checking call has failed or there was an error during connection and
 * you should check last error using p_socket_get_last_error().
 *
 * If socket is still pending for connection you will get #P_SOCKET_ERROR_CONNECTING.
 */
P_LIB_API pboolean		p_socket_check_connect_result	(PSocket		*socket);

/**
 * @brief Sets socket's keep alive flag.
 * @param socket #PSocket to set keep alive flag for.
 * @param keepalive Value for keep alive flag.
 * @since 0.0.1
 * @sa p_socket_get_keepalive()
 *
 * See p_socket_get_keepalive() documentation for description of this option.
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
 * @note This parameter can take effect only if it was set before calling
 * p_socket_listen(). Otherwise it will be ignored by underlying socket
 * system calls.
 * @since 0.0.1
 * @sa p_socket_get_listen_backlog()
 *
 * See p_socket_get_listen_backlog() documentation for description of this option.
 */
P_LIB_API void			p_socket_set_listen_backlog	(PSocket		*socket,
								 pint			backlog);

/**
 * @brief Sets @a socket timeout value for blocking IO operations.
 * @param socket #PSocket to set @a timeout for.
 * @param timeout Timeout value in milliseconds.
 * @since 0.0.1
 * @sa p_socket_get_timeoout(), p_socket_io_condition_wait()
 *
 * See p_socket_get_timeout() documentation for description of this option.
 */
P_LIB_API void			p_socket_set_timeout		(PSocket		*socket,
								 pint			timeout);

/**
 * @brief Binds socket to given local address.
 * @param socket #PSocket to bind.
 * @param address #PSocketAddress to bind given @a socket to.
 * @param allow_reuse Whether to allow socket's address reusing.
 * @return TRUE in case of success, FALSE otherwise.
 * @since 0.0.1
 * @sa p_socket_get_local_address()
 *
 * @a allow_reuse option allows to resolve address conflicts for several binded
 * sockets. In common case two or more sockets can't be binded to the same address (network
 * address and port) for the same data transfer protocol (i.e. TCP or UDP) because
 * they will be in conflicted state for data receiving. But socket can be also binded
 * for an any network interface (i.e. 0.0.0.0 network address) and particular port.
 * If you will try to bind another socket without @a allow_reuse option to particular
 * network address (i.e. 127.0.0.1) and the same port, p_socket_bind() call will fail.
 *
 * With @a allow_reuse option system will resolve this conflict: socket will be binded
 * to particular address and port (and will receive data targeted to this particular address)
 * while the first socket will be receiving all other data matching binded address.
 * This option is system dependent, some systems do not support it. Also some systems
 * have option to reuse address port in the same way, too.
 *
 * Connection oriented sockets have another problem - a so called linger time. It is
 * a time required for system to properly close socket connection (and this process can
 * be quite complicated). This time can be measured from several minutes to several hours (!).
 * Socket in such state is half-dead, but it holds binded address and attempt to bind
 * another socket on this address will fail. @a allow_reuse option allows to bind another
 * socket on such half-dead address, but behavior can be unexpected, it's better to refer to
 * system documentation for that.
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
 * @sa p_socket_is_connected(), p_socket_check_connect_result(), p_socket_get_remote_address()
 *
 * Calling this method on connectionless (i.e. UDP) socket will bind it to remote address
 * and p_socket_send() method can be used instead of p_socket_send_to(), so you do not
 * need to specify remote (target) address each time you need to send data. Repeat call
 * will re-bind it to another remote address.
 *
 * Calling this method twice on connection oriented socket (i.e. TCP) will have no effect
 * and will fail.
 */
P_LIB_API pboolean		p_socket_connect		(PSocket		*socket,
								 PSocketAddress		*address);

/**
 * @brief Puts socket in listen state.
 * @param socket #PSocket to start listening.
 * @return TRUE in case of success, FALSE otherwise.
 * @since 0.0.1
 * @sa p_socket_get_listen_backlog(), p_socket_set_listen_backlog()
 *
 * This call has meaning only for connection oriented (i.e. TCP) sockets.
 * Before starting to accept incoming connections, socket must be put into passive
 * mode using p_socket_listen(). After that p_socket_accept() can be used to accept
 * incoming connections.
 *
 * Maximum number of pending connections defines by backlog parameter, see p_socket_get_listen_backlog()
 * documentation for more information. Backlog parameter must be set before calling
 * p_socket_listen() to take effect.
 */
P_LIB_API pboolean		p_socket_listen			(PSocket 		*socket);

/**
 * @brief Accepts socket's connection.
 * @param socket #PSocket to accept connection from.
 * @return #PSocket with accepted connection in case of success, NULL otherwise.
 * @since 0.0.1
 *
 * This call has meaning only for connection oriented (i.e. TCP) sockets.
 * Socket can accept new incoming connections only after calling p_socket_listen().
 */
P_LIB_API PSocket *		p_socket_accept			(PSocket		*socket);

/**
 * @brief Receives data from given socket.
 * @param socket #PSocket to receive data from.
 * @param buffer Buffer to write received data in.
 * @param buflen Length of @a buffer.
 * @return Size of written data in case of success, -1 otherwise.
 * @note If @a socket is in blocking mode, then the caller will be blocked until data arrived.
 * @since 0.0.1
 * @sa p_socket_receive_from()
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
 * @note If @a socket is in blocking mode, then the caller will be blocked until data arrived.
 * @since 0.0.1
 * @sa p_socket_receive()
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
 * @return Size of sent data in case of success, -1 otherwise.
 * @note If @a socket is in blocking mode, then the caller will be blocked until data sent.
 * @since 0.0.1
 * @sa p_socket_send_to()
 *
 * Do not use this call for connectionless (i.e. UDP) sockets which are not binded
 * to remote address using p_socket_connect(), because it will always fail - use
 * p_socket_send_to() instead.
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
 * @return Size of sent data in case of success, -1 otherwise.
 * @note If @a socket is in blocking mode, then the caller will be blocked until data sent.
 * @since 0.0.1
 * @sa p_socket_send()
 *
 * This call is used when dealing with connectionless (i.e. UDP) sockets. You
 * can bind such a socket to remote address using p_socket_connect() and use p_socket_send()
 * instead. If you are working with TCP sockets use p_socket_send() after establishing connection.
 */
P_LIB_API pssize		p_socket_send_to		(PSocket		*socket,
								 PSocketAddress		*address,
								 const pchar		*buffer,
								 psize			buflen);

/**
 * @brief Closes socket.
 * @param socket #PSocket to close.
 * @return TRUE in case of success, FALSE otherwise.
 * @since 0.0.1
 * @sa p_socket_free()
 *
 * For connection oriented sockets some time required to completely close
 * socket connection. See documentation for p_socket_bind() for more information.
 */
P_LIB_API pboolean		p_socket_close			(PSocket		*socket);

/**
 * @brief Shutdowns socket's data transfer directions.
 * @param socket #PSocket to shutdown.
 * @param shutdown_read Whether to shutdown read data transfer direction.
 * @param shutdown_write Whether to shutdown write data transfer direction.
 * @return TRUE in case of success, FALSE otherwise.
 * @note Shutdown of any direction is possible only on socket in connected
 * state. Otherwise call will fail.
 * @since 0.0.1
 *
 * After shutdown data tranfer couldn't be restored in any direction.
 */
P_LIB_API pboolean		p_socket_shutdown		(PSocket		*socket,
								 pboolean		shutdown_read,
								 pboolean		shutdown_write);

/**
 * @brief Closes socket and frees its resources.
 * @param socket #PSocket to free resources from.
 * @since 0.0.1
 * @sa p_socket_close()
 */
P_LIB_API void			p_socket_free			(PSocket 		*socket);

/**
 * @brief Gets last socket error.
 * @param socket #PSocket to get error for.
 * @return Last error occurred.
 * @since 0.0.1
 * @sa p_socket_clear_last_error()
 */
P_LIB_API PSocketError		p_socket_get_last_error		(const PSocket		*socket);

/**
 * @brief Clears @a socket error state to #P_SOCKET_ERROR_NONE.
 * @param socket #PSocket to clear error for.
 * @since 0.0.1
 * @sa p_socket_get_last_error()
 */
P_LIB_API void			p_socket_clear_last_error	(PSocket		*socket);

/**
 * @brief Sets socket's buffer size for given data transfer direction.
 * @param socket #PSocket to set buffer size for.
 * @param dir Direction to set buffer size on.
 * @param size Size of buffer to set.
 * @return TRUE in case of success, FALSE otherwise.
 * @since 0.0.1
 */
P_LIB_API pboolean		p_socket_set_buffer_size	(PSocket		*socket,
								 PSocketDirection	dir,
								 psize			size);

/**
 * @brief Wait for specified IO @a condition on @a socket.
 * @param socket #PSocket to wait for a @a condition on.
 * @param condition An IO condition to wait for on @a socket.
 * @return TRUE if @a condition has been met, FALSE otherwise.
 * @since 0.0.1
 * @sa p_socket_get_timeout(), p_socket_set_timeout()
 *
 * Waits until a @a condition will be met on @a socket or an error occured. If timeout
 * was set using p_socket_set_timeout() and network IO operation doesn't finish until
 * timeout expired, call will fail with #P_SOCKET_ERROR_TIMED_OUT.
 */
P_LIB_API pboolean		p_socket_io_condition_wait	(PSocket		*socket,
								 PSocketIOCondition	condition);

P_END_DECLS

#endif /* __PSOCKET_H__ */
