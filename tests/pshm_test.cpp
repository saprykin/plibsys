#define BOOST_TEST_MODULE pshm_test

#include "plib.h"

#include <boost/test/included/unit_test.hpp>

BOOST_AUTO_TEST_SUITE (BOOST_TEST_MODULE)

BOOST_AUTO_TEST_CASE (pshm_general_test)
{
	PShm		*shm, *shm2;
	ppointer	addr, addr2;
	int		i;

	shm = p_shm_new ("p_shm_test_memory_block", 1024, P_SHM_ACCESS_READWRITE);
	BOOST_REQUIRE (shm != NULL);
	BOOST_REQUIRE (p_shm_get_size (shm) == 1024);

	addr = p_shm_get_address (shm);
	BOOST_REQUIRE (addr != NULL);

	shm2 = p_shm_new ("p_shm_test_memory_block", 1024, P_SHM_ACCESS_READONLY);
	BOOST_REQUIRE (shm2 != NULL);
	BOOST_REQUIRE (p_shm_get_size (shm2) == 1024);

	addr2 = p_shm_get_address (shm2);
	BOOST_REQUIRE (shm2 != NULL);

	for (i = 0; i < 512; ++i) {
		BOOST_CHECK (p_shm_lock (shm));
		*(((pchar *) addr) + i) = 'a';
		BOOST_CHECK (p_shm_unlock (shm));
	}

	for (i = 0; i < 512; ++i) {
		BOOST_CHECK (p_shm_lock (shm2));
		BOOST_CHECK (*(((pchar *) addr) + i) == 'a');
		BOOST_CHECK (p_shm_unlock (shm2));
	}

	for (i = 0; i < 1024; ++i) {
		BOOST_CHECK (p_shm_lock (shm));
		*(((pchar *) addr) + i) = 'b';
		BOOST_CHECK (p_shm_unlock (shm));
	}

	for (i = 0; i < 1024; ++i) {
		BOOST_CHECK (p_shm_lock (shm2));
		BOOST_CHECK (*(((pchar *) addr) + i) != 'c');
		BOOST_CHECK (p_shm_unlock (shm2));
	}

	for (i = 0; i < 1024; ++i) {
		BOOST_CHECK (p_shm_lock (shm2));
		BOOST_CHECK (*(((pchar *) addr) + i) == 'b');
		BOOST_CHECK (p_shm_unlock (shm2));
	}

	p_shm_free (shm);

	shm = p_shm_new ("p_shm_test_memory_block_2", 1024, P_SHM_ACCESS_READWRITE);
	BOOST_REQUIRE (shm != NULL);
	BOOST_REQUIRE (p_shm_get_size (shm) == 1024);

	addr = p_shm_get_address (shm);
	BOOST_REQUIRE (addr != NULL);

	for (i = 0; i < 1024; ++i) {
		BOOST_CHECK (p_shm_lock (shm));
		BOOST_CHECK (*(((pchar *) addr) + i) != 'b');
		BOOST_CHECK (p_shm_unlock (shm));
	}

	p_shm_free (shm);
	p_shm_free (shm2);
}

BOOST_AUTO_TEST_SUITE_END()
