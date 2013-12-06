#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE pshm_test

#include "plib.h"

#include <stdlib.h>
#include <time.h>

#include <boost/test/unit_test.hpp>

static void * shm_test_thread (void *arg)
{
	pint		rand_num;
	psize		shm_size;
	ppointer	addr;
	PShm		*shm;

	if (arg == NULL)
		p_uthread_exit (1);

	shm = (PShm *) arg;
	rand_num = rand () % 127;
	shm_size = p_shm_get_size (shm);
	addr = p_shm_get_address (shm);

	if (shm_size == 0 || addr == NULL)
		p_uthread_exit (1);

	if (!p_shm_lock (shm))
		p_uthread_exit (1);

	for (puint i = 0; i < shm_size; ++i)
		*(((pchar *) addr) + i) = (pchar) rand_num;

	if (!p_shm_unlock (shm))
		p_uthread_exit (1);

	p_uthread_exit (0);
}

BOOST_AUTO_TEST_SUITE (BOOST_TEST_MODULE)

BOOST_AUTO_TEST_CASE (pshm_general_test)
{
	PShm		*shm = NULL, *shm2 = NULL;
	ppointer	addr, addr2;
	pint		i;

	p_lib_init ();

	BOOST_REQUIRE (p_shm_get_address (shm) == NULL);
	BOOST_REQUIRE (p_shm_get_size (shm) == 0);
	p_shm_take_ownership (shm);

	shm = p_shm_new ("p_shm_test_memory_block", 1024, P_SHM_ACCESS_READWRITE);
	BOOST_REQUIRE (shm != NULL);
	BOOST_REQUIRE (p_shm_get_size (shm) == 1024);

	addr = p_shm_get_address (shm);
	BOOST_REQUIRE (addr != NULL);

	shm2 = p_shm_new ("p_shm_test_memory_block", 1024, P_SHM_ACCESS_READONLY);

	if (shm2 == NULL) {
		/* OK, some systems may want exactly the same permissions */
		shm2 = p_shm_new ("p_shm_test_memory_block", 1024, P_SHM_ACCESS_READWRITE);
	}

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

	p_lib_shutdown ();
}

BOOST_AUTO_TEST_CASE (pshm_thread_test)
{
	PShm		*shm;
	PUThread	*thr1, *thr2, *thr3;
	ppointer	addr;
	pint		i, val;
	pboolean	test_ok;

	p_lib_init ();

	srand ((puint) time (NULL));

	shm = p_shm_new ("p_shm_test_memory_block", 1024 * 1024, P_SHM_ACCESS_READWRITE);
	BOOST_REQUIRE (shm != NULL);
	p_shm_take_ownership (shm);

	if (p_shm_get_size (shm) != 1024 * 1024) {
		p_shm_free (shm);
		shm = p_shm_new ("p_shm_test_memory_block", 1024 * 1024, P_SHM_ACCESS_READWRITE);
		BOOST_REQUIRE (shm != NULL);
	}

	BOOST_REQUIRE (p_shm_get_size (shm) == 1024 * 1024);

	addr = p_shm_get_address (shm);
	BOOST_REQUIRE (addr != NULL);

	thr1 = p_uthread_create ((PUThreadFunc) shm_test_thread, (ppointer) shm, true);
	BOOST_REQUIRE (thr1 != NULL);

	thr2 = p_uthread_create ((PUThreadFunc) shm_test_thread, (ppointer) shm, true);
	BOOST_REQUIRE (thr2 != NULL);

	thr3 = p_uthread_create ((PUThreadFunc) shm_test_thread, (ppointer) shm, true);
	BOOST_REQUIRE (thr3 != NULL);

	BOOST_CHECK (p_uthread_join (thr1) == 0);
	BOOST_CHECK (p_uthread_join (thr2) == 0);
	BOOST_CHECK (p_uthread_join (thr3) == 0);

	test_ok = TRUE;
	val = *((pchar *) addr);

	for (i = 1; i < 1024 * 1024; ++i)
		if (*(((pchar *) addr) + i) != val) {
			test_ok = FALSE;
			break;
		}

	BOOST_REQUIRE (test_ok == TRUE);

	p_uthread_free (thr1);
	p_uthread_free (thr2);
	p_uthread_free (thr3);
	p_shm_free (shm);

	p_lib_shutdown ();
}

BOOST_AUTO_TEST_SUITE_END()
