#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE pshmbuffer_test

#include "plib.h"

#include <string.h>

#include <boost/test/unit_test.hpp>

volatile static pboolean is_working = TRUE;
static pchar test_str[] = "This is a test string!";

static void * shm_buffer_test_write_thread (void *arg)
{
	P_UNUSED (arg);

	PShmBuffer *buffer = p_shm_buffer_new ("pshm_test_buffer", 1024);

	if (buffer == NULL)
		p_uthread_exit (1);

	while (is_working == TRUE) {
		p_uthread_sleep (3);

		if (p_shm_buffer_get_free_space (buffer) < sizeof (test_str))
			continue;

		if (p_shm_buffer_write (buffer, (ppointer) test_str, sizeof (test_str)) != sizeof (test_str)) {
			p_shm_buffer_free (buffer);
			p_uthread_exit (1);
		}
	}

	p_shm_buffer_free (buffer);
	p_uthread_exit (0);
}

static void * shm_buffer_test_read_thread (void *arg)
{
	P_UNUSED (arg);

	PShmBuffer	*buffer = p_shm_buffer_new ("pshm_test_buffer", 1024);
	pchar		test_buf[sizeof (test_str)];

	if (buffer == NULL)
		p_uthread_exit (1);

	while (is_working == TRUE) {
		p_uthread_sleep (3);

		if (p_shm_buffer_get_used_space (buffer) < sizeof (test_str))
			continue;

		if (p_shm_buffer_read (buffer, (ppointer) test_buf, sizeof (test_buf)) != sizeof (test_buf)) {
			p_shm_buffer_free (buffer);
			p_uthread_exit (1);
		}

		if (strncmp (test_buf, test_str, sizeof (test_buf)) != 0) {
			p_shm_buffer_free (buffer);
			p_uthread_exit (1);
		}
	}

	p_shm_buffer_free (buffer);
	p_uthread_exit (0);
}

BOOST_AUTO_TEST_SUITE (BOOST_TEST_MODULE)

BOOST_AUTO_TEST_CASE (pshmbuffer_bad_input_test)
{
	p_lib_init ();

	BOOST_CHECK (p_shm_buffer_new (NULL, 0) == NULL);
	BOOST_CHECK (p_shm_buffer_read (NULL, NULL, 0) == -1);
	BOOST_CHECK (p_shm_buffer_read (NULL, NULL, 0) == -1);
	BOOST_CHECK (p_shm_buffer_get_free_space (NULL) == -1);
	BOOST_CHECK (p_shm_buffer_get_used_space (NULL) == -1);

	p_shm_buffer_clear (NULL);
	p_shm_buffer_free (NULL);

	p_lib_shutdown ();
}

BOOST_AUTO_TEST_CASE (pshmbuffer_general_test)
{
	p_lib_init ();

	pchar		test_buf[sizeof (test_str)];
	PShmBuffer	*buffer = NULL;

	/* Buffer may be from the previous test on UNIX systems */
	buffer = p_shm_buffer_new ("pshm_test_buffer", 1024);
	BOOST_REQUIRE (buffer != NULL);
	p_shm_buffer_take_ownership (buffer);
	p_shm_buffer_free (buffer);
	buffer = p_shm_buffer_new ("pshm_test_buffer", 1024);
	BOOST_REQUIRE (buffer != NULL);

	BOOST_CHECK (p_shm_buffer_get_free_space (buffer) == 1024);
	BOOST_CHECK (p_shm_buffer_get_used_space (buffer) == 0);
	p_shm_buffer_clear (buffer);
	BOOST_CHECK (p_shm_buffer_get_free_space (buffer) == 1024);
	BOOST_CHECK (p_shm_buffer_get_used_space (buffer) == 0);

	memset (test_buf, 0, sizeof (test_buf));

	BOOST_CHECK (p_shm_buffer_write (buffer, (ppointer) test_str, sizeof (test_str)) == sizeof (test_str));
	BOOST_CHECK (p_shm_buffer_get_free_space (buffer) == (1024 - sizeof (test_str)));
	BOOST_CHECK (p_shm_buffer_get_used_space (buffer) == sizeof (test_str));
	BOOST_CHECK (p_shm_buffer_read (buffer, (ppointer) test_buf, sizeof (test_buf)) == sizeof (test_str));

	BOOST_CHECK (strncmp (test_buf, test_str, sizeof (test_str)) == 0);
	BOOST_CHECK (p_shm_buffer_get_free_space (buffer) == 1024);
	BOOST_CHECK (p_shm_buffer_get_used_space (buffer) == 0);

	p_shm_buffer_clear (buffer);
	p_shm_buffer_free (buffer);

	p_lib_shutdown ();
}

BOOST_AUTO_TEST_CASE (pshmbuffer_thread_test)
{
	p_lib_init ();

	PShmBuffer *	buffer = NULL;
	PUThread	*thr1, *thr2;

	/* Buffer may be from the previous test on UNIX systems */
	buffer = p_shm_buffer_new ("pshm_test_buffer", 1024);
	BOOST_REQUIRE (buffer != NULL);
	p_shm_buffer_take_ownership (buffer);
	p_shm_buffer_free (buffer);

	thr1 = p_uthread_create ((PUThreadFunc) shm_buffer_test_write_thread, NULL, TRUE);
	BOOST_REQUIRE (thr1 != NULL);

	thr2 = p_uthread_create ((PUThreadFunc) shm_buffer_test_read_thread, NULL, TRUE);
	BOOST_REQUIRE (thr1 != NULL);

	p_uthread_sleep (5000);

	is_working = FALSE;

	BOOST_CHECK (p_uthread_join (thr1) == 0);
	BOOST_CHECK (p_uthread_join (thr2) == 0);

	p_uthread_free (thr1);
	p_uthread_free (thr2);

	p_lib_shutdown ();
}

BOOST_AUTO_TEST_SUITE_END()
