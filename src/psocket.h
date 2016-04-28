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
 * - connection-less (or datagram sockets, i.e. UDP).
 *
 * Connection oriented sockets work with the data in a stream, connection-less sockets
 * work with the data in independent packets (datagrams). Former guarantees delivery,
 * while latter doesn't.
 *
 * #PSocket supports INET and INET6 address families which specify network communication
 * addresses used by created sockets: IPv4 and IPv6 correspondingly. INET6 family
 * is not supported on all platforms, refer to documentation for target platform.
 * #PSocket supports different underlying data transfer protocols: TCP, UDP and
 * other. Note that not all protocols can be used with any socket type, i.e.
 * you can use TCP protocol with stream socket, but you can't use UDP protocol with
 * stream socket. You can specify #P_SOCKET_PROTOCOL_DEFAULT protocol when creating socket
 * and appropriate best matching socket type will be selected.
 *
 * In common socket communication case server and client sides are involved.
 * Depending on whether sockets are connection oriented, there are slightly different
 * usage patterns.
 *
 * For connection oriented sockets server side acts as following:
 * - creates socket using p_socket_new();
 * - binds socket to particular local address using p_socket_bind();
 * - starts to listen incoming connections using p_socket_listen();
 * - takes incoming connection from the internal queue using p_socket_accept().
 *
 * Client side acts as following:
 * - creates socket using p_socket_new();
 * - binds socket to particular local address using p_socket_bind();
 * - connects to server using p_socket_connect().
 *
 * After connection was successfully established, both sides can send and receive data
 * from each other using p_socket_send() and p_socket_receive().
 *
 * When using connection-less sockets, all is a bit simpler. There is no server side
 * or client side - anyone can send and receive data without establishing connection.
 * Just create a socket, bind it to local address and send/receive data using
 * p_socket_send_to() and p_socket_receive(). You can also call p_socket_connect() on
 * connection-less socket to prevent passing target address each time when sending data
 * and use p_socket_send() instead of p_socket_send_to().
 *
 * #PSocket can operate in blocking and non-blocking (async) modes. By default it is in
 * blocking mode. When using #PSocket in blocking mode each non-immediate call on it
 * will block caller thread until I/O operation will be completed. For example, p_socket_accept()
 * call can wait for incoming connection for some time, and calling it on blocking socket
 * will prevent caller thread from further execution until it receives new incoming connection.
 * In non-blocking mode any call will return immediatly and you must check it's result.
 * You can change socket mode using p_socket_set_blocking().
 *
 * #PSocket always puts socket descriptor (or SOCKET handle on Windows) into non-blocking
 * mode and emulates blocking mode if need. If you need to perform some hacks and need
 * blocking behavior from descriptor for some reason, use p_socket_get_fd() to get internal
 * socket descriptor (SOCKET handle on Windows).
 *
 * Close-on-exec flag is always set on socket desciptor. Use p_socket_get_fd() to overwrite
 * this behaviour.
 *
 * #PSocket ignores SIGPIPE signal if can. Take it into account if you want to handle this signal.
 *
 * Note that before using #PSocket API you must call p_libsys_init() in order to initialize system
 * resources (on UNIX this will do nothing, but on Windows this routine is required),
 * and p_libsys_shutdown() after #PSocket API usage is over. Commonly first routine called on
 * the program's start, and the second one before application termination.
 *
 * Here is an example of #PSocket usage:
 * @code
 * PSocketAddress *addr;
 * PSocket	  *sock;
 *
 * p_libsys_init ();
 * ...
 * if ((addr = p_socket_address_new ("127.0.0.1", 5432)) == NULL) {
 *	...
 * }
 *
 * if ((sock = p_socket_new (P_SOCKET_FAMILY_INET, P_SOCKET_TYPE_DATAGRAM, P_SOCKET_PROTOCOL_UDP)) == NULL) {
 *	p_socket_address_free (addr);
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
 * p_socket_address_free (addr);
 * p_socket_close (sock);
 * p_socket_free (sock);
 * p_libsys_shutdown ();
 * @endcode
 * Here UDP socket was created, binded to localhost address and port 5432. Do not forget to close
 * socket and free memory after its usage.
 */

#if !defined (__PLIBSYS_H_INSIDE__) && !defined (PLIBSYS_COMPILATION)
#  error "Header files shouldn't be included directly, consider using <plibsys.h> instead."
#endif

#ifndef __PSOCKET_H__
#define __PSOCKET_H__

#include <pmacros.h>
#include <psocketaddress.h>
#include <perror.h>

P_BEGIN_DECLS

/** Socket protocols specified by IANA  */
typedef enum _PSocketProtocol {
	P_SOCKET_PROTOCOL_UNKNOWN		= -1,	/**< Unknown protocol.	*/
	P_SOCKET_PROTOCOL_DEFAULT		= 0,	/**< Default protocol.	*/
	P_SOCKET_PROTOCOL_TCP			= 6,	/**< TCP protocol.	*/
	P_SOCKET_PROTOCOL_UDP			= 17,	/**< UDP protocol.	*/
	P_SOCKET_PROTOCOL_SCTP			= 132	/**< SCTP protocol.	*/
} PSocketProtocol;

/** Socket types */
typedef enum _PSocketType {
	P_SOCKET_TYPE_UNKNOWN			= 0,	/**< Unknown type.							*/
	P_SOCKET_TYPE_STREAM			= 1,	/**< Connection oritented, reliable, stream of bytes (i.e. TCP).	*/
	P_SOCKET_TYPE_DATAGRAM			= 2,	/**< Connection-less, unreliable, datagram passing (i.e. UDP).		*/
	P_SOCKET_TYPE_SEQPACKET			= 3	/**< Connection-less, reliable, datagram passing (i.e. SCTP).		*/
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
 * @param[out] error Error report object, NULL to ignore.
 * @return Pointer to #PSocket in case of success, NULL otherwise.
 * @since 0.0.1
 * @sa p_socket_new(), p_socket_get_fd()
 *
 * Given file descriptor @a fd will be put in non-blocking mode. #PSocket will
 * emulate blocking mode if need.
 *
 * If socket was not binded yet then on some systems (i.e. Windows) call may
 * fail to get socket family from descriptor thus failing to construct #PSocket
 * object.
 */
P_LIB_API PSocket *		p_socket_new_from_fd		(pint 			fd,
								 PError			**error);

/**
 * @brief Creates new #PSocket object.
 * @param family Socket family.
 * @param type Socket type.
 * @param protocol Socket data transfer protocol.
 * @param[out] error Error report object, NULL to ignore.
 * @return Pointer to #PSocket in case of success, NULL otherwise.
 * @since 0.0.1
 * @note If all given parameters are not compatible with each other, then function
 * will fail. Use #P_SOCKET_PROTOCOL_DEFAULT to automatically match best protocol for @a type.
 * @sa #PSocketFamily, #PSocketType, #PSocketProtocol, p_socket_new_from_fd()
 *
 * @a protocol is passing directly to operating system socket() call, #PSocketProtocol has
 * the same values as system definitions. You can pass any existing protocol value to this call if you
 * know it exactly.
 */
P_LIB_API PSocket *		p_socket_new 			(PSocketFamily		family,
								 PSocketType		type,
								 PSocketProtocol	protocol,
								 PError			**error);

/**
 * @brief Gets underlying file descriptor of the @a socket.
 * @param socket #PSocket to get file descriptor for.
 * @return File descriptor in case of success, -1 otherwise.
 * @since 0.0.1
 * @sa p_socket_new_from_fd()
 */
P_LIB_API pint			p_socket_get_fd 		(const PSocket		*socket);

/**
 * @brief Gets @a socket address family.
 * @param socket #PSocket to get address family for.
 * @return #PSocketFamily in case of success, #P_SOCKET_FAMILY_UNKNOWN otherwise.
 * @since 0.0.1
 * @sa #PSocketFamily, p_socket_new()
 *
 * Socket address family specifies address space which will be used to communicate
 * with other sockets. For now, INET and INET6 families are supported. INET6 family
 * available only if operating system supports it.
 */
P_LIB_API PSocketFamily		p_socket_get_family 		(const PSocket		*socket);

/**
 * @brief Gets @a socket type.
 * @param socket #PSocket to get type for.
 * @return #PSocketType in case of success, #P_SOCKET_TYPE_UNKNOWN otherwise.
 * @since 0.0.1
 * @sa #PSocketType, p_socket_new()
 */
P_LIB_API PSocketType		p_socket_get_type 		(const PSocket		*socket);

/**
 * @brief Gets @a socket data transfer protocol.
 * @param socket #PSocket to get data transfer protocol for.
 * @return #PSocketProtocol in case of success, #P_SOCKET_PROTOCOL_UNKNOWN otherwise.
 * @since 0.0.1
 * @sa #PSocketProtocol, p_socket_new()
 */
P_LIB_API PSocketProtocol	p_socket_get_protocol		(const PSocket		*socket);

/**
 * @brief Checks whether SO_KEEPALIVE flag is enabled.
 * @param socket #PSocket to check SO_KEEPALIVE flag for.
 * @return TRUE if SO_KEEPALIVE flag is enabled, FALSE otherwise.
 * @since 0.0.1
 * @sa p_socket_set_keepalive()
 *
 * This option is only has effect for connection oriented sockets.
 * After connection has been established between sockets, they periodically
 * send ping packets to each other to make sure that connection is alive. The time
 * interval between alive packets is system dependent and varies from several
 * minutes to several hours.
 *
 * The main usage of this option is to detect dead clients on the server side
 * and close such a broken sockets to free resources for actual clients which may
 * want to connect to server. Some servers may let clients to be idle for a long time,
 * so such an option helps to detect died clients faster without sending them real
 * data. It's some kind of garbage collecting.
 */
P_LIB_API pboolean		p_socket_get_keepalive		(const PSocket		*socket);

/**
 * @brief Checks whether @a socket is used in blocking mode.
 * @param socket #PSocket to check blocking mode for.
 * @return TRUE if @a socket is in blocking mode, FALSE otherwise.
 * @note Blocking socket will wait for I/O operation to be completed before
 * returning to caller function.
 * @since 0.0.1
 * @sa p_socket_set_blocking()
 *
 * Underlying socket descriptor is always set to non-blocking mode by default and
 * #PSocket emulates blocking mode by itself.
 */
P_LIB_API pboolean		p_socket_get_blocking		(PSocket 		*socket);

/**
 * @brief Gets @a socket listen backlog parameter.
 * @param socket #PSocket to get listen backlog parameter for.
 * @return Listen backlog parameter in case of success, -1 otherwise.
 * @since 0.0.1
 * @sa p_socket_set_listen_backlog(), p_socket_listen()
 *
 * This parameter only has meaning for connection oriented sockets.
 * Backlog parameter specifies how much pending connections from other clients
 * can be stored in internal (system) queue. If socket has already number of pending
 * connections equal to backlog parameter, and another client attempts to connect on that time,
 * it (client) will either be refused. This behavior is system dependent.
 *
 * This parameter is system dependent, some systems may not allow to set it to
 * high values. By default #PSocket attempts to set it to 5.
 */
P_LIB_API pint			p_socket_get_listen_backlog	(const PSocket 		*socket);

/**
 * @brief Gets @a socket timeout for blocking I/O operations.
 * @param socket #PSocket to get timeout for.
 * @return Timeout for blocking I/O operations in milliseconds, -1 in case of fail.
 * @since 0.0.1
 * @sa p_socket_set_timeout(), p_socket_io_condition_wait()
 *
 * For blocking socket timeout value means maximum amount of time for which blocking
 * call will be waiting until newtwork I/O operation completed. If operation is not finished
 * after timeout, blocking call returns with error set to #P_ERROR_IO_TIMED_OUT.
 *
 * For non-blocking socket timeout affects only on p_socket_io_condition_wait() maximum
 * waiting time.
 *
 * Zero timeout means that operation which requires time to complete network I/O will be blocked
 * until operation finished of error occured.
 */
P_LIB_API pint			p_socket_get_timeout		(const PSocket		*socket);

/**
 * @brief Gets @a socket local (bound) address.
 * @param socket #PSocket to get local address for.
 * @param[out] error Error report object, NULL to ignore.
 * @return #PSocketAddress with socket local address in case of success,
 * NULL otherwise.
 * @since 0.0.1
 * @sa p_socket_bind()
 *
 * If @a socket was not bound explicitly with p_socket_bind() or implicitly with
 * p_socket_connect(), the call will fail.
 */
P_LIB_API PSocketAddress *	p_socket_get_local_address	(const PSocket 		*socket,
								 PError			**error);

/**
 * @brief Gets @a socket remote endpoint address.
 * @param socket #PSocket to get remote endpoint address for.
 * @param[out] error Error report object, NULL to ignore.
 * @return #PSocketAddress with socket endpoint remote address in case of success,
 * NULL otherwise.
 * @since 0.0.1
 * @sa p_socket_connect()
 *
 * If @a socket was not connected to endpoint address with p_socket_connect() this call will fail.
 */
P_LIB_API PSocketAddress *	p_socket_get_remote_address	(const PSocket 		*socket,
								 PError			**error);

/**
 * @brief Checks whether @a socket is connected.
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
 * @param[out] error Error report object, NULL to ignore.
 * @return TRUE if was no error, FALSE otherwise.
 * @since 0.0.1
 * @sa p_socket_io_condition_wait()
 *
 * Usually this call is used after calling p_socket_connect() on the socket in
 * non-blocking mode to check connection state. If call returns FALSE result then
 * connection checking call has failed or there was an error during connection and
 * you should check last error using @a error object.
 *
 * If socket is still pending for connection you will get #P_ERROR_IO_IN_PROGRESS.
 *
 * After calling p_socket_connect() on non-blocking socket, you can wait for connection
 * operation to be finished using p_socket_io_condition_wait() with #P_SOCKET_IO_CONDITION_POLLOUT
 * option.
 */
P_LIB_API pboolean		p_socket_check_connect_result	(PSocket		*socket,
								 PError			**error);

/**
 * @brief Sets @a socket SO_KEEPALIVE flag.
 * @param socket #PSocket to set SO_KEEPALIVE flag for.
 * @param keepalive Value for SO_KEEPALIVE flag.
 * @since 0.0.1
 * @sa p_socket_get_keepalive()
 *
 * See p_socket_get_keepalive() documentation for description of this option.
 */
P_LIB_API void			p_socket_set_keepalive		(PSocket 		*socket,
								 pboolean		keepalive);

/**
 * @brief Sets @a socket blocking mode.
 * @param socket #PSocket to set blocking mode for.
 * @param blocking Whether to set @a socket to blocking mode.
 * @note Blocking socket will wait for I/O operation to be completed before
 * returning to caller function.
 * @note On some OS blocking timeout may be less than threads scheduling
 * granularity, so actual timeout can be greater than specified one.
 * @since 0.0.1
 * @sa p_socket_get_blocking()
 */
P_LIB_API void			p_socket_set_blocking		(PSocket 		*socket,
								 pboolean		blocking);

/**
 * @brief Sets @a socket listen backlog parameter.
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
 * @brief Sets @a socket timeout value for blocking I/O operations.
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
 * @brief Binds @a socket to given local address.
 * @param socket #PSocket to bind.
 * @param address #PSocketAddress to bind given @a socket to.
 * @param allow_reuse Whether to allow socket's address reusing.
 * @param[out] error Error report object, NULL to ignore.
 * @return TRUE in case of success, FALSE otherwise.
 * @since 0.0.1
 * @sa p_socket_get_local_address()
 *
 * @a allow_reuse option allows to resolve address conflicts for several binded
 * sockets. It controls SO_REUSEADDR socket flag.
 * In common case two or more sockets can't be bound to the same address (network
 * address and port) for the same data transfer protocol (i.e. TCP or UDP) because
 * they will be in conflicted state for data receiving. But socket can be also bound
 * for an any network interface (i.e. 0.0.0.0 network address) and particular port.
 * If you will try to bind another socket without @a allow_reuse option to particular
 * network address (i.e. 127.0.0.1) and the same port, p_socket_bind() call will fail.
 *
 * With @a allow_reuse option system will resolve this conflict: socket will be bound
 * to particular address and port (and will receive data targeted to this particular address)
 * while the first socket will be receiving all other data matching bound address.
 * This option is system dependent, some systems do not support it. Also some systems
 * have option to reuse address port (SO_REUSEPORT) in the same way, too.
 *
 * Connection oriented sockets have another problem - a so called linger time. It is
 * a time required by system to properly close socket connection (and this process can
 * be quite complicated). This time can be measured from several minutes to several hours (!).
 * Socket in such state is half-dead, but it holds bound address and attempt to bind
 * another socket on this address will fail. @a allow_reuse option allows to bind another
 * socket on such half-dead address, but behavior can be unexpected, it's better to refer to
 * system documentation for that.
 *
 * In general case, server socket should be bound with @a allow_reuse set to TRUE, while
 * client socket shouldn't set this option to TRUE. If you restart client quickly with the
 * same address it can fail to bind.
 */
P_LIB_API pboolean		p_socket_bind			(const PSocket 		*socket,
								 PSocketAddress		*address,
								 pboolean		allow_reuse,
								 PError			**error);

/**
 * @brief Connects @a socket to given remote address.
 * @param socket #PSocket to connect.
 * @param address #PSocketAddress to connect @a socket to.
 * @param[out] error Error report object, NULL to ignore.
 * @return TRUE in case of success, FALSE otherwise.
 * @since 0.0.1
 * @sa p_socket_is_connected(), p_socket_check_connect_result(), p_socket_get_remote_address()
 * p_socket_io_condition_wait()
 *
 * Calling this method on connection-less socket will bind it to remote address
 * and p_socket_send() method can be used instead of p_socket_send_to(), so you do not
 * need to specify remote (target) address each time you need to send data. Socket will
 * also discards incoming data from another addresses. Repeat call will re-bind it to another
 * remote address.
 *
 * For connection oriented socket it tries to establish connection with listening remote
 * socket. Calling this method twice will have no effect and will fail.
 *
 * If @a socket is in non-blocking mode, then you can wait for connection using
 * p_socket_io_condition_wait() with #P_SOCKET_IO_CONDITION_POLLOUT parameters. You should
 * check connection result after that using p_socket_check_connect_result().
 */
P_LIB_API pboolean		p_socket_connect		(PSocket		*socket,
								 PSocketAddress		*address,
								 PError			**error);

/**
 * @brief Puts @a socket in listen state.
 * @param socket #PSocket to start listening.
 * @param[out] error Error report object, NULL to ignore.
 * @return TRUE in case of success, FALSE otherwise.
 * @since 0.0.1
 * @sa p_socket_get_listen_backlog(), p_socket_set_listen_backlog()
 *
 * This call has meaning only for connection oriented sockets.
 * Before starting to accept incoming connections, socket must be put into passive
 * mode using p_socket_listen(). After that p_socket_accept() can be used to accept
 * incoming connections.
 *
 * Maximum number of pending connections defined by backlog parameter, see p_socket_get_listen_backlog()
 * documentation for more information. Backlog parameter must be set before calling
 * p_socket_listen() to take effect.
 */
P_LIB_API pboolean		p_socket_listen			(PSocket 		*socket,
								 PError			**error);

/**
 * @brief Accepts @a socket incoming connection.
 * @param socket #PSocket to accept incoming connection from.
 * @param[out] error Error report object, NULL to ignore.
 * @return New #PSocket with accepted connection in case of success, NULL otherwise.
 * @since 0.0.1
 *
 * This call has meaning only for connection oriented sockets.
 * Socket can accept new incoming connections only after calling p_socket_bin() and p_socket_listen().
 */
P_LIB_API PSocket *		p_socket_accept			(const PSocket		*socket,
								 PError			**error);

/**
 * @brief Receives data from given @a socket.
 * @param socket #PSocket to receive data from.
 * @param buffer Buffer to write received data in.
 * @param buflen Length of @a buffer.
 * @param[out] error Error report object, NULL to ignore.
 * @return Size of written data in case of success, -1 otherwise.
 * @note If @a socket is in blocking mode, then the caller will be blocked until data arrived.
 * @since 0.0.1
 * @sa p_socket_receive_from(), p_socket_connect()
 *
 * If @a buflen is less than received data size, only @a buflen bytes of data will be
 * written to @a buffer, and excess bytes may be discarded depending on the socket message
 * type.
 *
 * This call is normally used only with connected socket, see p_socket_connect().
 */
P_LIB_API pssize		p_socket_receive		(const PSocket		*socket,
								 pchar			*buffer,
								 psize			buflen,
								 PError			**error);

/**
 * @brief Receives data from given @a socket and saves remote address.
 * @param socket #PSocket to receive data from.
 * @param[out] address Pointer to store remote address in case of success, caller is responsible
 * to free it after using. May be NULL.
 * @param buffer Buffer to write received data in.
 * @param buflen Length of @a buffer.
 * @param[out] error Error report object, NULL to ignore.
 * @return Size of written data in case of success, -1 otherwise.
 * @note If @a socket is in blocking mode, then the caller will be blocked until data arrived.
 * @since 0.0.1
 * @sa p_socket_receive()
 *
 * If @a buflen is less than received data size, only @a buflen bytes of data will be
 * written to @a buffer, and excess bytes may be discarded depending on the socket message
 * type.
 *
 * This call is normally used only with connection-less socket.
 */
P_LIB_API pssize		p_socket_receive_from		(const PSocket 		*socket,
								 PSocketAddress		**address,
								 pchar 			*buffer,
								 psize			buflen,
								 PError			**error);

/**
 * @brief Sends data through given @a socket.
 * @param socket #PSocket to send data through.
 * @param buffer Buffer with data to send.
 * @param buflen Length of @a buffer.
 * @param[out] error Error report object, NULL to ignore.
 * @return Size of sent data in case of success, -1 otherwise.
 * @note If @a socket is in blocking mode, then the caller will be blocked until data sent.
 * @since 0.0.1
 * @sa p_socket_send_to()
 *
 * Do not use this call for connection-less sockets which are not connected to remote address
 * using p_socket_connect(), because it will always fail - use p_socket_send_to() instead.
 */
P_LIB_API pssize		p_socket_send			(const PSocket		*socket,
								 const pchar		*buffer,
								 psize			buflen,
								 PError			**error);

/**
 * @brief Sends data through given @a socket to given address.
 * @param socket #PSocket to send data through.
 * @param address #PSocketAddress to send data to.
 * @param buffer Buffer with data to send.
 * @param buflen Length of @a buffer.
 * @param[out] error Error report object, NULL to ignore.
 * @return Size of sent data in case of success, -1 otherwise.
 * @note If @a socket is in blocking mode, then the caller will be blocked until data sent.
 * @since 0.0.1
 * @sa p_socket_send()
 *
 * This call is used when dealing with connection-less sockets. You can bind such a socket to
 * remote address using p_socket_connect() and use p_socket_send() instead. If you are working
 * with connection oriented sockets use p_socket_send() after establishing connection.
 */
P_LIB_API pssize		p_socket_send_to		(const PSocket		*socket,
								 PSocketAddress		*address,
								 const pchar		*buffer,
								 psize			buflen,
								 PError			**error);

/**
 * @brief Closes @a socket.
 * @param socket #PSocket to close.
 * @param[out] error Error report object, NULL to ignore.
 * @return TRUE in case of success, FALSE otherwise.
 * @since 0.0.1
 * @sa p_socket_free()
 *
 * For connection oriented sockets some time required to completely close
 * socket connection. See documentation for p_socket_bind() for more information.
 */
P_LIB_API pboolean		p_socket_close			(PSocket		*socket,
								 PError			**error);

/**
 * @brief Shutdowns a full-duplex @a socket data transfer link.
 * @param socket #PSocket to shutdown link.
 * @param shutdown_read Whether to shutdown incoming data transfer link.
 * @param shutdown_write Whether to shutdown outcoming data transfer link.
 * @param[out] error Error report object, NULL to ignore.
 * @return TRUE in case of success, FALSE otherwise.
 * @note Shutdown of any link is possible only on socket in connected state. Otherwise call
 * will fail.
 * @since 0.0.1
 *
 * After shutdowning, data tranffer link couldn't be restored in any direction. It is often
 * used to gracefully close connection for connection oriented socket.
 */
P_LIB_API pboolean		p_socket_shutdown		(PSocket		*socket,
								 pboolean		shutdown_read,
								 pboolean		shutdown_write,
								 PError			**error);

/**
 * @brief Closes @a socket (if not closed yet) and frees its resources.
 * @param socket #PSocket to free resources from.
 * @since 0.0.1
 * @sa p_socket_close()
 */
P_LIB_API void			p_socket_free			(PSocket 		*socket);

/**
 * @brief Sets @a socket buffer size for given data transfer direction.
 * @param socket #PSocket to set buffer size for.
 * @param dir Direction to set buffer size on.
 * @param size Size of buffer to set.
 * @param[out] error Error report object, NULL to ignore.
 * @return TRUE in case of success, FALSE otherwise.
 * @since 0.0.1
 */
P_LIB_API pboolean		p_socket_set_buffer_size	(const PSocket		*socket,
								 PSocketDirection	dir,
								 psize			size,
								 PError			**error);

/**
 * @brief Wait for specified I/O @a condition on @a socket.
 * @param socket #PSocket to wait for a @a condition on.
 * @param condition An I/O condition to wait for on @a socket.
 * @param[out] error Error report object, NULL to ignore.
 * @return TRUE if @a condition has been met, FALSE otherwise.
 * @since 0.0.1
 * @sa p_socket_get_timeout(), p_socket_set_timeout()
 *
 * Waits until a @a condition will be met on @a socket or an error occured. If timeout
 * was set using p_socket_set_timeout() and network I/O operation doesn't finish until
 * timeout expired, call will fail with #P_ERROR_IO_TIMED_OUT.
 */
P_LIB_API pboolean		p_socket_io_condition_wait	(const PSocket		*socket,
								 PSocketIOCondition	condition,
								 PError			**error);

P_END_DECLS

#endif /* __PSOCKET_H__ */
