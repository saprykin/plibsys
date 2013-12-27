/* 
 * Copyright (C) 2008 Christian Kellner, Samuel Cormier-Iijima
 * Copyright (C) 2009 Codethink Limited
 * Copyright (C) 2009 Red Hat, Inc
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

/* TODO: handle SIGPIPE signal */

#include "pmem.h"
#include "psocket.h"

#ifndef P_OS_WIN
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <sys/poll.h>
#else
#include <winsock2.h>
#include <windows.h>
#endif

#include <stdlib.h>
#include <string.h>

/* On old Solaris systems SOMAXCONN is set to 5 */
#define P_SOCKET_DEFAULT_BACKLOG	5

struct _PSocket {
	PSocketFamily	family;
	PSocketProtocol	protocol;
	PSocketType	type;
	PSocketError	error;
	pint		fd;
	pint		listen_backlog;
	puint		inited		: 1;
	puint		blocking	: 1;
	puint		keepalive	: 1;
	puint		closed		: 1;
	puint		connected	: 1;
	puint		listening	: 1;
#ifdef P_OS_WIN
	WSAEVENT	events;
#endif
};

#ifndef SHUT_RD
#  define SHUT_RD			0
#endif

#ifndef SHUT_WR
#  define SHUT_WR			1
#endif

#ifndef SHUT_RDWR
#  define SHUT_RDWR			2
#endif

#ifdef MSG_NOSIGNAL
#  define P_SOCKET_DEFAULT_SEND_FLAGS	MSG_NOSIGNAL
#else
#  define P_SOCKET_DEFAULT_SEND_FLAGS	0
#endif

#ifdef P_OS_WIN
static PSocketError __p_socket_get_error_win (pint err_code);
#else
static PSocketError __p_socket_get_error_unix (pint err_code);
#endif

static pint __p_socket_get_errno (void);
static void __p_socket_set_error (PSocket *socket);
static pint __p_socket_set_fd_blocking (pint fd, pboolean blocking);
static pboolean __p_socket_check (PSocket *socket);
static void __p_socket_set_details_from_fd (PSocket *socket);


#ifdef P_OS_WIN
static PSocketError __p_socket_get_error_win (pint err_code)
{
	switch (err_code) {
	case WSAEADDRINUSE:
		return P_SOCKET_ERROR_ADDRESS_IN_USE;
	case WSAEWOULDBLOCK:
		return P_SOCKET_ERROR_WOULD_BLOCK;
	case WSAEACCES:
		return P_SOCKET_ERROR_ACCESS_DENIED;
	case WSA_INVALID_HANDLE:
	case WSA_INVALID_PARAMETER:
	case WSAEBADF:
	case WSAENOTSOCK:
		return P_SOCKET_ERROR_INVALID_ARGUMENT;
	case WSAESOCKTNOSUPPORT:
	case WSAEOPNOTSUPP:
	case WSAEPFNOSUPPORT:
	case WSAEAFNOSUPPORT:
	case WSAEPROTONOSUPPORT:
		return P_SOCKET_ERROR_NOT_SUPPORTED;
	case WSAECANCELLED:
		return P_SOCKET_ERROR_ABORTED;
	default:
		return P_SOCKET_ERROR_FAILED;
	}
}
#endif /* P_OS_WIN */

#ifndef P_OS_WIN
static PSocketError
__p_socket_get_error_unix (pint err_code)
{
	switch (err_code) {
#ifdef EACCES
	case EACCES:
		return P_SOCKET_ERROR_ACCESS_DENIED;
		break;
#endif

#ifdef EPERM
	case EPERM:
		return P_SOCKET_ERROR_ACCESS_DENIED;
		break;
#endif

#ifdef ENOMEM
	case ENOMEM:
		return P_SOCKET_ERROR_NO_RESOURCES;
		break;
#endif

#ifdef ENOSR
	case ENOSR:
		return P_SOCKET_ERROR_NO_RESOURCES;
		break;
#endif

#ifdef ENOBUFS
	case ENOBUFS:
		return P_SOCKET_ERROR_NO_RESOURCES;
		break;
#endif

#ifdef ENFILE
	case ENFILE:
		return P_SOCKET_ERROR_NO_RESOURCES;
		break;
#endif

#ifdef EMFILE
	case EMFILE:
		return P_SOCKET_ERROR_NO_RESOURCES;
		break;
#endif

#ifdef EINVAL
	case EINVAL:
		return P_SOCKET_ERROR_INVALID_ARGUMENT;
		break;
#endif

#ifdef EBADF
	case EBADF:
		return P_SOCKET_ERROR_INVALID_ARGUMENT;
		break;
#endif

#ifdef ENOTSOCK
	case ENOTSOCK:
		return P_SOCKET_ERROR_INVALID_ARGUMENT;
		break;
#endif

#ifdef EFAULT
	case EFAULT:
		return P_SOCKET_ERROR_INVALID_ARGUMENT;
		break;
#endif

#ifdef EPROTOTYPE
	case EPROTOTYPE:
		return P_SOCKET_ERROR_INVALID_ARGUMENT;
		break;
#endif

	/* On Linux these errors can have same codes */
#if defined(ENOTSUP) && (!defined(EOPNOTSUPP) || ENOTSUP != EOPNOTSUPP)
	case ENOTSUP:
		return P_SOCKET_ERROR_NOT_SUPPORTED;
		break;
#endif

#ifdef ENOPROTOOPT
	case ENOPROTOOPT:
		return P_SOCKET_ERROR_NOT_SUPPORTED;
		break;
#endif

#ifdef EPROTONOSUPPORT
	case EPROTONOSUPPORT:
		return P_SOCKET_ERROR_NOT_SUPPORTED;
		break;
#endif

#ifdef EAFNOSUPPORT
	case EAFNOSUPPORT:
		return P_SOCKET_ERROR_NOT_SUPPORTED;
		break;
#endif

#ifdef EOPNOTSUPP
	case EOPNOTSUPP:
		return P_SOCKET_ERROR_NOT_SUPPORTED;
		break;
#endif

#ifdef EADDRNOTAVAIL
	case EADDRNOTAVAIL:
		return P_SOCKET_ERROR_NOT_AVAILABLE;
		break;
#endif

#ifdef ENETUNREACH
	case ENETUNREACH:
		return P_SOCKET_ERROR_NOT_AVAILABLE;
		break;
#endif

#ifdef ENETDOWN
	case ENETDOWN:
		return P_SOCKET_ERROR_NOT_AVAILABLE;
		break;
#endif

#ifdef EHOSTDOWN
	case EHOSTDOWN:
		return P_SOCKET_ERROR_NOT_AVAILABLE;
		break;
#endif

#ifdef ENONET
	case ENONET:
		return P_SOCKET_ERROR_NOT_AVAILABLE;
		break;
#endif

#ifdef EHOSTUNREACH
	case EHOSTUNREACH:
		return P_SOCKET_ERROR_NOT_AVAILABLE;
		break;
#endif

#ifdef EINPROGRESS
	case EINPROGRESS:
		return P_SOCKET_ERROR_CONNECTING;
		break;
#endif

#ifdef EALREADY
	case EALREADY:
		return P_SOCKET_ERROR_CONNECTING;
		break;
#endif

#ifdef EISCONN
	case EISCONN:
		return P_SOCKET_ERROR_CONNECTED;
		break;
#endif

#ifdef ECONNREFUSED
	case ECONNREFUSED:
		return P_SOCKET_ERROR_CONNECTION_REFUSED;
		break;
#endif

#ifdef ENOTCONN
	case ENOTCONN:
		return P_SOCKET_ERROR_NOT_CONNECTED;
		break;
#endif

#ifdef ECONNABORTED
	case ECONNABORTED:
		return P_SOCKET_ERROR_ABORTED;
		break;
#endif

#ifdef EADDRINUSE
	case EADDRINUSE:
		return P_SOCKET_ERROR_ADDRESS_IN_USE;
		break;
#endif

#ifdef ETIMEDOUT
	case ETIMEDOUT:
		return P_SOCKET_ERROR_TIMED_OUT;
		break;
#endif

		/* Some magic to deal with EWOULDBLOCK and EAGAIN.
		 * Apparently on HP-UX these are actually defined to different values,
		 * but on Linux, for example, they are the same. */
#if defined(EWOULDBLOCK) && defined(EAGAIN) && EWOULDBLOCK == EAGAIN
		/* We have both and they are the same: only emit one case. */
	case EAGAIN:
		return P_SOCKET_ERROR_WOULD_BLOCK;
		break;
#else
		/* Else: consider each of them separately. This handles both the
		 * case of having only one and the case where they are different values. */
# ifdef EAGAIN
	case EAGAIN:
		return P_SOCKET_ERROR_WOULD_BLOCK;
		break;
# endif

# ifdef EWOULDBLOCK
	case EWOULDBLOCK:
		return P_SOCKET_ERROR_WOULD_BLOCK;
		break;
# endif
#endif

	default:
		return P_SOCKET_ERROR_FAILED;
		break;
	}
}
#endif /* !P_OS_WIN */

static pint __p_socket_get_errno (void)
{
#ifdef P_OS_WIN
	return WSAGetLastError ();
#else
	return errno;
#endif
}

static void
__p_socket_set_error (PSocket *socket)
{
	if (!socket)
		return;

#ifdef P_OS_WIN
	socket->error = __p_socket_get_error_win (__p_socket_get_errno ());
#else
	socket->error = __p_socket_get_error_unix (__p_socket_get_errno ());
#endif
}

static pint
__p_socket_set_fd_blocking (pint	fd,
			    pboolean	blocking)
{
#ifndef P_OS_WIN
	pint32 arg;
#else
	pulong arg;
#endif

#ifndef P_OS_WIN
	if ((arg = fcntl (fd, F_GETFL, NULL)) < 0) {
		P_WARNING ("PSocket: error getting socket flags");
		arg = 0;
	}

	arg = (!blocking) ? (arg | O_NONBLOCK) : (arg & ~O_NONBLOCK);

	if (fcntl (fd, F_SETFL, arg) < 0) {
#else
	arg = !blocking;

	if (ioctlsocket (fd, FIONBIO, &arg) == SOCKET_ERROR) {
#endif
		P_ERROR ("PSocket: error setting socket flags");
		return __p_socket_get_errno ();
	}

	return 0;
}

static pboolean
__p_socket_check (PSocket *socket)
{
	if (!socket->inited) {
		P_ERROR ("PSocket: invalid socket, not initialized");
		return FALSE;
	}

	if (socket->closed)  {
		P_ERROR ("PSocket: socket is already closed");
		return FALSE;
	}
	return TRUE;
}

static void
__p_socket_set_details_from_fd (PSocket *socket)
{
	struct sockaddr_storage	address;
	pint			fd, value;
	socklen_t		addrlen, optlen;
#ifdef P_OS_WIN
	/* See comment below */
	BOOL			bool_val = FALSE;
#else
	pint			bool_val;
#endif

	fd = socket->fd;
	optlen = sizeof value;
	if (getsockopt (fd, SOL_SOCKET, SO_TYPE, (void *) &value, &optlen) != 0) {
		P_ERROR ("PSocket: failed to get socket info for fd");

		__p_socket_set_error (socket);

		return;
	}

	if (optlen != sizeof value) {
		P_ERROR ("PSocket: failed to get socket info for fd");
		return;
	}

	switch (value) {
	case SOCK_STREAM:
		socket->type = P_SOCKET_TYPE_STREAM;
		break;

	case SOCK_DGRAM:
		socket->type = P_SOCKET_TYPE_DATAGRAM;
		break;

	case SOCK_SEQPACKET:
		socket->type = P_SOCKET_TYPE_SEQPACKET;
		break;

	default:
		socket->type = P_SOCKET_TYPE_UNKNOWN;
		break;
	}

	addrlen = sizeof address;
	if (getsockname (fd, (struct sockaddr *) &address, &addrlen) != 0) {
		P_ERROR ("PSocket: failed to get socket address info");

		__p_socket_set_error (socket);

		return;
	}

	switch (address.ss_family) {
	case P_SOCKET_FAMILY_INET:
		socket->family = P_SOCKET_FAMILY_INET;
		break;
#ifdef AF_INET6
	case P_SOCKET_FAMILY_INET6:
		socket->family = P_SOCKET_FAMILY_INET6;
		break;
#endif
	default:
		socket->family = P_SOCKET_FAMILY_UNKNOWN;
		break;
	}

	if (socket->family != P_SOCKET_FAMILY_UNKNOWN) {
		addrlen = sizeof address;
		if (getpeername (fd, (struct sockaddr *) &address, &addrlen) >= 0)
			socket->connected = TRUE;
	}

	optlen = sizeof bool_val;
	if (getsockopt (fd, SOL_SOCKET, SO_KEEPALIVE, (void *) &bool_val, &optlen) == 0)  {
#ifndef P_OS_WIN
		/* Experimentation indicates that the SO_KEEPALIVE value is
		 * actually a char on Windows, even if documentation claims it
		 * to be a BOOL which is a typedef for int. */
		if (optlen != sizeof bool_val)
			P_WARNING ("PSocket: failed to get socket keepalive flag");
#endif
		socket->keepalive = !!bool_val;
	}  else
		/* Can't read, maybe not supported, assume FALSE */
		socket->keepalive = FALSE;
}

pboolean
__p_socket_init_once (void)
{
#ifdef P_OS_WIN
	WORD	ver_req;
	WSADATA	wsa_data;
	pint	err;
 
	ver_req = MAKEWORD (2, 2);
 
	if ((err = WSAStartup (ver_req, &wsa_data)) != 0)
		return FALSE;
 
	if (LOBYTE (wsa_data.wVersion) != 2 || HIBYTE (wsa_data.wVersion ) != 2 ) {
		WSACleanup( );
		return FALSE;
	}
#else
#  ifdef SIGPIPE
	signal (SIGPIPE, SIG_IGN);
#  endif	
#endif
	return TRUE;
}

void
__p_socket_close_once (void)
{
#ifdef P_OS_WIN
	WSACleanup ();
#endif
}

P_LIB_API PSocket *
p_socket_new_from_fd (pint fd)
{
	PSocket *ret;

	if (fd < 0) {
		P_ERROR ("PSocket: unable to create socket from bad fd");
		return NULL;
	}

	if ((ret = p_malloc0 (sizeof (PSocket))) == NULL) {
		P_ERROR ("PSocket: failed to allocate memory");
		return NULL;
	}

	ret->fd = fd;

	__p_socket_set_details_from_fd (ret);

	if (__p_socket_set_fd_blocking (ret->fd, FALSE) != 0)
		__p_socket_set_error (ret);

	p_socket_set_listen_backlog (ret, P_SOCKET_DEFAULT_BACKLOG);
	
	ret->blocking = FALSE;
	ret->inited = TRUE;

#ifdef P_OS_WIN
	if ((ret->events = WSACreateEvent ()) == WSA_INVALID_EVENT) {
		P_ERROR ("PSocket: WSACreateEvent failed");
		p_socket_close (ret);
		p_socket_free (ret);
		return NULL;
	}
#endif

	return ret;
}

P_LIB_API PSocket *
p_socket_new (PSocketFamily	family,
	      PSocketType	type,
	      PSocketProtocol	protocol)
{
	PSocket	*ret;
	pint	native_type, fd;
#ifndef P_OS_WIN
	pint	flags;
#endif

	if (family == P_SOCKET_FAMILY_UNKNOWN
	    || type == P_SOCKET_TYPE_UNKNOWN
	    || protocol == P_SOCKET_PROTOCOL_UNKNOWN)
		return NULL;

	switch (type) {
	case P_SOCKET_TYPE_STREAM:
		native_type = SOCK_STREAM;
		break;

	case P_SOCKET_TYPE_DATAGRAM:
		native_type = SOCK_DGRAM;
		break;

	case P_SOCKET_TYPE_SEQPACKET:
		native_type = SOCK_SEQPACKET;
		break;

	default:
		P_ERROR ("PSocket: unable to create socket with unknown family");
		return NULL;
	}

	if (protocol == -1)  {
		P_ERROR ("PSocket: unable to create socket with unknown protocol");
		return NULL;
	}

#ifdef SOCK_CLOEXEC
	native_type |= SOCK_CLOEXEC;
#endif
	if ((fd = (pint) socket (family, native_type, protocol)) < 0) {
		P_ERROR ("PSocket: failed to create socket");
		return NULL;
	}

#ifndef P_OS_WIN
	flags = fcntl (fd, F_GETFD, 0);
	if (flags != -1 && (flags & FD_CLOEXEC) == 0) {
		flags |= FD_CLOEXEC;
		fcntl (fd, F_SETFD, flags);
	}
#endif
	
	if ((ret = p_malloc0 (sizeof (PSocket))) == NULL) {
		P_ERROR ("PSocket: failed to allocate memory");
#ifndef P_OS_WIN
		close (fd);
#else
		closesocket (fd);
#endif
		return NULL;
	}

	if (__p_socket_set_fd_blocking (fd, FALSE) != 0)
		__p_socket_set_error (ret);

	ret->fd = fd;
	ret->blocking = FALSE;
	ret->family = family;
	ret->protocol = protocol;
	ret->type = type;
	ret->inited = TRUE;

	p_socket_set_listen_backlog (ret, P_SOCKET_DEFAULT_BACKLOG);

#ifdef P_OS_WIN
	if ((ret->events = WSACreateEvent ()) == WSA_INVALID_EVENT) {
		P_ERROR ("PSocket: WSACreateEvent failed");
		p_socket_close (ret);
		p_socket_free (ret);
		return NULL;
	}
#endif

	return ret;
}

P_LIB_API pint
p_socket_get_fd (PSocket *socket)
{
	if (!socket)
		return -1;

	return socket->fd;
}

P_LIB_API PSocketFamily
p_socket_get_family (PSocket *socket)
{
	if (!socket)
		return P_SOCKET_FAMILY_UNKNOWN;

	return socket->family;
}

P_LIB_API PSocketType
p_socket_get_type (PSocket *socket)
{
	if (!socket)
		return P_SOCKET_TYPE_UNKNOWN;

	return socket->type;
}

P_LIB_API PSocketProtocol
p_socket_get_protocol (PSocket *socket)
{
	if (!socket)
		return P_SOCKET_PROTOCOL_UNKNOWN;

	return socket->protocol;
}

P_LIB_API pboolean
p_socket_get_keepalive (PSocket *socket)
{
	if (!socket)
		return FALSE;

	return socket->keepalive;
}

P_LIB_API pboolean
p_socket_get_blocking (PSocket *socket)
{
	if (!socket)
		return FALSE;

	return socket->blocking;
}

P_LIB_API int
p_socket_get_listen_backlog (PSocket *socket)
{
	if (!socket)
		return -1;

	return socket->listen_backlog;
}

P_LIB_API PSocketAddress *
p_socket_get_local_address (PSocket *socket)
{
	struct sockaddr_storage	buffer;
	socklen_t		len;

	if (!socket)
		return NULL;

	len = sizeof (buffer);

	if (getsockname (socket->fd, (struct sockaddr *) &buffer, &len) < 0) {
		P_ERROR ("PSocket: unable to get local socket address");

		__p_socket_set_error (socket);

		return NULL;
	}

	return p_socket_address_new_from_native (&buffer, (size_t) len);
}

P_LIB_API PSocketAddress *
p_socket_get_remote_address (PSocket *socket)
{
	struct sockaddr_storage	buffer;
	socklen_t 		len;

	if (!socket)
		return NULL;

	len = sizeof (buffer);

	if (getpeername (socket->fd, (struct sockaddr *) &buffer, &len) < 0) {
		P_ERROR ("PSocket: unable to get remote socket address");

		__p_socket_set_error (socket);

		return NULL;
	}

	return p_socket_address_new_from_native (&buffer, (size_t) len);
}

P_LIB_API pboolean
p_socket_is_connected (PSocket *socket)
{
	if (!socket)
		return FALSE;

	return socket->connected;
}

P_LIB_API void
p_socket_set_keepalive (PSocket		*socket,
			pboolean	keepalive)
{
	pint val;

	if (!socket)
		return;

	if (socket->keepalive == keepalive)
		return;

	val = !! (pint) keepalive;
	if (setsockopt (socket->fd, SOL_SOCKET, SO_KEEPALIVE, &val, sizeof (val)) < 0) {
		P_ERROR ("PSocket: failed to set keepalive flag");
		__p_socket_set_error (socket);

		return;
	}

	socket->keepalive = !! (pint) keepalive;
}

P_LIB_API void
p_socket_set_blocking (PSocket	*socket,
		       pboolean	blocking)
{
	if (!socket)
		return;

	if (socket->blocking == blocking)
		return;

	if (__p_socket_set_fd_blocking (socket->fd, blocking) != 0)
		__p_socket_set_error (socket);

	socket->blocking = blocking;
}

P_LIB_API void
p_socket_set_listen_backlog (PSocket	*socket,
			     pint	backlog)
{
	if (!socket || socket->listening)
		return;

	if (socket->listen_backlog != backlog)
		socket->listen_backlog = backlog;
}

P_LIB_API pboolean
p_socket_bind (PSocket		*socket,
	       PSocketAddress	*address,
	       pboolean		allow_reuse)
{
	struct sockaddr_storage	addr;
#ifndef P_OS_WIN
	pint			value;
#endif

	if (!socket || !address)
		return FALSE;

	if (!__p_socket_check (socket))
		return FALSE;

	/* SO_REUSEADDR on Windows means something else and is not what we want.
	   It always allows the UNIX variant of SO_REUSEADDR anyway */
#ifndef P_OS_WIN
	value = !! (pint) allow_reuse;
	/* Ignore errors here, the only likely error is "not supported", and
	   this is a "best effort" thing mainly */
	setsockopt (socket->fd, SOL_SOCKET, SO_REUSEADDR, &value, sizeof (value));
#endif

	if (!p_socket_address_to_native (address, &addr, sizeof addr))
		return FALSE;

	if (bind (socket->fd, (struct sockaddr *) &addr, (pint) p_socket_address_get_native_size (address)) < 0) {
		P_ERROR ("PSocket: failed to bind socket address");

		__p_socket_set_error (socket);

		return FALSE;
	}

	return TRUE;
}

P_LIB_API pboolean
p_socket_connect (PSocket		*socket,
		  PSocketAddress	*address)
{
	struct sockaddr_storage	buffer;

	if (!socket || !address)
		return FALSE;

	if (!__p_socket_check (socket))
		return FALSE;

	if (!p_socket_address_to_native (address, &buffer, sizeof buffer))
		return FALSE;

	while (TRUE) {
		if (connect (socket->fd, (struct sockaddr *) &buffer,
			     (pint) p_socket_address_get_native_size (address)) < 0) {
#ifndef P_OS_WIN
			if (__p_socket_get_errno () == EINTR)
				continue;
#endif
			P_ERROR ("PSocket: failed to connect");

			__p_socket_set_error (socket);

			return FALSE;
		}

		break;
	}

	socket->connected = TRUE;
	return TRUE;
}

P_LIB_API pboolean
p_socket_listen (PSocket *socket)
{
	if (!socket)
		return FALSE;

	if (!__p_socket_check (socket))
		return FALSE;

	if (listen (socket->fd, socket->listen_backlog) < 0) {
		P_ERROR ("PSocket: unable to listen");

		__p_socket_set_error (socket);

		return FALSE;
	}

	socket->listening = TRUE;
	return TRUE;
}

P_LIB_API PSocket *
p_socket_accept (PSocket *socket)
{
	PSocket		*ret;
	pint		res;
#ifndef P_OS_WIN
	pint		flags;
#endif

	if (!socket)
		return NULL;

	if (!__p_socket_check (socket))
		return NULL;

	while (TRUE) {
		if ((res = (pint) accept (socket->fd, NULL, 0)) < 0) {
#ifndef P_OS_WIN
			if (__p_socket_get_errno () == EINTR)
				continue;
#endif
			P_ERROR ("PSocket: failed to accept");

			__p_socket_set_error (socket);

			return NULL;
		}

		break;
	}

#ifdef P_OS_WIN
	/* The socket inherits the accepting sockets event mask and even object,
	   we need to remove that */
	WSAEventSelect (res, NULL, 0);
#else
	flags = fcntl (res, F_GETFD, 0);
	if (flags != -1 &&  (flags & FD_CLOEXEC) == 0) {
		flags |= FD_CLOEXEC;
		fcntl (res, F_SETFD, flags);
	}
#endif

	if ((ret = p_socket_new_from_fd (res)) == NULL) {
#ifdef P_OS_WIN
		closesocket (res);
#else
		close (res);
#endif
	} else
		ret->protocol = socket->protocol;

	return ret;
}

P_LIB_API pssize
p_socket_receive (PSocket	*socket,
		  pchar		*buffer,
		  psize		buflen)
{
	pssize			ret;
	pint			evret, timeout;
#ifndef P_OS_WIN
	struct pollfd		pfd;
#endif

	if (!socket || !buffer)
		return -1;

	if (!__p_socket_check (socket))
		return -1;

	timeout = (socket->blocking) ? 250 : 1;

#ifdef P_OS_WIN
	WSAResetEvent (socket->events);
	WSAEventSelect (socket->fd, socket->events, FD_READ);

	while (TRUE) {
		evret = WSAWaitForMultipleEvents (1, (const HANDLE*) &socket->events, TRUE, timeout, FALSE);
		
		if (evret == WSA_WAIT_FAILED) {
			ret = -1;
			__p_socket_set_error (socket);
			P_ERROR ("PSocket: WSAWaitForMultipleEvents failed");
			break;
		} else if (evret == WSA_WAIT_TIMEOUT) {
			if (!socket->blocking) {
				__p_socket_set_error (socket);
				ret = -1;
				break;
			}
		} else if (evret == WSA_WAIT_EVENT_0) {
			ret = recv (socket->fd, buffer, (pint) buflen, 0);

			if (ret < 0)
				__p_socket_set_error (socket);

			break;
		}
	}
#else
	pfd.fd = socket->fd;
	pfd.events = POLLIN;
	pfd.revents = 0;

	while (TRUE) {
		evret = poll (&pfd, 1, timeout);

		if (evret == 1) {
			ret = recv (socket->fd, buffer, buflen, 0);

			if (ret < 0)
				__p_socket_set_error (socket);

			break;
		} else if (evret == 0) {
			if (!socket->blocking) {
				__p_socket_set_error (socket);
				ret = -1;
				break;
			}
		} else if (evret == -1 && errno != EINTR) {
			__p_socket_set_error (socket);
			ret = -1;
			break;
		}
	}
#endif

	return ret;
}

P_LIB_API pssize
p_socket_receive_from (PSocket		*socket,
		       PSocketAddress	**address,
		       pchar		*buffer,
		       psize		buflen)
{
	struct sockaddr_storage sa;
	pssize			ret;
	socklen_t		optlen;
	pint			evret, timeout;
#ifndef P_OS_WIN
	struct pollfd		pfd;
#endif

	if (!socket || !address || !buffer || buflen <= 0)
		return -1;

	if (!__p_socket_check (socket))
		return -1;

	optlen = sizeof (sa);
	timeout = (socket->blocking) ? 250 : 1;

#ifdef P_OS_WIN
	WSAResetEvent (socket->events);
	WSAEventSelect (socket->fd, socket->events, FD_READ);

	while (TRUE) {
		evret = WSAWaitForMultipleEvents (1, (const HANDLE*) &socket->events, TRUE, timeout, FALSE);
		
		if (evret == WSA_WAIT_FAILED) {
			ret = -1;
			__p_socket_set_error (socket);
			P_ERROR ("PSocket: WSAWaitForMultipleEvents failed");
			break;
		} else if (evret == WSA_WAIT_TIMEOUT) {
			if (!socket->blocking) {
				__p_socket_set_error (socket);
				ret = -1;
				break;
			}
		} else if (evret == WSA_WAIT_EVENT_0) {
			ret = recvfrom (socket->fd, buffer, (pint) buflen, 0, (struct sockaddr *) &sa, &optlen);

			if (ret < 0)
				__p_socket_set_error (socket);

			break;
		}
	}
#else
	pfd.fd = socket->fd;
	pfd.events = POLLOUT;
	pfd.revents = 0;

	while (TRUE) {
		evret = poll (&pfd, 1, timeout);

		if (evret == 1) {
			ret = recvfrom (socket->fd, buffer, buflen, 0, (struct sockaddr *) &sa, &optlen);

			if (ret < 0)
				__p_socket_set_error (socket);

			break;
		} else if (evret == 0) {
			if (!socket->blocking) {
				__p_socket_set_error (socket);
				ret = -1;
				break;
			}
		} else if (evret == -1 && errno != EINTR) {
			__p_socket_set_error (socket);
			ret = -1;
			break;
		}
	}
#endif

	if (ret > 0)
		*address = p_socket_address_new_from_native (&sa, optlen);
	
	return ret;
}

P_LIB_API pssize
p_socket_send (PSocket		*socket,
	       const pchar	*buffer,
	       psize		buflen)
{
	pssize			ret;
	pint			evret, timeout;
#ifndef P_OS_WIN
	struct pollfd		pfd;
#endif

	if (!socket || !buffer || buflen <= 0)
		return -1;

	if (!__p_socket_check (socket))
		return -1;

	timeout = (socket->blocking) ? 250 : 1;

#ifdef P_OS_WIN
	WSAResetEvent (socket->events);
	WSAEventSelect (socket->fd, socket->events, FD_WRITE);

	while (TRUE) {
		evret = WSAWaitForMultipleEvents (1, (const HANDLE*) &socket->events, TRUE, timeout, FALSE);
		
		if (evret == WSA_WAIT_FAILED) {
			ret = -1;
			__p_socket_set_error (socket);
			P_ERROR ("PSocket: WSAWaitForMultipleEvents failed");
			break;
		} else if (evret == WSA_WAIT_TIMEOUT) {
			if (!socket->blocking) {
				__p_socket_set_error (socket);
				ret = -1;
				break;
			}
		} else if (evret == WSA_WAIT_EVENT_0) {
			ret = send (socket->fd, buffer, (pint) buflen, P_SOCKET_DEFAULT_SEND_FLAGS);

			if (ret < 0)
				__p_socket_set_error (socket);

			break;
		}
	}
#else
	pfd.fd = socket->fd;
	pfd.events = POLLOUT;
	pfd.revents = 0;

	while (TRUE) {
		evret = poll (&pfd, 1, timeout);

		if (evret == 1) {
			ret = send (socket->fd, buffer, buflen, P_SOCKET_DEFAULT_SEND_FLAGS);

			if (ret < 0)
				__p_socket_set_error (socket);

			break;
		} else if (evret == 0) {
			if (!socket->blocking) {
				__p_socket_set_error (socket);
				ret = -1;
				break;
			}
		} else if (evret == -1 && errno != EINTR) {
			__p_socket_set_error (socket);
			ret = -1;
			break;
		}
	}
#endif

	return ret;
}

P_LIB_API pssize
p_socket_send_to (PSocket		*socket,
		  PSocketAddress	*address,
		  const pchar		*buffer,
		  psize			buflen)
{
	struct sockaddr_storage	sa;
	pssize			ret;
	psize			optlen;
	pint			timeout, evret;
#ifndef P_OS_WIN
	struct pollfd		pfd;
#endif

	if (!socket || !address || !buffer)
		return -1;

	if (!__p_socket_check (socket))
		return -1;

	if (!p_socket_address_to_native (address, &sa, sizeof (sa)))
		return -1;

	optlen = p_socket_address_get_native_size (address);
	timeout = (socket->blocking) ? 250 : 1;

#ifdef P_OS_WIN
	WSAResetEvent (socket->events);
	WSAEventSelect (socket->fd, socket->events, FD_WRITE);

	while (TRUE) {
		evret = WSAWaitForMultipleEvents (1, (const HANDLE*) &socket->events, TRUE, timeout, FALSE);
		
		if (evret == WSA_WAIT_FAILED) {
			ret = -1;
			__p_socket_set_error (socket);
			P_ERROR ("PSocket: WSAWaitForMultipleEvents failed");
			break;
		} else if (evret == WSA_WAIT_TIMEOUT) {
			if (!socket->blocking) {
				__p_socket_set_error (socket);
				ret = -1;
				break;
			}
		} else if (evret == WSA_WAIT_EVENT_0) {
			ret = sendto (socket->fd, buffer, (pint) buflen, 0, (struct sockaddr *) &sa, (pint) optlen);

			if (ret < 0)
				__p_socket_set_error (socket);

			break;
		}
	}
#else
	pfd.fd = socket->fd;
	pfd.events = POLLOUT;
	pfd.revents = 0;

	while (TRUE) {
		evret = poll (&pfd, 1, timeout);

		if (evret == 1) {
			ret = sendto (socket->fd, buffer, buflen, 0, (struct sockaddr *) &sa, optlen);

			if (ret < 0)
				__p_socket_set_error (socket);

			break;
		} else if (evret == 0) {
			if (!socket->blocking) {
				__p_socket_set_error (socket);
				ret = -1;
				break;
			}
		} else if (evret == -1 && errno != EINTR) {
			__p_socket_set_error (socket);
			ret = -1;
			break;
		}
	}
#endif

	return ret;
}

P_LIB_API pboolean
p_socket_close (PSocket *socket)
{
	pint res;

	if (!socket)
		return FALSE;

	if (socket->closed)
		return TRUE;

	if (!__p_socket_check (socket))
		return FALSE;

	while (TRUE) {
#ifdef P_OS_WIN
		res = closesocket (socket->fd);
#else
		res = close (socket->fd);
#endif
		if (res == -1) {
#ifndef P_OS_WIN
			if (__p_socket_get_errno () == EINTR)
				continue;
#endif
			P_ERROR ("PSocket: failed to close socket");

			__p_socket_set_error (socket);

			return FALSE;
		}
		break;
	}

	socket->connected = FALSE;
	socket->closed = TRUE;
	socket->listening = FALSE;

	return TRUE;
}

P_LIB_API pboolean
p_socket_shutdown (PSocket	*socket,
		   pboolean	shutdown_read,
		   pboolean	shutdown_write)
{
	pint how;

	if (!socket)
		return FALSE;

	if (!__p_socket_check (socket))
		return FALSE;

	if (!shutdown_read && !shutdown_write)
		return TRUE;

#ifndef P_OS_WIN
	if (shutdown_read && shutdown_write)
		how = SHUT_RDWR;
	else if (shutdown_read)
		how = SHUT_RD;
	else
		how = SHUT_WR;
#else
	if (shutdown_read && shutdown_write)
		how = SD_BOTH;
	else if (shutdown_read)
		how = SD_RECEIVE;
	else
		how = SD_SEND;
#endif

	if (shutdown (socket->fd, how) != 0) {
		P_ERROR ("PSocket: failed to shutdown socket");

		__p_socket_set_error (socket);

		return FALSE;
	}

	if (shutdown_read && shutdown_write)
		socket->connected = FALSE;

	return TRUE;
}

P_LIB_API void
p_socket_free (PSocket *socket)
{
	if (!socket)
		return;

#ifdef P_OS_WIN
	if (socket->events)
		WSACloseEvent (socket->events);
#endif

	p_socket_close (socket);
	p_free (socket);
}

P_LIB_API PSocketError
p_socket_get_last_error (PSocket *socket)
{
	if (!socket)
		return P_SOCKET_ERROR_NONE;

	return socket->error;
}

P_LIB_API pboolean
p_socket_set_buffer_size (PSocket		*socket,
			  PSocketDirection	dir,
			  psize			size)
{
	pint	optval;

	if (socket == NULL)
		return FALSE;

	optval = (dir == P_SOCKET_DIRECTION_RCV) ? SO_RCVBUF : SO_SNDBUF;

	return setsockopt (socket->fd, SOL_SOCKET, optval, (const char *) &size, sizeof (size)) == 0;
}
