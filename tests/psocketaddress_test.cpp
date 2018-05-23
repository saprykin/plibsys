/*
 * The MIT License
 *
 * Copyright (C) 2013-2017 Alexander Saprykin <saprykin.spb@gmail.com>
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

#include "plibsys.h"
#include "ptestmacros.h"

#include <string.h>

P_TEST_MODULE_INIT ();

extern "C" ppointer pmem_alloc (psize nbytes)
{
	P_UNUSED (nbytes);
	return (ppointer) NULL;
}

extern "C" ppointer pmem_realloc (ppointer block, psize nbytes)
{
	P_UNUSED (block);
	P_UNUSED (nbytes);
	return (ppointer) NULL;
}

extern "C" void pmem_free (ppointer block)
{
	P_UNUSED (block);
}

P_TEST_CASE_BEGIN (psocketaddress_nomem_test)
{
	p_libsys_init ();

	PSocketAddress *sock_addr = p_socket_address_new ("192.168.0.1", 1058);
	P_TEST_CHECK (sock_addr != NULL);

	psize native_size = p_socket_address_get_native_size (sock_addr);
	P_TEST_CHECK (native_size > 0);

	ppointer addr_buf = p_malloc0 (native_size);
	P_TEST_CHECK (addr_buf != NULL);

	P_TEST_CHECK (p_socket_address_to_native (sock_addr, addr_buf, native_size - 1) == FALSE);
	P_TEST_CHECK (p_socket_address_to_native (sock_addr, addr_buf, native_size) == TRUE);
	p_socket_address_free (sock_addr);

	PSocketAddress *sock_addr6;
	psize native_size6;
	ppointer addr_buf6;

	if (p_socket_address_is_ipv6_supported ()) {
		sock_addr6 = p_socket_address_new ("2001:cdba:345f:24ab:fe45:5423:3257:9652", 1058);
		P_TEST_CHECK (sock_addr6 != NULL);

		native_size6 = p_socket_address_get_native_size (sock_addr6);
		P_TEST_CHECK (native_size6 > 0);

		addr_buf6 = p_malloc0 (native_size6);
		P_TEST_CHECK (addr_buf6 != NULL);

		P_TEST_CHECK (p_socket_address_to_native (sock_addr6, addr_buf6, native_size6 - 1) == FALSE);
		P_TEST_CHECK (p_socket_address_to_native (sock_addr6, addr_buf6, native_size6) == TRUE);
		p_socket_address_free (sock_addr6);
	}

	PMemVTable vtable;

	vtable.free    = pmem_free;
	vtable.malloc  = pmem_alloc;
	vtable.realloc = pmem_realloc;

	P_TEST_CHECK (p_mem_set_vtable (&vtable) == TRUE);

	P_TEST_CHECK (p_socket_address_new ("192.168.0.1", 1058) == NULL);
	P_TEST_CHECK (p_socket_address_new_any (P_SOCKET_FAMILY_INET, 1058) == NULL);
	P_TEST_CHECK (p_socket_address_new_loopback (P_SOCKET_FAMILY_INET, 1058) == NULL);
	P_TEST_CHECK (p_socket_address_new_from_native (addr_buf, native_size) == NULL);

	if (p_socket_address_is_ipv6_supported ())
		P_TEST_CHECK (p_socket_address_new_from_native (addr_buf6, native_size6) == NULL);

	p_mem_restore_vtable ();

	P_TEST_CHECK (p_socket_address_new_from_native (addr_buf, native_size - 1) == NULL);

	if (p_socket_address_is_ipv6_supported ()) {
		P_TEST_CHECK (p_socket_address_new_from_native (addr_buf6, native_size6 - 1) == NULL);
		p_free (addr_buf6);
	}

	p_free (addr_buf);

	p_libsys_shutdown ();
}
P_TEST_CASE_END ()

P_TEST_CASE_BEGIN (psocketaddress_bad_input_test)
{
	p_libsys_init ();

	P_TEST_CHECK (p_socket_address_new_from_native (NULL, 0) == NULL);
	P_TEST_CHECK (p_socket_address_new (NULL, 0) == NULL);
	P_TEST_CHECK (p_socket_address_new ("bad_address", 0) == NULL);
	P_TEST_CHECK (p_socket_address_new_any (P_SOCKET_FAMILY_UNKNOWN, 0) == NULL);
	P_TEST_CHECK (p_socket_address_new_loopback (P_SOCKET_FAMILY_UNKNOWN, 0) == NULL);
	P_TEST_CHECK (p_socket_address_to_native (NULL, NULL, 0) == FALSE);
	P_TEST_CHECK (p_socket_address_get_native_size (NULL) == 0);
	P_TEST_CHECK (p_socket_address_get_family (NULL) == P_SOCKET_FAMILY_UNKNOWN);
	P_TEST_CHECK (p_socket_address_get_address (NULL) == NULL);
	P_TEST_CHECK (p_socket_address_get_port (NULL) == 0);
	P_TEST_CHECK (p_socket_address_get_flow_info (NULL) == 0);
	P_TEST_CHECK (p_socket_address_get_scope_id (NULL) == 0);
	P_TEST_CHECK (p_socket_address_is_any (NULL) == FALSE);
	P_TEST_CHECK (p_socket_address_is_loopback (NULL) == FALSE);

	p_socket_address_set_flow_info (NULL, 0);
	p_socket_address_set_scope_id (NULL, 0);
	p_socket_address_free (NULL);

	p_libsys_shutdown ();
}
P_TEST_CASE_END ()

P_TEST_CASE_BEGIN (psocketaddress_general_test)
{
	p_libsys_init ();

	/* Test IPv4 LAN address */
	PSocketAddress *addr = p_socket_address_new ("192.168.0.1", 2345);

	P_TEST_REQUIRE (addr != NULL);
	P_TEST_CHECK (p_socket_address_is_loopback (addr) == FALSE);
	P_TEST_CHECK (p_socket_address_is_any (addr) == FALSE);
	P_TEST_CHECK (p_socket_address_get_family (addr) == P_SOCKET_FAMILY_INET);
	P_TEST_CHECK (p_socket_address_get_port (addr) == 2345);
	P_TEST_CHECK (p_socket_address_get_native_size (addr) > 0);
	P_TEST_CHECK (p_socket_address_get_flow_info (addr) == 0);
	P_TEST_CHECK (p_socket_address_get_scope_id (addr) == 0);

	pchar *addr_str = p_socket_address_get_address (addr);

	P_TEST_REQUIRE (addr_str != NULL);
	P_TEST_CHECK (strcmp (addr_str, "192.168.0.1") == 0);

	p_free (addr_str);
	p_socket_address_free (addr);

	if (p_socket_address_is_ipv6_supported ()) {
		/* Test IPv6 LAN address */
		addr = p_socket_address_new ("2001:cdba:345f:24ab:fe45:5423:3257:9652", 2345);

		P_TEST_REQUIRE (addr != NULL);
		P_TEST_CHECK (p_socket_address_is_loopback (addr) == FALSE);
		P_TEST_CHECK (p_socket_address_is_any (addr) == FALSE);
		P_TEST_CHECK (p_socket_address_get_family (addr) == P_SOCKET_FAMILY_INET6);
		P_TEST_CHECK (p_socket_address_get_port (addr) == 2345);
		P_TEST_CHECK (p_socket_address_get_native_size (addr) > 0);
		P_TEST_CHECK (p_socket_address_get_flow_info (addr) == 0);
		P_TEST_CHECK (p_socket_address_get_scope_id (addr) == 0);

		addr_str = p_socket_address_get_address (addr);

		P_TEST_REQUIRE (addr_str != NULL);
		P_TEST_CHECK (strcmp (addr_str, "2001:cdba:345f:24ab:fe45:5423:3257:9652") == 0);

		p_free (addr_str);
		p_socket_address_free (addr);
	}

	/* Test IPv4 loopback address */
	addr = p_socket_address_new_loopback (P_SOCKET_FAMILY_INET, 2345);

	P_TEST_REQUIRE (addr != NULL);
	P_TEST_CHECK (p_socket_address_is_loopback (addr) == TRUE);
	P_TEST_CHECK (p_socket_address_is_any (addr) == FALSE);
	P_TEST_CHECK (p_socket_address_get_family (addr) == P_SOCKET_FAMILY_INET);
	P_TEST_CHECK (p_socket_address_get_port (addr) == 2345);
	P_TEST_CHECK (p_socket_address_get_native_size (addr) > 0);
	P_TEST_CHECK (p_socket_address_get_flow_info (addr) == 0);
	P_TEST_CHECK (p_socket_address_get_scope_id (addr) == 0);

	p_socket_address_free (addr);

	if (p_socket_address_is_ipv6_supported ()) {
		/* Test IPv6 loopback address */
		addr = p_socket_address_new_loopback (P_SOCKET_FAMILY_INET6, 2345);

		P_TEST_REQUIRE (addr != NULL);
		P_TEST_CHECK (p_socket_address_is_loopback (addr) == TRUE);
		P_TEST_CHECK (p_socket_address_is_any (addr) == FALSE);
		P_TEST_CHECK (p_socket_address_get_family (addr) == P_SOCKET_FAMILY_INET6);
		P_TEST_CHECK (p_socket_address_get_port (addr) == 2345);
		P_TEST_CHECK (p_socket_address_get_native_size (addr) > 0);
		P_TEST_CHECK (p_socket_address_get_flow_info (addr) == 0);
		P_TEST_CHECK (p_socket_address_get_scope_id (addr) == 0);

		p_socket_address_free (addr);
	}

	/* Test IPv4 any interface */
	addr = p_socket_address_new_any (P_SOCKET_FAMILY_INET, 2345);

	P_TEST_REQUIRE (addr != NULL);
	P_TEST_CHECK (p_socket_address_is_loopback (addr) == FALSE);
	P_TEST_CHECK (p_socket_address_is_any (addr) == TRUE);
	P_TEST_CHECK (p_socket_address_get_family (addr) == P_SOCKET_FAMILY_INET);
	P_TEST_CHECK (p_socket_address_get_port (addr) == 2345);
	P_TEST_CHECK (p_socket_address_get_native_size (addr) > 0);
	P_TEST_CHECK (p_socket_address_get_flow_info (addr) == 0);
	P_TEST_CHECK (p_socket_address_get_scope_id (addr) == 0);

	psize native_size = p_socket_address_get_native_size (addr);

	p_socket_address_free (addr);

	/* Test IPv4 native raw data */
	ppointer native_buf = p_malloc0 (native_size);
	P_TEST_CHECK (native_buf != NULL);
	P_TEST_CHECK (p_socket_address_new_from_native (native_buf, native_size) == NULL);
	addr = p_socket_address_new ("192.168.0.2", 2345);
	P_TEST_REQUIRE (addr != NULL);

	p_socket_address_set_flow_info (addr, 1);
	p_socket_address_set_scope_id (addr, 1);

	P_TEST_CHECK (p_socket_address_to_native (addr, native_buf, native_size) == TRUE);
	p_socket_address_free (addr);

	addr = p_socket_address_new_from_native (native_buf, native_size);

	P_TEST_CHECK (addr != NULL);
	P_TEST_CHECK (p_socket_address_is_loopback (addr) == FALSE);
	P_TEST_CHECK (p_socket_address_is_any (addr) == FALSE);
	P_TEST_CHECK (p_socket_address_get_family (addr) == P_SOCKET_FAMILY_INET);
	P_TEST_CHECK (p_socket_address_get_port (addr) == 2345);
	P_TEST_CHECK (p_socket_address_get_native_size (addr) == native_size);
	P_TEST_CHECK (p_socket_address_get_flow_info (addr) == 0);
	P_TEST_CHECK (p_socket_address_get_scope_id (addr) == 0);

	addr_str = p_socket_address_get_address (addr);

	P_TEST_REQUIRE (addr_str != NULL);
	P_TEST_CHECK (strcmp (addr_str, "192.168.0.2") == 0);

	p_free (native_buf);
	p_free (addr_str);
	p_socket_address_free (addr);

	if (p_socket_address_is_ipv6_supported ()) {
		/* Test IPv6 any interface */
		addr = p_socket_address_new_any (P_SOCKET_FAMILY_INET6, 2345);

		P_TEST_REQUIRE (addr != NULL);
		P_TEST_CHECK (p_socket_address_is_loopback (addr) == FALSE);
		P_TEST_CHECK (p_socket_address_is_any (addr) == TRUE);
		P_TEST_CHECK (p_socket_address_get_family (addr) == P_SOCKET_FAMILY_INET6);
		P_TEST_CHECK (p_socket_address_get_port (addr) == 2345);
		P_TEST_CHECK (p_socket_address_get_native_size (addr) > 0);
		P_TEST_CHECK (p_socket_address_get_flow_info (addr) == 0);
		P_TEST_CHECK (p_socket_address_get_scope_id (addr) == 0);

		native_size = p_socket_address_get_native_size (addr);

		p_socket_address_free (addr);

		/* Test IPv6 native raw data */
		native_buf = p_malloc0 (native_size);
		P_TEST_CHECK (native_buf != NULL);
		P_TEST_CHECK (p_socket_address_new_from_native (native_buf, native_size) == NULL);
		addr = p_socket_address_new ("2001:cdba:345f:24ab:fe45:5423:3257:9652", 2345);
		P_TEST_REQUIRE (addr != NULL);

		p_socket_address_set_flow_info (addr, 1);
		p_socket_address_set_scope_id (addr, 1);

		P_TEST_CHECK (p_socket_address_to_native (addr, native_buf, native_size) == TRUE);
		p_socket_address_free (addr);

		addr = p_socket_address_new_from_native (native_buf, native_size);

		P_TEST_CHECK (addr != NULL);
		P_TEST_CHECK (p_socket_address_is_loopback (addr) == FALSE);
		P_TEST_CHECK (p_socket_address_is_any (addr) == FALSE);
		P_TEST_CHECK (p_socket_address_get_family (addr) == P_SOCKET_FAMILY_INET6);
		P_TEST_CHECK (p_socket_address_get_port (addr) == 2345);
		P_TEST_CHECK (p_socket_address_get_native_size (addr) == native_size);

		if (p_socket_address_is_flow_info_supported ())
			P_TEST_CHECK (p_socket_address_get_flow_info (addr) == 1);

		if (p_socket_address_is_scope_id_supported ())
			P_TEST_CHECK (p_socket_address_get_scope_id (addr) == 1);

		addr_str = p_socket_address_get_address (addr);

		P_TEST_REQUIRE (addr_str != NULL);
		P_TEST_CHECK (strcmp (addr_str, "2001:cdba:345f:24ab:fe45:5423:3257:9652") == 0);

		p_free (native_buf);
		p_free (addr_str);
		p_socket_address_free (addr);
	}

	if (p_socket_address_is_flow_info_supported () || p_socket_address_is_scope_id_supported ())
		P_TEST_CHECK (p_socket_address_is_ipv6_supported () == TRUE);

	p_libsys_shutdown ();
}
P_TEST_CASE_END ()

P_TEST_SUITE_BEGIN()
{
	P_TEST_SUITE_RUN_CASE (psocketaddress_nomem_test);
	P_TEST_SUITE_RUN_CASE (psocketaddress_bad_input_test);
	P_TEST_SUITE_RUN_CASE (psocketaddress_general_test);
}
P_TEST_SUITE_END()
