/*
 * Copyright (C) 2013-2016 Alexander Saprykin <xelfium@gmail.com>
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

#ifndef PLIB_TESTS_STATIC
#  define BOOST_TEST_DYN_LINK
#endif

#define BOOST_TEST_MODULE psocketaddress_test

#include "plib.h"

#include <string.h>

#ifdef PLIB_TESTS_STATIC
#  include <boost/test/included/unit_test.hpp>
#else
#  include <boost/test/unit_test.hpp>
#endif

BOOST_AUTO_TEST_SUITE (BOOST_TEST_MODULE)

BOOST_AUTO_TEST_CASE (psocketaddress_bad_input_test)
{
	p_lib_init ();

	BOOST_CHECK (p_socket_address_new_from_native (NULL, 0) == NULL);
	BOOST_CHECK (p_socket_address_new (NULL, 0) == NULL);
	BOOST_CHECK (p_socket_address_new ("bad_address", 0) == NULL);
	BOOST_CHECK (p_socket_address_new_any (P_SOCKET_FAMILY_UNKNOWN, 0) == NULL);
	BOOST_CHECK (p_socket_address_new_loopback (P_SOCKET_FAMILY_UNKNOWN, 0) == NULL);
	BOOST_CHECK (p_socket_address_to_native (NULL, NULL, 0) == FALSE);
	BOOST_CHECK (p_socket_address_get_native_size (NULL) == 0);
	BOOST_CHECK (p_socket_address_get_family (NULL) == P_SOCKET_FAMILY_UNKNOWN);
	BOOST_CHECK (p_socket_address_get_address (NULL) == NULL);
	BOOST_CHECK (p_socket_address_get_port (NULL) == 0);
	BOOST_CHECK (p_socket_address_is_any (NULL) == FALSE);
	BOOST_CHECK (p_socket_address_is_loopback (NULL) == FALSE);

	p_socket_address_free (NULL);

	p_lib_shutdown ();
}

BOOST_AUTO_TEST_CASE (psocketaddress_general_test)
{
	p_lib_init ();

	/* Test IPv4 LAN address */
	PSocketAddress *addr = p_socket_address_new ("192.168.0.1", 2345);

	BOOST_REQUIRE (addr != NULL);
	BOOST_CHECK (p_socket_address_is_loopback (addr) == FALSE);
	BOOST_CHECK (p_socket_address_is_any (addr) == FALSE);
	BOOST_CHECK (p_socket_address_get_family (addr) == P_SOCKET_FAMILY_INET);
	BOOST_CHECK (p_socket_address_get_port (addr) == 2345);
	BOOST_CHECK (p_socket_address_get_native_size (addr) > 0);

	pchar *addr_str = p_socket_address_get_address (addr);

	BOOST_REQUIRE (addr_str != NULL);
	BOOST_CHECK (strcmp (addr_str, "192.168.0.1") == 0);

	p_free (addr_str);
	p_socket_address_free (addr);

#ifdef AF_INET6
	/* Test IPv6 LAN address */
	addr = p_socket_address_new ("2001:cdba:345f:24ab:fe45:5423:3257:9652", 2345);

	BOOST_REQUIRE (addr != NULL);
	BOOST_CHECK (p_socket_address_is_loopback (addr) == FALSE);
	BOOST_CHECK (p_socket_address_is_any (addr) == FALSE);
	BOOST_CHECK (p_socket_address_get_family (addr) == P_SOCKET_FAMILY_INET6);
	BOOST_CHECK (p_socket_address_get_port (addr) == 2345);
	BOOST_CHECK (p_socket_address_get_native_size (addr) > 0);

	addr_str = p_socket_address_get_address (addr);

	BOOST_REQUIRE (addr_str != NULL);
	BOOST_CHECK (strcmp (addr_str, "2001:cdba:345f:24ab:fe45:5423:3257:9652") == 0);

	p_free (addr_str);
	p_socket_address_free (addr);
#endif

	/* Test IPv4 loopback address */
	addr = p_socket_address_new_loopback (P_SOCKET_FAMILY_INET, 2345);

	BOOST_REQUIRE (addr != NULL);
	BOOST_CHECK (p_socket_address_is_loopback (addr) == TRUE);
	BOOST_CHECK (p_socket_address_is_any (addr) == FALSE);
	BOOST_CHECK (p_socket_address_get_family (addr) == P_SOCKET_FAMILY_INET);
	BOOST_CHECK (p_socket_address_get_port (addr) == 2345);
	BOOST_CHECK (p_socket_address_get_native_size (addr) > 0);

	p_socket_address_free (addr);

#ifdef AF_INET6
	/* Test IPv6 loopback address */
	addr = p_socket_address_new_loopback (P_SOCKET_FAMILY_INET6, 2345);

	BOOST_REQUIRE (addr != NULL);
	BOOST_CHECK (p_socket_address_is_loopback (addr) == TRUE);
	BOOST_CHECK (p_socket_address_is_any (addr) == FALSE);
	BOOST_CHECK (p_socket_address_get_family (addr) == P_SOCKET_FAMILY_INET6);
	BOOST_CHECK (p_socket_address_get_port (addr) == 2345);
	BOOST_CHECK (p_socket_address_get_native_size (addr) > 0);

	p_socket_address_free (addr);
#endif

	/* Test IPv4 any interface */
	addr = p_socket_address_new_any (P_SOCKET_FAMILY_INET, 2345);

	BOOST_REQUIRE (addr != NULL);
	BOOST_CHECK (p_socket_address_is_loopback (addr) == FALSE);
	BOOST_CHECK (p_socket_address_is_any (addr) == TRUE);
	BOOST_CHECK (p_socket_address_get_family (addr) == P_SOCKET_FAMILY_INET);
	BOOST_CHECK (p_socket_address_get_port (addr) == 2345);
	BOOST_CHECK (p_socket_address_get_native_size (addr) > 0);

	psize native_size = p_socket_address_get_native_size (addr);

	p_socket_address_free (addr);

	/* Test IPv4 native raw data */
	ppointer native_buf = p_malloc0 (native_size);
	BOOST_CHECK (native_buf != NULL);
	BOOST_CHECK (p_socket_address_new_from_native (native_buf, native_size) == NULL);
	addr = p_socket_address_new ("192.168.0.2", 2345);
	BOOST_REQUIRE (addr != NULL);
	BOOST_CHECK (p_socket_address_to_native (addr, native_buf, native_size) == TRUE);
	p_socket_address_free (addr);

	addr = p_socket_address_new_from_native (native_buf, native_size);

	BOOST_CHECK (addr != NULL);
	BOOST_CHECK (p_socket_address_is_loopback (addr) == FALSE);
	BOOST_CHECK (p_socket_address_is_any (addr) == FALSE);
	BOOST_CHECK (p_socket_address_get_family (addr) == P_SOCKET_FAMILY_INET);
	BOOST_CHECK (p_socket_address_get_port (addr) == 2345);
	BOOST_CHECK (p_socket_address_get_native_size (addr) == native_size);

	addr_str = p_socket_address_get_address (addr);

	BOOST_REQUIRE (addr_str != NULL);
	BOOST_CHECK (strcmp (addr_str, "192.168.0.2") == 0);

	p_free (native_buf);
	p_free (addr_str);
	p_socket_address_free (addr);

#ifdef AF_INET6
	/* Test IPv6 any interface */
	addr = p_socket_address_new_any (P_SOCKET_FAMILY_INET6, 2345);

	BOOST_REQUIRE (addr != NULL);
	BOOST_CHECK (p_socket_address_is_loopback (addr) == FALSE);
	BOOST_CHECK (p_socket_address_is_any (addr) == TRUE);
	BOOST_CHECK (p_socket_address_get_family (addr) == P_SOCKET_FAMILY_INET6);
	BOOST_CHECK (p_socket_address_get_port (addr) == 2345);
	BOOST_CHECK (p_socket_address_get_native_size (addr) > 0);

	native_size = p_socket_address_get_native_size (addr);

	p_socket_address_free (addr);

	/* Test IPv6 native raw data */
	native_buf = p_malloc0 (native_size);
	BOOST_CHECK (native_buf != NULL);
	BOOST_CHECK (p_socket_address_new_from_native (native_buf, native_size) == NULL);
	addr = p_socket_address_new ("2001:cdba:345f:24ab:fe45:5423:3257:9652", 2345);
	BOOST_REQUIRE (addr != NULL);
	BOOST_CHECK (p_socket_address_to_native (addr, native_buf, native_size) == TRUE);
	p_socket_address_free (addr);

	addr = p_socket_address_new_from_native (native_buf, native_size);

	BOOST_CHECK (addr != NULL);
	BOOST_CHECK (p_socket_address_is_loopback (addr) == FALSE);
	BOOST_CHECK (p_socket_address_is_any (addr) == FALSE);
	BOOST_CHECK (p_socket_address_get_family (addr) == P_SOCKET_FAMILY_INET6);
	BOOST_CHECK (p_socket_address_get_port (addr) == 2345);
	BOOST_CHECK (p_socket_address_get_native_size (addr) == native_size);

	addr_str = p_socket_address_get_address (addr);

	BOOST_REQUIRE (addr_str != NULL);
	BOOST_CHECK (strcmp (addr_str, "2001:cdba:345f:24ab:fe45:5423:3257:9652") == 0);

	p_free (native_buf);
	p_free (addr_str);
	p_socket_address_free (addr);
#endif

	p_lib_shutdown ();
}

BOOST_AUTO_TEST_SUITE_END()
