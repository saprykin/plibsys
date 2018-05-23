/*
 * The MIT License
 *
 * Copyright (C) 2010-2016 Alexander Saprykin <saprykin.spb@gmail.com>
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

/**
 * @file psocket.h
 * @brief Socket implementation
 * @author Alexander Saprykin
 *
 * A socket is a communication primitive usually working over a network. You can
 * send data to someone's socket by its address and receive data as well through
 * the same socket. This is one of the most popular and standardizated way for
 * network communication supported by vast majority of all the modern operating
 * systems. It also hides all the details of underlying networking protocols and
 * other layers, providing a unified and transparent approach for communication.
 *
 * There are two kinds of socket:
 * - connection oriented (or stream sockets, i.e. TCP);
 * - connection-less (or datagram sockets, i.e. UDP).
 *
 * Connection oriented sockets work with data in a stream, connection-less
 * sockets work with data using independent packets (datagrams). The former
 * guarantees delivery, while the latter doesn't (actually some connection-less
 * protocols provide delivery quarantee, i.e. SCTP).
 *
 * #PSocket supports INET and INET6 address families which specify network
 * communication addresses used by created sockets: IPv4 and IPv6,
 * correspondingly. INET6 family is not supported on all platforms, refer to
 * documentation for a particular target platform.
 *
 * #PSocket supports different underlying data transfer protocols: TCP, UDP and
 * others. Note that not all protocols can be used with any socket type, i.e.
 * you can use the TCP protocol with a stream socket, but you can't use the UDP
 * protocol with the stream socket. You can specify #P_SOCKET_PROTOCOL_DEFAULT
 * protocol when creating a socket and appropriate the best matching socket type
 * will be selected.
 *
 * In a common socket communication case server and client sides are involved.
 * Depending on whether sockets are connection oriented, there are slightly
 * different action sequences for data exchanging.
 *
 * For connection oriented sockets the server side acts as following:
 * - creates a socket using p_socket_new();
 * - binds the socket to a particular local address using p_socket_bind();
 * - starts to listen incoming connections using p_socket_listen();
 * - takes an incoming connection from the internal queue using
 * p_socket_accept().
 *
 * The client side acts as following:
 * - creates a socket using p_socket_new();
 * - binds the socket to a particular local address using p_socket_bind();
 * - connects to the server using p_socket_connect().
 *
 * After the connection was successfully established, both the sides can send
 * and receive data from each other using p_socket_send() and
 * p_socket_receive(). Binding of the client socket is actually optional.
 *
 * When using connection-less sockets, all is a bit simpler. There is no server
 * side or client side - anyone can send and receive data without establishing a
 * connection. Just create a socket, bind it to a local address and send/receive
 * data using p_socket_send_to() and p_socket_receive(). You can also call
 * p_socket_connect() on a connection-less socket to prevent passing the target
 * address each time when sending data and then use p_socket_send() instead of
 * p_socket_send_to(). This time binding is required.
 *
 * #PSocket can operate in blocking and non-blocking (async) modes. By default
 * it is in the blocking mode. When using #PSocket in the blocking mode each
 * non-immediate call on it will block a caller thread until an I/O operation
 * will be completed. For example, the p_socket_accept() call can wait for an
 * incoming connection for some time, and calling it on a blocking socket will
 * prevent the caller thread from further execution until it receives a new
 * incoming connection. In the non-blocking mode any call will return
 * immediately and you must check its result. You can set the socket mode using
 * p_socket_set_blocking().
 *
 * #PSocket always puts a socket descriptor (or SOCKET handle on Windows) into
 * the non-blocking mode and emulates the blocking mode if required. If you need
 * to perform some hacks and need blocking behavior from the descriptor for some
 * reason, use p_socket_get_fd() to get an internal socket descriptor (SOCKET
 * handle on Windows).
 *
 * The close-on-exec flag is always set on the socket desciptor. Use
 * p_socket_get_fd() to overwrite this behavior.
 *
 * #PSocket ignores the SIGPIPE signal on UNIX systems if possible. Take it into
 * account if you want to handle this signal.
 *
 * Note that before using the #PSocket API you must call p_libsys_init() in
 * order to initialize system resources (on UNIX this will do nothing, but on
 * Windows this routine is required). Usually this routine should be called on a
 * program's start.
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
 * if ((sock = p_socket_new (P_SOCKET_FAMILY_INET,
 *			     P_SOCKET_TYPE_DATAGRAM,
 *			     P_SOCKET_PROTOCOL_UDP)) == NULL) {
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
 * Here a UDP socket was created, bound to the localhost address and the port
 * @a 5432. Do not forget to close the socket and free memory after its usage.
 */

#if !defined (PLIBSYS_H_INSIDE) && !defined (PLIBSYS_COMPILATION)
#  error "Header files shouldn't be included directly, consider using <plibsys.h> instead."
#endif

#ifndef PLIBSYS_HEADER_PSOCKET_H
#define PLIBSYS_HEADER_PSOCKET_H

#include <pmacros.h>
#include <psocketaddress.h>
#include <perror.h>

P_BEGIN_DECLS

/** Socket protocols specified by the IANA.  */
typedef enum PSocketProtocol_ {
	P_SOCKET_PROTOCOL_UNKNOWN	= -1,	/**< Unknown protocol.	*/
	P_SOCKET_PROTOCOL_DEFAULT	= 0,	/**< Default protocol.	*/
	P_SOCKET_PROTOCOL_TCP		= 6,	/**< TCP protocol.	*/
	P_SOCKET_PROTOCOL_UDP		= 17,	/**< UDP protocol.	*/
	P_SOCKET_PROTOCOL_SCTP		= 132	/**< SCTP protocol.	*/
} PSocketProtocol;

/** Socket types. */
typedef enum PSocketType_ {
	P_SOCKET_TYPE_UNKNOWN		= 0,	/**< Unknown type.							*/
	P_SOCKET_TYPE_STREAM		= 1,	/**< Connection oritented, reliable, stream of bytes (i.e. TCP).	*/
	P_SOCKET_TYPE_DATAGRAM		= 2,	/**< Connection-less, unreliable, datagram passing (i.e. UDP).		*/
	P_SOCKET_TYPE_SEQPACKET		= 3	/**< Connection-less, reliable, datagram passing (i.e. SCTP).		*/
} PSocketType;

/** Socket direction for data operations. */
typedef enum PSocketDirection_ {
	P_SOCKET_DIRECTION_SND		= 0,	/**< Send direction.	*/
	P_SOCKET_DIRECTION_RCV		= 1	/**< Receive direction.	*/
} PSocketDirection;

/** Socket IO waiting (polling) conditions. */
typedef enum PSocketIOCondition_ {
	P_SOCKET_IO_CONDITION_POLLIN	= 1,	/**< Ready to read.	*/
	P_SOCKET_IO_CONDITION_POLLOUT	= 2	/**< Ready to write.	*/
} PSocketIOCondition;

/** Socket opaque structure. */
typedef struct PSocket_ PSocket;

/**
 * @brief Creates a new #PSocket object from a file descriptor.
 * @param fd File descriptor to create the socket from.
 * @param[out] error Error report object, NULL to ignore.
 * @return Pointer to #PSocket in case of success, NULL otherwise.
 * @since 0.0.1
 * @sa p_socket_new(), p_socket_get_fd()
 *
 * The given file descriptor @a fd will be put in a non-blocking mode. #PSocket
 * will emulate a blocking mode if required.
 *
 * If the socket was not bound yet then on some systems (i.e. Windows) call may
 * fail to get a socket family from the descriptor thus failing to construct the
 * #PSocket object.
 */
P_LIB_API PSocket *		p_socket_new_from_fd		(pint 			fd,
								 PError			**error);

/**
 * @brief Creates a new #PSocket object.
 * @param family Socket family.
 * @param type Socket type.
 * @param protocol Socket data transfer protocol.
 * @param[out] error Error report object, NULL to ignore.
 * @return Pointer to #PSocket in case of success, NULL otherwise.
 * @since 0.0.1
 * @note If all the given parameters are not compatible with each other, then
 * the function will fail. Use #P_SOCKET_PROTOCOL_DEFAULT to automatically
 * match the best protocol for a particular @a type.
 * @sa #PSocketFamily, #PSocketType, #PSocketProtocol, p_socket_new_from_fd()
 *
 * The @a protocol is passed directly to the operating system socket() call,
 * #PSocketProtocol has the same values as the system definitions. You can pass
 * any existing protocol value to this call if you know it exactly.
 */
P_LIB_API PSocket *		p_socket_new 			(PSocketFamily		family,
								 PSocketType		type,
								 PSocketProtocol	protocol,
								 PError			**error);

/**
 * @brief Gets an underlying file descriptor of a @a socket.
 * @param socket #PSocket to get the file descriptor for.
 * @return File descriptor in case of success, -1 otherwise.
 * @since 0.0.1
 * @sa p_socket_new_from_fd()
 */
P_LIB_API pint			p_socket_get_fd 		(const PSocket		*socket);

/**
 * @brief Gets a @a socket address family.
 * @param socket #PSocket to get the address family for.
 * @return #PSocketFamily in case of success, #P_SOCKET_FAMILY_UNKNOWN
 * otherwise.
 * @since 0.0.1
 * @sa #PSocketFamily, p_socket_new()
 *
 * The socket address family specifies address space which will be used to
 * communicate with other sockets. For now, the INET and INET6 families are
 * supported. The INET6 family is available only if the operating system
 * supports it.
 */
P_LIB_API PSocketFamily		p_socket_get_family 		(const PSocket		*socket);

/**
 * @brief Gets a @a socket type.
 * @param socket #PSocket to get the type for.
 * @return #PSocketType in case of success, #P_SOCKET_TYPE_UNKNOWN otherwise.
 * @since 0.0.1
 * @sa #PSocketType, p_socket_new()
 */
P_LIB_API PSocketType		p_socket_get_type 		(const PSocket		*socket);

/**
 * @brief Gets a @a socket data transfer protocol.
 * @param socket #PSocket to get the data transfer protocol for.
 * @return #PSocketProtocol in case of success, #P_SOCKET_PROTOCOL_UNKNOWN
 * otherwise.
 * @since 0.0.1
 * @sa #PSocketProtocol, p_socket_new()
 */
P_LIB_API PSocketProtocol	p_socket_get_protocol		(const PSocket		*socket);

/**
 * @brief Checks whether the SO_KEEPALIVE flag is enabled.
 * @param socket #PSocket to check the SO_KEEPALIVE flag for.
 * @return TRUE if the SO_KEEPALIVE flag is enabled, FALSE otherwise.
 * @since 0.0.1
 * @sa p_socket_set_keepalive()
 *
 * This option only has effect for connection oriented sockets. After a
 * connection has been established between two sockets, they periodically send
 * ping packets to each other to make sure that the connection is alive. A
 * time interval between alive packets is system dependent and varies from
 * several minutes to several hours.
 *
 * The main usage of this option is to detect dead clients on a server side and
 * close such the broken sockets to free resources for the actual clients which
 * may want to connect to the server. Some servers may let clients to be idle
 * for a long time, so such an option helps to detect died clients faster
 * without sending them real data. It's some kind of garbage collecting.
 */
P_LIB_API pboolean		p_socket_get_keepalive		(const PSocket		*socket);

/**
 * @brief Checks whether @a socket is used in a blocking mode.
 * @param socket #PSocket to check the blocking mode for.
 * @return TRUE if @a socket is in the blocking mode, FALSE otherwise.
 * @note A blocking socket will wait for an I/O operation to be completed before
 * returning to the caller function.
 * @since 0.0.1
 * @sa p_socket_set_blocking()
 *
 * The underlying socket descriptor is always set to the non-blocking mode by
 * default and #PSocket emulates the blocking mode if required.
 */
P_LIB_API pboolean		p_socket_get_blocking		(PSocket 		*socket);

/**
 * @brief Gets a @a socket listen backlog parameter.
 * @param socket #PSocket to get the listen backlog parameter for.
 * @return Listen backlog parameter in case of success, -1 otherwise.
 * @since 0.0.1
 * @sa p_socket_set_listen_backlog(), p_socket_listen()
 *
 * This parameter only has meaning for the connection oriented sockets. The
 * backlog parameter specifies how much pending connections from other clients
 * can be stored in the internal (system) queue. If the socket has already the
 * number of pending connections equal to the backlog parameter, and another
 * client attempts to connect on that time, it (client) will either be refused
 * or retransmitted. This behavior is system and protocol dependent.
 *
 * Some systems may not allow to set it to high values. By default #PSocket
 * attempts to set it to 5.
 */
P_LIB_API pint			p_socket_get_listen_backlog	(const PSocket 		*socket);

/**
 * @brief Gets a @a socket timeout for blocking I/O operations.
 * @param socket #PSocket to get the timeout for.
 * @return Timeout for blocking I/O operations in milliseconds, -1 in case of
 * fail.
 * @since 0.0.1
 * @sa p_socket_set_timeout(), p_socket_io_condition_wait()
 *
 * For a blocking socket a timeout value means maximum amount of time for which
 * a blocking call will wait until a newtwork I/O operation completes. If the
 * operation is not finished after the timeout, the blocking call returns with
 * the error set to #P_ERROR_IO_TIMED_OUT.
 *
 * For a non-blocking socket the timeout affects only on the
 * p_socket_io_condition_wait() maximum waiting time.
 *
 * Zero timeout means that the operation which requires a time to complete
 * network I/O will be blocked until the operation finishes or error occurres.
 */
P_LIB_API pint			p_socket_get_timeout		(const PSocket		*socket);

/**
 * @brief Gets a @a socket local (bound) address.
 * @param socket #PSocket to get the local address for.
 * @param[out] error Error report object, NULL to ignore.
 * @return #PSocketAddress with the socket local address in case of success,
 * NULL otherwise.
 * @since 0.0.1
 * @sa p_socket_bind()
 *
 * If the @a socket was not bound explicitly with p_socket_bind() or implicitly
 * with p_socket_connect(), the call will fail.
 */
P_LIB_API PSocketAddress *	p_socket_get_local_address	(const PSocket 		*socket,
								 PError			**error);

/**
 * @brief Gets a @a socket remote endpoint address.
 * @param socket #PSocket to get the remote endpoint address for.
 * @param[out] error Error report object, NULL to ignore.
 * @return #PSocketAddress with the socket endpoint remote address in case of
 * success, NULL otherwise.
 * @since 0.0.1
 * @sa p_socket_connect()
 *
 * If the @a socket was not connected to the endpoint address with
 * p_socket_connect(), the call will fail.
 *
 * @warning On Syllable this call will always return NULL for connection-less
 * sockets (though connecting is possible).
 */
P_LIB_API PSocketAddress *	p_socket_get_remote_address	(const PSocket 		*socket,
								 PError			**error);

/**
 * @brief Checks whether a @a socket is connected.
 * @param socket #PSocket to check a connection for.
 * @return TRUE if the @a socket is connected, FALSE otherwise.
 * @since 0.0.1
 * @sa p_socket_connect(), p_socket_check_connect_result()
 *
 * This function doesn't check if the socket is still connected, it only checks
 * whether the p_socket_connect() call was successfully performed on the
 * @a socket.
 */
P_LIB_API pboolean		p_socket_is_connected		(const PSocket		*socket);

/**
 * @brief Checks whether a @a socket is closed.
 * @param socket #PSocket to check a closed state.
 * @return TRUE if the @a socket is closed, FALSE otherwise.
 * @since 0.0.1
 * @sa p_socket_close(), p_socket_shutdown()
 *
 * If the socket is in a non-blocking mode this call will not return TRUE until
 * p_socket_check_connect_result() is called. The socket will be closed if
 * p_socket_shutdown() is called for both the directions.
 */
P_LIB_API pboolean		p_socket_is_closed		(const PSocket		*socket);

/**
 * @brief Checks a connection state after calling p_socket_connect().
 * @param socket #PSocket to check the connection state for.
 * @param[out] error Error report object, NULL to ignore.
 * @return TRUE if was no error, FALSE otherwise.
 * @since 0.0.1
 * @sa p_socket_io_condition_wait()
 * @warning Not supported on Syllable for connection-less sockets.
 *
 * Usually this call is used after calling p_socket_connect() on a socket in a
 * non-blocking mode to check the connection state. If call returns the FALSE
 * result then the connection checking call has failed or there was an error
 * during the connection and you should check the last error using an @a error
 * object.
 *
 * If the socket is still pending for the connection you will get the
 * #P_ERROR_IO_IN_PROGRESS error code.
 *
 * After calling p_socket_connect() on a non-blocking socket, you can wait for
 * a connection operation to be finished using p_socket_io_condition_wait()
 * with the #P_SOCKET_IO_CONDITION_POLLOUT option.
 */
P_LIB_API pboolean		p_socket_check_connect_result	(PSocket		*socket,
								 PError			**error);

/**
 * @brief Sets the @a socket SO_KEEPALIVE flag.
 * @param socket #PSocket to set the SO_KEEPALIVE flag for.
 * @param keepalive Value for the SO_KEEPALIVE flag.
 * @since 0.0.1
 * @sa p_socket_get_keepalive()
 *
 * See p_socket_get_keepalive() documentation for a description of this option.
 */
P_LIB_API void			p_socket_set_keepalive		(PSocket 		*socket,
								 pboolean		keepalive);

/**
 * @brief Sets a @a socket blocking mode.
 * @param socket #PSocket to set the blocking mode for.
 * @param blocking Whether to set the @a socket into the blocking mode.
 * @note A blocking socket will wait for an I/O operation to be completed
 * before returning to the caller function.
 * @note On some operating systems a blocking timeout may be less than threads
 * scheduling granularity, so the actual timeout can be greater than specified
 * one.
 * @since 0.0.1
 * @sa p_socket_get_blocking()
 */
P_LIB_API void			p_socket_set_blocking		(PSocket 		*socket,
								 pboolean		blocking);

/**
 * @brief Sets a @a socket listen backlog parameter.
 * @param socket #PSocket to set the listen backlog parameter for.
 * @param backlog Value for the listen backlog parameter.
 * @note This parameter can take effect only if it was set before calling
 * p_socket_listen(). Otherwise it will be ignored by underlying socket
 * system calls.
 * @since 0.0.1
 * @sa p_socket_get_listen_backlog()
 *
 * See p_socket_get_listen_backlog() documentation for a description of this
 * option.
 */
P_LIB_API void			p_socket_set_listen_backlog	(PSocket		*socket,
								 pint			backlog);

/**
 * @brief Sets a @a socket timeout value for blocking I/O operations.
 * @param socket #PSocket to set the @a timeout for.
 * @param timeout Timeout value in milliseconds.
 * @since 0.0.1
 * @sa p_socket_get_timeoout(), p_socket_io_condition_wait()
 *
 * See p_socket_get_timeout() documentation for a description of this option.
 */
P_LIB_API void			p_socket_set_timeout		(PSocket		*socket,
								 pint			timeout);

/**
 * @brief Binds a @a socket to a given local address.
 * @param socket #PSocket to bind.
 * @param address #PSocketAddress to bind the given @a socket to.
 * @param allow_reuse Whether to allow socket's address reusing.
 * @param[out] error Error report object, NULL to ignore.
 * @return TRUE in case of success, FALSE otherwise.
 * @since 0.0.1
 * @sa p_socket_get_local_address()
 *
 * The @a allow_reuse option allows to resolve address conflicts for several
 * bound sockets. It controls the SO_REUSEADDR socket flag.
 *
 * In a common case two or more sockets can't be bound to the same address
 * (a network address and a port) for the same data transfer protocol (i.e. TCP
 * or UDP) because they will be in a conflicted state for data receiving. But
 * the socket can be also bound for the any network interface (i.e. 0.0.0.0
 * network address) and a particular port. If you will try to bind another
 * socket without the @a allow_reuse option to a particular network address
 * (i.e. 127.0.0.1) and the same port, the p_socket_bind() call will fail.
 *
 * With the @a allow_reuse option the system will resolve this conflict: the
 * socket will be bound to the particular address and port (and will receive
 * data targeted to this particular address) while the first socket will be
 * receiving all other data matching the bound address.
 *
 * This option is system dependent, some systems do not support it. Also some
 * systems have option to reuse the address port (SO_REUSEPORT) in the same way,
 * too.
 *
 * Connection oriented sockets have another problem - the so called linger time.
 * It is a time required by the system to properly close a socket connection
 * (and this process can be quite complicated). This time can be measured from
 * several minutes to several hours (!). The socket in such a state is
 * half-dead, but it holds the bound address and attempt to bind another socket
 * on this address will fail. The @a allow_reuse option allows to bind another
 * socket on such a half-dead address, but behavior can be unexpected, it's
 * better to refer to the system documentation for that.
 *
 * In general case, a server socket should be bound with the @a allow_reuse set
 * to TRUE, while a client socket shouldn't set this option to TRUE. If you
 * restart the client quickly with the same address it can fail to bind.
 */
P_LIB_API pboolean		p_socket_bind			(const PSocket 		*socket,
								 PSocketAddress		*address,
								 pboolean		allow_reuse,
								 PError			**error);

/**
 * @brief Connects a @a socket to a given remote address.
 * @param socket #PSocket to connect.
 * @param address #PSocketAddress to connect the @a socket to.
 * @param[out] error Error report object, NULL to ignore.
 * @return TRUE in case of success, FALSE otherwise.
 * @since 0.0.1
 * @sa p_socket_is_connected(), p_socket_check_connect_result(),
 * p_socket_get_remote_address(), p_socket_io_condition_wait()
 * @warning On Syllable this method changes a local port of the connection
 * oriented socket in case of success.
 *
 * Calling this method on the connection-less socket will bind it to the remote
 * address and the p_socket_send() method can be used instead of
 * p_socket_send_to(), so you do not need to specify the remote (target) address
 * each time you need to send data. The socket will also discard incoming data
 * from other addresses. The same call again will re-bind it to another remote
 * address.
 *
 * For the connection oriented socket it tries to establish a connection with
 * a listening remote socket. The same call again will have no effect and will
 * fail.
 *
 * If the @a socket is in a non-blocking mode, then you can wait for the
 * connection using p_socket_io_condition_wait() with the
 * #P_SOCKET_IO_CONDITION_POLLOUT parameter. You should check the connection
 * result after that using p_socket_check_connect_result().
 */
P_LIB_API pboolean		p_socket_connect		(PSocket		*socket,
								 PSocketAddress		*address,
								 PError			**error);

/**
 * @brief Puts a @a socket into a listening state.
 * @param socket #PSocket to start listening.
 * @param[out] error Error report object, NULL to ignore.
 * @return TRUE in case of success, FALSE otherwise.
 * @since 0.0.1
 * @sa p_socket_get_listen_backlog(), p_socket_set_listen_backlog()
 *
 * This call has meaning only for connection oriented sockets. Before starting
 * to accept incoming connections, the socket must be put into the passive mode
 * using p_socket_listen(). After that p_socket_accept() can be used to
 * accept incoming connections.
 *
 * Maximum number of pending connections is defined by the backlog parameter,
 * see p_socket_get_listen_backlog() documentation for more information. The
 * backlog parameter must be set before calling p_socket_listen() to take
 * effect.
 */
P_LIB_API pboolean		p_socket_listen			(PSocket 		*socket,
								 PError			**error);

/**
 * @brief Accepts a @a socket incoming connection.
 * @param socket #PSocket to accept the incoming connection from.
 * @param[out] error Error report object, NULL to ignore.
 * @return New #PSocket with the accepted connection in case of success, NULL
 * otherwise.
 * @since 0.0.1
 *
 * This call has meaning only for connection oriented sockets. The socket can
 * accept new incoming connections only after calling p_socket_bind() and
 * p_socket_listen().
 */
P_LIB_API PSocket *		p_socket_accept			(const PSocket		*socket,
								 PError			**error);

/**
 * @brief Receives data from a given @a socket.
 * @param socket #PSocket to receive data from.
 * @param buffer Buffer to write received data in.
 * @param buflen Length of @a buffer.
 * @param[out] error Error report object, NULL to ignore.
 * @return Size in bytes of written data in case of success, -1 otherwise.
 * @note If the @a socket is in a blocking mode, then the caller will be blocked
 * until data arrives.
 * @since 0.0.1
 * @sa p_socket_receive_from(), p_socket_connect()
 *
 * If the @a buflen is less than the received data size, only @a buflen bytes of
 * data will be written to the @a buffer, and excess bytes may be discarded
 * depending on a socket message type.
 *
 * This call is normally used only with the a connected socket, see
 * p_socket_connect().
 */
P_LIB_API pssize		p_socket_receive		(const PSocket		*socket,
								 pchar			*buffer,
								 psize			buflen,
								 PError			**error);

/**
 * @brief Receives data from a given @a socket and saves a remote address.
 * @param socket #PSocket to receive data from.
 * @param[out] address Pointer to store the remote address in case of success,
 * may be NULL. The caller is responsible to free it after usage.
 * @param buffer Buffer to write received data in.
 * @param buflen Length of @a buffer.
 * @param[out] error Error report object, NULL to ignore.
 * @return Size in bytes of written data in case of success, -1 otherwise.
 * @note If the @a socket is in a blocking mode, then the caller will be blocked
 * until data arrives.
 * @since 0.0.1
 * @sa p_socket_receive()
 *
 * If the @a buflen is less than the received data size, only @a buflen bytes of
 * data will be written to the @a buffer, and excess bytes may be discarded
 * depending on a socket message type.
 *
 * This call is normally used only with a connection-less socket.
 */
P_LIB_API pssize		p_socket_receive_from		(const PSocket 		*socket,
								 PSocketAddress		**address,
								 pchar 			*buffer,
								 psize			buflen,
								 PError			**error);

/**
 * @brief Sends data through a given @a socket.
 * @param socket #PSocket to send data through.
 * @param buffer Buffer with data to send.
 * @param buflen Length of @a buffer.
 * @param[out] error Error report object, NULL to ignore.
 * @return Size in bytes of sent data in case of success, -1 otherwise.
 * @note If the @a socket is in a blocking mode, then the caller will be blocked
 * until data sent.
 * @since 0.0.1
 * @sa p_socket_send_to()
 *
 * Do not use this call for connection-less sockets which are not connected to a
 * remote address using p_socket_connect() because it will always fail, use
 * p_socket_send_to() instead.
 */
P_LIB_API pssize		p_socket_send			(const PSocket		*socket,
								 const pchar		*buffer,
								 psize			buflen,
								 PError			**error);

/**
 * @brief Sends data through a given @a socket to a given address.
 * @param socket #PSocket to send data through.
 * @param address #PSocketAddress to send data to.
 * @param buffer Buffer with data to send.
 * @param buflen Length of @a buffer.
 * @param[out] error Error report object, NULL to ignore.
 * @return Size in bytes of sent data in case of success, -1 otherwise.
 * @note If the @a socket is in a blocking mode, then the caller will be blocked
 * until data sent.
 * @since 0.0.1
 * @sa p_socket_send()
 *
 * This call is used when dealing with connection-less sockets. You can bind
 * such a socket to a remote address using p_socket_connect() and use
 * p_socket_send() instead. If you are working with connection oriented sockets
 * then use p_socket_send() after establishing a connection.
 */
P_LIB_API pssize		p_socket_send_to		(const PSocket		*socket,
								 PSocketAddress		*address,
								 const pchar		*buffer,
								 psize			buflen,
								 PError			**error);

/**
 * @brief Closes a @a socket.
 * @param socket #PSocket to close.
 * @param[out] error Error report object, NULL to ignore.
 * @return TRUE in case of success, FALSE otherwise.
 * @since 0.0.1
 * @sa p_socket_free(), p_socket_is_closed()
 *
 * For connection oriented sockets some time is required to completely close
 * a socket connection. See documentation for p_socket_bind() for more
 * information.
 */
P_LIB_API pboolean		p_socket_close			(PSocket		*socket,
								 PError			**error);

/**
 * @brief Shutdowns a full-duplex @a socket data transfer link.
 * @param socket #PSocket to shutdown link.
 * @param shutdown_read Whether to shutdown the incoming data transfer link.
 * @param shutdown_write Whether to shutdown the outcoming data transfer link.
 * @param[out] error Error report object, NULL to ignore.
 * @return TRUE in case of success, FALSE otherwise.
 * @note Shutdown of any link is possible only on the socket in a connected
 * state, otherwise the call will fail.
 * @since 0.0.1
 *
 * After shutdowning the data transfer link couldn't be restored in any
 * direction. It is often used to gracefully close a connection for a connection
 * oriented socket.
 */
P_LIB_API pboolean		p_socket_shutdown		(PSocket		*socket,
								 pboolean		shutdown_read,
								 pboolean		shutdown_write,
								 PError			**error);

/**
 * @brief Closes a @a socket (if not closed yet) and frees its resources.
 * @param socket #PSocket to free resources from.
 * @since 0.0.1
 * @sa p_socket_close()
 */
P_LIB_API void			p_socket_free			(PSocket 		*socket);

/**
 * @brief Sets the @a socket buffer size for a given data transfer direction.
 * @param socket #PSocket to set the buffer size for.
 * @param dir Direction to set the buffer size on.
 * @param size Size of the buffer to set, in bytes.
 * @param[out] error Error report object, NULL to ignore.
 * @return TRUE in case of success, FALSE otherwise.
 * @since 0.0.1
 * @warning Not supported on Syllable.
 */
P_LIB_API pboolean		p_socket_set_buffer_size	(const PSocket		*socket,
								 PSocketDirection	dir,
								 psize			size,
								 PError			**error);

/**
 * @brief Waits for a specified I/O @a condition on @a socket.
 * @param socket #PSocket to wait for @a condition on.
 * @param condition An I/O condition to wait for on @a socket.
 * @param[out] error Error report object, NULL to ignore.
 * @return TRUE if @a condition has been met, FALSE otherwise.
 * @since 0.0.1
 * @sa p_socket_get_timeout(), p_socket_set_timeout()
 *
 * Waits until @a condition will be met on @a socket or an error occurred. If
 * timeout was set using p_socket_set_timeout() and a network I/O operation
 * doesn't finish until timeout expired, call will fail with
 * #P_ERROR_IO_TIMED_OUT error code.
 */
P_LIB_API pboolean		p_socket_io_condition_wait	(const PSocket		*socket,
								 PSocketIOCondition	condition,
								 PError			**error);

P_END_DECLS

#endif /* PLIBSYS_HEADER_PSOCKET_H */
