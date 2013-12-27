#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE psocket_test

#include "plib.h"

#include <string.h>

#include <boost/test/unit_test.hpp>

static pchar socket_data[] = "This is a socket test data!";
volatile static pboolean is_sender_working = TRUE;
volatile static pboolean is_receiver_working = TRUE;

typedef struct _SocketTestData {
	puint16		sender_port;
	puint16		receiver_port;
	pboolean	shutdown_channel;
} SocketTestData;

static pboolean test_socket_address_directly (const PSocketAddress *addr, puint16 port)
{
	if (addr == NULL)
		return FALSE;

	pchar *addr_str = p_socket_address_get_address (addr);
	PSocketFamily remote_family = p_socket_address_get_family (addr);
	puint16 remote_port = p_socket_address_get_port (addr);
	psize remote_size = p_socket_address_get_native_size (addr);

	pboolean ret = (strcmp (addr_str, "127.0.0.1") == 0 && remote_family == P_SOCKET_FAMILY_INET &&
			remote_port == port && remote_size > 0) ? TRUE : FALSE;

	p_free (addr_str);

	return ret;
}

static pboolean test_socket_address (PSocket *socket, puint16 port)
{
	/* Test remote address */
	PSocketAddress *remote_addr = p_socket_get_remote_address (socket);

	if (remote_addr == NULL)
		return FALSE;

	pboolean ret = test_socket_address_directly (remote_addr, port);

	p_socket_address_free (remote_addr);

	return ret;
}

static pboolean compare_socket_addresses (const PSocketAddress *addr1, const PSocketAddress *addr2)
{
	if (addr1 == NULL || addr2 == NULL)
		return FALSE;

	pchar *addr_str1 = p_socket_address_get_address (addr1);
	pchar *addr_str2 = p_socket_address_get_address (addr2);

	if (addr_str1 == NULL || addr_str2 == NULL) {
		p_free (addr_str1);
		p_free (addr_str2);

		return FALSE;
	}

	pboolean addr_cmp = (strcmp (addr_str1, addr_str2) == 0 ? TRUE : FALSE);

	p_free (addr_str1);
	p_free (addr_str2);

	if (addr_cmp == FALSE)
		return FALSE;

	if (p_socket_address_get_family (addr1) != p_socket_address_get_family (addr2))
		return FALSE;

	if (p_socket_address_get_native_size (addr1) != p_socket_address_get_native_size (addr2))
		return FALSE;

	return TRUE;
}

static void * udp_socket_sender_thread (void *arg)
{
	pint send_counter = 0;

	if (arg == NULL)
		p_uthread_exit (-1);

	SocketTestData *data = (SocketTestData *) (arg);

	/* Create sender socket */
	PSocket *skt_sender = p_socket_new (P_SOCKET_FAMILY_INET, P_SOCKET_TYPE_DATAGRAM, P_SOCKET_PROTOCOL_UDP);

	if (skt_sender == NULL)
		p_uthread_exit (-1);

	PSocketAddress *addr_sender = p_socket_address_new_loopback (P_SOCKET_FAMILY_INET, data->sender_port);

	if (addr_sender == NULL) {
		p_socket_free (skt_sender);
		p_uthread_exit (-1);
	}

	if (p_socket_bind (skt_sender, addr_sender, FALSE) == FALSE) {
		p_socket_free (skt_sender);
		p_socket_address_free (addr_sender);
		p_uthread_exit (-1);
	} else {
		PSocketAddress *local_addr = p_socket_get_local_address (skt_sender);

		if (local_addr == NULL) {
			p_socket_free (skt_sender);
			p_socket_address_free (addr_sender);
			p_uthread_exit (-1);
		}

		data->sender_port = p_socket_address_get_port (local_addr);

		p_socket_address_free (local_addr);
	}

	p_socket_set_blocking (skt_sender, FALSE);
	p_socket_address_free (addr_sender);

	/* Test that remote address is NULL */
	PSocketAddress *remote_addr = p_socket_get_remote_address (skt_sender);

	if (remote_addr != NULL) {
		p_socket_address_free (remote_addr);
		p_socket_free (skt_sender);
		p_uthread_exit (-1);
	}

	/* Test that we are not connected */
	if (p_socket_is_connected (skt_sender) == TRUE) {
		p_socket_free (skt_sender);
		p_uthread_exit (-1);
	}

	while (is_sender_working == TRUE && data->receiver_port == 0) {
		p_uthread_sleep (1);
		continue;
	}

	PSocketAddress *addr_receiver = NULL;

	if (data->receiver_port != 0)
		addr_receiver = p_socket_address_new ("127.0.0.1", data->receiver_port);

	while (is_sender_working == TRUE) {
		if (data->receiver_port == 0)
			break;

		if (p_socket_send_to (skt_sender, addr_receiver, socket_data, sizeof (socket_data)) == sizeof (socket_data))
			++send_counter;

		p_uthread_sleep (1);
	}

	p_socket_address_free (addr_receiver);
	p_socket_free (skt_sender);
	p_uthread_exit (send_counter);
}

static void * udp_socket_receiver_thread (void *arg)
{
	pchar	recv_buffer[sizeof (socket_data) * 3];
	pint	recv_counter = 0;

	if (arg == NULL)
		p_uthread_exit (-1);

	SocketTestData *data = (SocketTestData *) (arg);

	/* Create receiving socket */
	PSocket *skt_receiver = p_socket_new (P_SOCKET_FAMILY_INET, P_SOCKET_TYPE_DATAGRAM, P_SOCKET_PROTOCOL_UDP);

	if (skt_receiver == NULL)
		p_uthread_exit (-1);

	PSocketAddress *addr_receiver = p_socket_address_new ("127.0.0.1", data->receiver_port);

	if (addr_receiver == NULL) {
		p_socket_free (skt_receiver);
		p_uthread_exit (-1);
	}

	if (p_socket_bind (skt_receiver, addr_receiver, TRUE) == FALSE) {
		p_socket_free (skt_receiver);
		p_socket_address_free (addr_receiver);
		p_uthread_exit (-1);
	} else {
		PSocketAddress *local_addr = p_socket_get_local_address (skt_receiver);

		if (local_addr == NULL) {
			p_socket_free (skt_receiver);
			p_socket_address_free (addr_receiver);
			p_uthread_exit (-1);
		}

		data->receiver_port = p_socket_address_get_port (local_addr);

		p_socket_address_free (local_addr);
	}

	p_socket_set_blocking (skt_receiver, FALSE);
	p_socket_address_free (addr_receiver);

	/* Test that remote address is NULL */
	PSocketAddress *remote_addr = p_socket_get_remote_address (skt_receiver);

	if (remote_addr != NULL) {
		p_socket_address_free (remote_addr);
		p_socket_free (skt_receiver);
		p_uthread_exit (-1);
	}

	/* Test that we are not connected */
	if (p_socket_is_connected (skt_receiver) == TRUE) {
		p_socket_free (skt_receiver);
		p_uthread_exit (-1);
	}

	while (is_receiver_working == TRUE) {
		pssize received = p_socket_receive (skt_receiver, recv_buffer, sizeof (recv_buffer));

		if (received == sizeof (socket_data))
			++recv_counter;
		else if (received > 0) {
			p_socket_free (skt_receiver);
			p_uthread_exit (-1);
		}

		p_uthread_sleep (1);
	}

	p_socket_free (skt_receiver);
	p_uthread_exit (recv_counter);
}

static void * tcp_socket_sender_thread (void *arg)
{
	pint		send_counter = 0;
	pint		send_total, send_now;
	pboolean	is_connected = FALSE;

	if (arg == NULL)
		p_uthread_exit (-1);

	SocketTestData *data = (SocketTestData *) (arg);

	/* Create sender socket */
	PSocket *skt_sender = p_socket_new (P_SOCKET_FAMILY_INET, P_SOCKET_TYPE_STREAM, P_SOCKET_PROTOCOL_DEFAULT);

	if (skt_sender == NULL)
		p_uthread_exit (-1);

	if (p_socket_get_fd (skt_sender) < 0)
		p_uthread_exit (-1);

	while (is_sender_working == TRUE && data->receiver_port == 0) {
		p_uthread_sleep (1);
		continue;
	}

	PSocketAddress *addr_sender = p_socket_address_new_loopback (P_SOCKET_FAMILY_INET, data->sender_port);

	if (addr_sender == NULL) {
		p_socket_free (skt_sender);
		p_uthread_exit (-1);
	}

	if (p_socket_bind (skt_sender, addr_sender, FALSE) == FALSE) {
		p_socket_free (skt_sender);
		p_socket_address_free (addr_sender);
		p_uthread_exit (-1);
	} else {
		PSocketAddress *local_addr = p_socket_get_local_address (skt_sender);

		if (local_addr == NULL) {
			p_socket_free (skt_sender);
			p_socket_address_free (addr_sender);
			p_uthread_exit (-1);
		}

		data->sender_port = p_socket_address_get_port (local_addr);

		p_socket_address_free (local_addr);
	}

	p_socket_set_blocking (skt_sender, FALSE);
	p_socket_address_free (addr_sender);

	send_total = 0;
	send_now = 0;

	while (data->receiver_port == 0) {
		p_uthread_sleep (1);
		continue;
	}

	PSocketAddress *addr_receiver = NULL;

	if (data->receiver_port != 0)
		addr_receiver = p_socket_address_new ("127.0.0.1", data->receiver_port);

	while (is_sender_working == TRUE) {
		if (data->receiver_port == 0)
			break;

		if (is_connected == FALSE) {
			is_connected = p_socket_connect (skt_sender, addr_receiver);

			if (is_connected == FALSE) {
				p_uthread_sleep (1);
				continue;
			} else {
				if (test_socket_address (skt_sender, data->receiver_port) == FALSE)
					break;

				p_socket_set_keepalive (skt_sender, TRUE);

				if (p_socket_get_keepalive (skt_sender) == FALSE)
					break;

				if (p_socket_shutdown (skt_sender, TRUE, data->shutdown_channel) == FALSE)
					break;
			}
		}

		if (data->shutdown_channel == FALSE && p_socket_is_connected (skt_sender) == FALSE) {
			p_socket_free (skt_sender);
			p_uthread_exit (-1);
		}

		send_now = p_socket_send (skt_sender, socket_data + send_total, sizeof (socket_data) - send_total);

		if (send_now > 0)
			send_total += send_now;

		if (send_total == sizeof (socket_data)) {
			send_total = 0;
			++send_counter;
		}

		p_uthread_sleep (1);
	}

	if (p_socket_close (skt_sender) == FALSE)
		send_counter = -1;

	p_socket_address_free (addr_receiver);
	p_socket_free (skt_sender);
	p_uthread_exit (send_counter);
}

static void * tcp_socket_receiver_thread (void *arg)
{
	pchar		recv_buffer[sizeof (socket_data)];
	pint		recv_counter = 0;
	pint		recv_total, recv_now;

	if (arg == NULL)
		p_uthread_exit (-1);

	SocketTestData *data = (SocketTestData *) (arg);

	/* Create receiving socket */
	pint fd = socket (P_SOCKET_FAMILY_INET, SOCK_STREAM, P_SOCKET_PROTOCOL_TCP);

	if (fd < 0)
		p_uthread_exit (-1);

	PSocket *skt_receiver = p_socket_new_from_fd (fd);

	if (skt_receiver == NULL) {
		close (fd);
		p_uthread_exit (-1);
	}

	PSocketAddress *addr_receiver = p_socket_address_new ("127.0.0.1", data->receiver_port);

	if (addr_receiver == NULL) {
		p_socket_free (skt_receiver);
		p_uthread_exit (-1);
	}

	if (p_socket_bind (skt_receiver, addr_receiver, TRUE) == FALSE ||
	    p_socket_listen (skt_receiver) == FALSE) {
		p_socket_free (skt_receiver);
		p_socket_address_free (addr_receiver);
		p_uthread_exit (-1);
	} else {
		PSocketAddress *local_addr = p_socket_get_local_address (skt_receiver);

		if (local_addr == NULL) {
			p_socket_free (skt_receiver);
			p_socket_address_free (addr_receiver);
			p_uthread_exit (-1);
		}

		data->receiver_port = p_socket_address_get_port (local_addr);

		p_socket_address_free (local_addr);
	}

	p_socket_set_blocking (skt_receiver, FALSE);
	p_socket_address_free (addr_receiver);

	PSocket *conn_socket = NULL;
	recv_total = 0;
	recv_now = 0;

	while (is_receiver_working == TRUE) {
		if (conn_socket == NULL) {
			conn_socket = p_socket_accept (skt_receiver);

			if (conn_socket == NULL) {
				p_uthread_sleep (1);
				continue;
			} else {
				if (test_socket_address (conn_socket, data->sender_port) == FALSE)
					break;

				if (p_socket_shutdown (conn_socket, data->shutdown_channel, TRUE) == FALSE)
					break;
			}
		}

		if (data->shutdown_channel == FALSE && p_socket_is_connected (conn_socket) == FALSE) {
			p_socket_free (conn_socket);
			p_socket_free (skt_receiver);
			p_uthread_exit (-1);
		}

		PSocketAddress *remote_addr = NULL;

		recv_now = p_socket_receive_from (conn_socket, &remote_addr, recv_buffer + recv_total, sizeof (recv_buffer) - recv_total);

		if (remote_addr != NULL && test_socket_address_directly (remote_addr, data->sender_port) == TRUE)
			break;

		p_socket_address_free (remote_addr);

		if (recv_now > 0)
			recv_total += recv_now;

		if (recv_total == sizeof (recv_buffer)) {
			recv_total = 0;

			if (strncmp (recv_buffer, socket_data, sizeof (recv_buffer)) == 0)
				++recv_counter;

			memset (recv_buffer, 0, sizeof (recv_buffer));
		}

		p_uthread_sleep (1);
	}

	if (p_socket_close (skt_receiver) == FALSE)
		recv_counter = -1;

	p_socket_free (conn_socket);
	p_socket_free (skt_receiver);

	p_uthread_exit (recv_counter);
}

BOOST_AUTO_TEST_SUITE (BOOST_TEST_MODULE)

BOOST_AUTO_TEST_CASE (psocket_bad_input_test)
{
	p_lib_init ();

	BOOST_CHECK (p_socket_new_from_fd (-1) == NULL);
	BOOST_CHECK (p_socket_new (P_SOCKET_FAMILY_UNKNOWN, P_SOCKET_TYPE_UNKNOWN, P_SOCKET_PROTOCOL_UNKNOWN) == NULL);
	BOOST_CHECK (p_socket_get_fd (NULL) == -1);
	BOOST_CHECK (p_socket_get_family (NULL) == P_SOCKET_FAMILY_UNKNOWN);
	BOOST_CHECK (p_socket_get_type (NULL) == P_SOCKET_TYPE_UNKNOWN);
	BOOST_CHECK (p_socket_get_protocol (NULL) == P_SOCKET_PROTOCOL_UNKNOWN);
	BOOST_CHECK (p_socket_get_keepalive (NULL) == FALSE);
	BOOST_CHECK (p_socket_get_blocking (NULL) == FALSE);
	BOOST_CHECK (p_socket_get_listen_backlog (NULL) == -1);
	BOOST_CHECK (p_socket_get_local_address (NULL) == NULL);
	BOOST_CHECK (p_socket_get_remote_address (NULL) == NULL);
	BOOST_CHECK (p_socket_is_connected (NULL) == FALSE);

	p_socket_set_keepalive (NULL, FALSE);
	p_socket_set_blocking (NULL, FALSE);
	p_socket_set_listen_backlog (NULL, 0);

	BOOST_CHECK (p_socket_bind (NULL, NULL, FALSE) == FALSE);
	BOOST_CHECK (p_socket_connect (NULL, NULL) == FALSE);
	BOOST_CHECK (p_socket_listen (NULL) == FALSE);
	BOOST_CHECK (p_socket_accept (NULL) == NULL);
	BOOST_CHECK (p_socket_receive (NULL, NULL, 0) == -1);
	BOOST_CHECK (p_socket_receive_from (NULL, NULL, NULL, 0) == -1);
	BOOST_CHECK (p_socket_send (NULL, NULL, 0) == -1);
	BOOST_CHECK (p_socket_send_to (NULL, NULL, NULL, 0) == -1);
	BOOST_CHECK (p_socket_close (NULL) == FALSE);
	BOOST_CHECK (p_socket_shutdown (NULL, FALSE, FALSE) == FALSE);
	BOOST_CHECK (p_socket_get_last_error (NULL) == P_SOCKET_ERROR_NONE);
	BOOST_CHECK (p_socket_set_buffer_size (NULL, P_SOCKET_DIRECTION_RCV, 0) == FALSE);
	BOOST_CHECK (p_socket_set_buffer_size (NULL, P_SOCKET_DIRECTION_SND, 0) == FALSE);

	p_socket_free (NULL);

	p_lib_shutdown ();
}

BOOST_AUTO_TEST_CASE (psocket_general_test)
{
	p_lib_init ();

	/* Test UDP socket */
	PSocket *socket = p_socket_new (P_SOCKET_FAMILY_INET, P_SOCKET_TYPE_DATAGRAM, P_SOCKET_PROTOCOL_UDP);
	p_socket_set_listen_backlog (socket, 12);

	BOOST_CHECK (socket != NULL);
	BOOST_CHECK (p_socket_get_family (socket) == P_SOCKET_FAMILY_INET);
	BOOST_CHECK (p_socket_get_fd (socket) >= 0);
	BOOST_CHECK (p_socket_get_listen_backlog (socket) == 12);
	BOOST_CHECK (p_socket_get_last_error (socket) == P_SOCKET_ERROR_NONE);
	BOOST_CHECK (p_socket_get_remote_address (socket) == NULL);
	BOOST_CHECK (p_socket_get_protocol (socket) == P_SOCKET_PROTOCOL_UDP);
	BOOST_CHECK (p_socket_get_blocking (socket) == FALSE);
	BOOST_CHECK (p_socket_get_type (socket) == P_SOCKET_TYPE_DATAGRAM);
	BOOST_CHECK (p_socket_get_keepalive (socket) == FALSE);

	PSocketAddress *sock_addr = p_socket_address_new ("127.0.0.1", 32111);
	BOOST_CHECK (sock_addr != NULL);

	BOOST_CHECK (p_socket_bind (socket, sock_addr, TRUE) == TRUE);

	PSocketAddress *addr = p_socket_get_local_address (socket);
	BOOST_CHECK (addr != NULL);

	BOOST_CHECK (compare_socket_addresses (sock_addr, addr) == TRUE);

	p_socket_address_free (sock_addr);
	p_socket_address_free (addr);

	BOOST_CHECK (p_socket_set_buffer_size (socket, P_SOCKET_DIRECTION_RCV, 72 * 1024) == TRUE);
	BOOST_CHECK (p_socket_set_buffer_size (socket, P_SOCKET_DIRECTION_SND, 72 * 1024) == TRUE);

	BOOST_CHECK (p_socket_is_connected (socket) == FALSE);
	BOOST_CHECK (p_socket_close (socket));
	p_socket_free (socket);

	/* Test TCP socket */
	socket = p_socket_new (P_SOCKET_FAMILY_INET, P_SOCKET_TYPE_STREAM, P_SOCKET_PROTOCOL_TCP);
	p_socket_set_blocking (socket, TRUE);
	p_socket_set_listen_backlog (socket, 11);

	BOOST_CHECK (socket != NULL);
	BOOST_CHECK (p_socket_get_family (socket) == P_SOCKET_FAMILY_INET);
	BOOST_CHECK (p_socket_get_fd (socket) >= 0);
	BOOST_CHECK (p_socket_get_listen_backlog (socket) == 11);
	BOOST_CHECK (p_socket_get_last_error (socket) == P_SOCKET_ERROR_NONE);
	BOOST_CHECK (p_socket_get_remote_address (socket) == NULL);
	BOOST_CHECK (p_socket_get_protocol (socket) == P_SOCKET_PROTOCOL_TCP);
	BOOST_CHECK (p_socket_get_blocking (socket) == TRUE);
	BOOST_CHECK (p_socket_get_type (socket) == P_SOCKET_TYPE_STREAM);
	BOOST_CHECK (p_socket_get_keepalive (socket) == FALSE);

	sock_addr = p_socket_address_new ("127.0.0.1", 0);
	BOOST_CHECK (sock_addr != NULL);

	BOOST_CHECK (p_socket_bind (socket, sock_addr, TRUE) == TRUE);

	addr = p_socket_get_local_address (socket);
	BOOST_CHECK (addr != NULL);

	BOOST_CHECK (compare_socket_addresses (sock_addr, addr) == TRUE);

	p_socket_address_free (sock_addr);
	p_socket_address_free (addr);

	BOOST_CHECK (p_socket_set_buffer_size (socket, P_SOCKET_DIRECTION_RCV, 72 * 1024) == TRUE);
	BOOST_CHECK (p_socket_set_buffer_size (socket, P_SOCKET_DIRECTION_SND, 72 * 1024) == TRUE);

	BOOST_CHECK (p_socket_is_connected (socket) == FALSE);
	BOOST_CHECK (p_socket_close (socket));
	p_socket_free (socket);

	p_lib_shutdown ();
}

BOOST_AUTO_TEST_CASE (psocket_udp_test)
{
	p_lib_init ();

	is_sender_working = TRUE;
	is_receiver_working = TRUE;

	SocketTestData data;
	data.receiver_port = 0;
	data.sender_port = 0;
	data.shutdown_channel = FALSE;

	PUThread *receiver_thr = p_uthread_create ((PUThreadFunc) udp_socket_receiver_thread, (ppointer) &data, TRUE);
	PUThread *sender_thr = p_uthread_create ((PUThreadFunc) udp_socket_sender_thread, (ppointer) &data, TRUE);

	BOOST_CHECK (sender_thr != NULL);
	BOOST_CHECK (receiver_thr != NULL);

	p_uthread_sleep (8000);

	is_sender_working = FALSE;
	pint send_counter = p_uthread_join (sender_thr);

	p_uthread_sleep (2000);

	is_receiver_working = FALSE;
	pint recv_counter = p_uthread_join (receiver_thr);

	BOOST_CHECK (send_counter > 0);
	BOOST_CHECK (recv_counter > 0);
	BOOST_CHECK (abs (send_counter - recv_counter) <= 1);

	p_uthread_free (sender_thr);
	p_uthread_free (receiver_thr);

	p_lib_shutdown ();
}

BOOST_AUTO_TEST_CASE (psocket_tcp_test)
{
	p_lib_init ();

	is_sender_working = TRUE;
	is_receiver_working = TRUE;

	SocketTestData data;
	data.receiver_port = 0;
	data.sender_port = 0;
	data.shutdown_channel = FALSE;

	PUThread *receiver_thr = p_uthread_create ((PUThreadFunc) tcp_socket_receiver_thread, (ppointer) &data, TRUE);
	PUThread *sender_thr = p_uthread_create ((PUThreadFunc) tcp_socket_sender_thread, (ppointer) &data, TRUE);

	BOOST_CHECK (receiver_thr != NULL);
	BOOST_CHECK (sender_thr != NULL);

	p_uthread_sleep (8000);

	is_sender_working = FALSE;
	pint send_counter = p_uthread_join (sender_thr);

	p_uthread_sleep (2000);

	is_receiver_working = FALSE;
	pint recv_counter = p_uthread_join (receiver_thr);

	BOOST_CHECK (send_counter > 0);
	BOOST_CHECK (recv_counter > 0);
	BOOST_CHECK (abs (send_counter - recv_counter) <= 1);

	p_uthread_free (sender_thr);
	p_uthread_free (receiver_thr);

	p_lib_shutdown ();
}

BOOST_AUTO_TEST_CASE (psocket_shutdown_test)
{
	p_lib_init ();

	is_sender_working = TRUE;
	is_receiver_working = TRUE;

	SocketTestData data;
	data.receiver_port = 0;
	data.sender_port = 0;
	data.shutdown_channel = TRUE;

	PUThread *receiver_thr = p_uthread_create ((PUThreadFunc) tcp_socket_receiver_thread, (ppointer) &data, TRUE);
	PUThread *sender_thr = p_uthread_create ((PUThreadFunc) tcp_socket_sender_thread, (ppointer) &data, TRUE);

	BOOST_CHECK (receiver_thr != NULL);
	BOOST_CHECK (sender_thr != NULL);

	p_uthread_sleep (8000);

	is_sender_working = FALSE;
	pint send_counter = p_uthread_join (sender_thr);

	p_uthread_sleep (2000);

	is_receiver_working = FALSE;
	pint recv_counter = p_uthread_join (receiver_thr);

	BOOST_CHECK (send_counter == 0);
	BOOST_CHECK (recv_counter == 0);

	p_uthread_free (sender_thr);
	p_uthread_free (receiver_thr);

	p_lib_shutdown ();
}

BOOST_AUTO_TEST_SUITE_END()
