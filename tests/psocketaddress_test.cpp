#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE psocketaddress_test

#include "plib.h"

#include <string.h>

#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE (BOOST_TEST_MODULE)

BOOST_AUTO_TEST_CASE (psocketaddress_bad_input_test)
{
	p_lib_init ();

	BOOST_CHECK (p_socket_address_new_from_native (NULL, 0) == NULL);
	BOOST_CHECK (p_socket_address_new (NULL, 0) == NULL);
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

	/* Test LAN address */
	PSocketAddress *addr = p_socket_address_new ("192.168.0.1", 2345);

	BOOST_REQUIRE (addr != NULL);
	BOOST_CHECK (p_socket_address_is_loopback (addr) == FALSE);
	BOOST_CHECK (p_socket_address_is_any (addr) == FALSE);
	BOOST_CHECK (p_socket_address_get_family (addr) != P_SOCKET_FAMILY_UNKNOWN);
	BOOST_CHECK (p_socket_address_get_port (addr) == 2345);
	BOOST_CHECK (p_socket_address_get_native_size (addr) > 0);

	pchar *addr_str = p_socket_address_get_address (addr);

	BOOST_REQUIRE (addr_str != NULL);
	BOOST_CHECK (strcmp (addr_str, "192.168.0.1") == 0);

	p_free (addr_str);
	p_socket_address_free (addr);

	/* Test loopback address */
	addr = p_socket_address_new_loopback (P_SOCKET_FAMILY_INET, 2345);

	BOOST_REQUIRE (addr != NULL);
	BOOST_CHECK (p_socket_address_is_loopback (addr) == TRUE);
	BOOST_CHECK (p_socket_address_is_any (addr) == FALSE);
	BOOST_CHECK (p_socket_address_get_family (addr) == P_SOCKET_FAMILY_INET);
	BOOST_CHECK (p_socket_address_get_port (addr) == 2345);
	BOOST_CHECK (p_socket_address_get_native_size (addr) > 0);

	p_socket_address_free (addr);

	/* Test any interface */
	addr = p_socket_address_new_any (P_SOCKET_FAMILY_INET, 2345);

	BOOST_REQUIRE (addr != NULL);
	BOOST_CHECK (p_socket_address_is_loopback (addr) == FALSE);
	BOOST_CHECK (p_socket_address_is_any (addr) == TRUE);
	BOOST_CHECK (p_socket_address_get_family (addr) == P_SOCKET_FAMILY_INET);
	BOOST_CHECK (p_socket_address_get_port (addr) == 2345);
	BOOST_CHECK (p_socket_address_get_native_size (addr) > 0);

	psize native_size = p_socket_address_get_native_size (addr);

	p_socket_address_free (addr);

	/* Test native raw data */
	ppointer native_buf = p_malloc0 (native_size);
	BOOST_CHECK (native_buf != NULL);
	addr = p_socket_address_new ("192.168.0.2", 2345);
	BOOST_REQUIRE (addr != NULL);
	BOOST_CHECK (p_socket_address_to_native (addr, native_buf, native_size) == TRUE);
	p_socket_address_free (addr);

	addr = p_socket_address_new_from_native (native_buf, native_size);

	BOOST_CHECK (addr != NULL);
	BOOST_CHECK (p_socket_address_is_loopback (addr) == FALSE);
	BOOST_CHECK (p_socket_address_is_any (addr) == FALSE);
	BOOST_CHECK (p_socket_address_get_family (addr) != P_SOCKET_FAMILY_UNKNOWN);
	BOOST_CHECK (p_socket_address_get_port (addr) == 2345);
	BOOST_CHECK (p_socket_address_get_native_size (addr) == native_size);

	addr_str = p_socket_address_get_address (addr);

	BOOST_REQUIRE (addr_str != NULL);
	BOOST_CHECK (strcmp (addr_str, "192.168.0.2") == 0);

	p_free (addr_str);
	p_socket_address_free (addr);

	p_lib_shutdown ();
}

BOOST_AUTO_TEST_SUITE_END()
