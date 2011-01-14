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

#if !defined (__PLIB_H_INSIDE__) && !defined (PLIB_COMPILATION)
#  error "Header files shouldn't be included directly, consider using <plib.h> instead."
#endif

#ifndef __PSOCKET_H__
#define __PSOCKET_H__

#include <pmacros.h>
#include <psocketaddress.h>

P_BEGIN_DECLS

typedef enum _PSocketProtocol {
	P_SOCKET_PROTOCOL_UNKNOWN		= -1,
	P_SOCKET_PROTOCOL_DEFAULT		= 0,
	P_SOCKET_PROTOCOL_TCP			= 6,
	P_SOCKET_PROTOCOL_UDP			= 17
} PSocketProtocol;

typedef enum _PSocketError {
	P_SOCKET_ERROR_NONE			= 0,
	P_SOCKET_ERROR_NO_RESOURCES		= 1,
	P_SOCKET_ERROR_NOT_AVAILABLE		= 2,
	P_SOCKET_ERROR_ACCESS_DENIED		= 3,
	P_SOCKET_ERROR_CONNECTED		= 4,
	P_SOCKET_ERROR_CONNECTING		= 5,
	P_SOCKET_ERROR_ABORTED			= 6,
	P_SOCKET_ERROR_INVALID_ARGUMENT		= 7,
	P_SOCKET_ERROR_NOT_SUPPORTED		= 8,
	P_SOCKET_ERROR_TIMED_OUT		= 9,
	P_SOCKET_ERROR_WOULD_BLOCK		= 10,
	P_SOCKET_ERROR_ADDRESS_IN_USE		= 11,
	P_SOCKET_ERROR_CONNECTION_REFUSED	= 12,
	P_SOCKET_ERROR_NOT_CONNECTED		= 13,
	P_SOCKET_ERROR_FAILED			= 14
} PSocketError;

typedef enum _PSocketType {
	P_SOCKET_TYPE_UNKNOWN			= 0,
	P_SOCKET_TYPE_STREAM			= 1,
	P_SOCKET_TYPE_DATAGRAM			= 2,
	P_SOCKET_TYPE_SEQPACKET			= 3
} PSocketType;

typedef enum _PSocketDirection {
	P_SOCKET_DIRECTION_SND			= 0,
	P_SOCKET_DIRECTION_RCV			= 1
} PSocketDirection;

typedef struct _PSocket PSocket;

P_LIB_API PSocket *		p_socket_new_from_fd		(pint 			fd);
P_LIB_API PSocket *		p_socket_new 			(PSocketFamily		family,
								 PSocketType		type,
								 PSocketProtocol	protocol);
P_LIB_API pint			p_socket_get_fd 		(PSocket 		*socket);
P_LIB_API PSocketFamily		p_socket_get_family 		(PSocket 		*socket);
P_LIB_API PSocketType		p_socket_get_type 		(PSocket 		*socket);
P_LIB_API PSocketProtocol	p_socket_get_protocol		(PSocket 		*socket);
P_LIB_API pboolean		p_socket_get_keepalive		(PSocket 		*socket);
P_LIB_API pboolean		p_socket_get_blocking		(PSocket 		*socket);
P_LIB_API pint			p_socket_get_listen_backlog	(PSocket 		*socket);
P_LIB_API PSocketAddress *	p_socket_get_local_address	(PSocket 		*socket);
P_LIB_API PSocketAddress *	p_socket_get_remote_address	(PSocket 		*socket);
P_LIB_API pboolean		p_socket_is_connected		(PSocket 		*socket);
P_LIB_API void			p_socket_set_keepalive		(PSocket 		*socket,
								 pboolean		keepalive);
P_LIB_API void			p_socket_set_blocking		(PSocket 		*socket,
								 pboolean		blocking);
P_LIB_API void			p_socket_set_listen_backlog	(PSocket		*socket,
								 pint			backlog);
P_LIB_API pboolean		p_socket_bind			(PSocket 		*socket,
								 PSocketAddress		*address,
								 pboolean		allow_reuse);
P_LIB_API pboolean		p_socket_connect		(PSocket		*socket,
								 PSocketAddress		*address);
P_LIB_API pboolean		p_socket_check_connect_result	(PSocket 		*socket);
P_LIB_API pboolean		p_socket_listen			(PSocket 		*socket);
P_LIB_API PSocket *		p_socket_accept			(PSocket		*socket);
P_LIB_API pssize		p_socket_receive		(PSocket		*socket,
								 pchar			*buffer,
								 psize			buflen);
P_LIB_API pssize		p_socket_receive_from		(PSocket 		*socket,
								 PSocketAddress		**address,
								 pchar 			*buffer,
								 psize			buflen);
P_LIB_API pssize		p_socket_send			(PSocket		*socket,
								 const pchar		*buffer,
								 psize			buflen);
P_LIB_API pssize		p_socket_send_to		(PSocket		*socket,
								 PSocketAddress		*address,
								 const pchar		*buffer,
								 psize			buflen);
P_LIB_API pboolean		p_socket_close			(PSocket		*socket);
P_LIB_API pboolean		p_socket_shutdown		(PSocket		*socket,
								 pboolean		shutdown_read,
								 pboolean		shutdown_write);
P_LIB_API void			p_socket_free			(PSocket 		*socket);
P_LIB_API PSocketError		p_socket_get_last_error		(PSocket		*socket);
P_LIB_API pboolean		p_socket_set_buffer_size	(PSocket		*socket,
								 PSocketDirection	dir,
								 psize			size);

P_END_DECLS

#endif /* __PSOCKET_H__ */

