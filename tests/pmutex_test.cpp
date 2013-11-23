#define BOOST_TEST_MODULE pmutex_test

#include "plib.h"

#include <boost/test/included/unit_test.hpp>

static pint mutex_test_val = 10;
static PMutex *global_mutex = NULL;

static void * mutex_test_thread (void *)
{
	pint	i;

	for (i = 0; i < 1000; ++i) {
		if (!p_mutex_lock (global_mutex))
			p_uthread_exit (1);

		if (mutex_test_val == 10)
			--mutex_test_val;
		else {
			p_uthread_sleep (1);
			++mutex_test_val;
		}

		if (!p_mutex_unlock (global_mutex))
			p_uthread_exit (1);
	}

	p_uthread_exit (0);
}

BOOST_AUTO_TEST_SUITE (BOOST_TEST_MODULE)

BOOST_AUTO_TEST_CASE (pmutex_general_test)
{
	PUThread	*thr1, *thr2;

	BOOST_REQUIRE (p_mutex_lock (global_mutex) == FALSE);
	BOOST_REQUIRE (p_mutex_unlock (global_mutex) == FALSE);
	BOOST_REQUIRE (p_mutex_trylock (global_mutex) == FALSE);

	mutex_test_val = 10;
	global_mutex = p_mutex_new ();
	BOOST_REQUIRE (global_mutex != NULL);

	thr1 = p_uthread_create ((PUThreadFunc) mutex_test_thread, NULL, true);
	BOOST_REQUIRE (thr1 != NULL);

	thr2 = p_uthread_create ((PUThreadFunc) mutex_test_thread, NULL, true);
	BOOST_REQUIRE (thr2 != NULL);

	BOOST_CHECK (p_uthread_join (thr1) == 0);
	BOOST_CHECK (p_uthread_join (thr2) == 0);

	BOOST_REQUIRE (mutex_test_val == 10);

	p_uthread_free (thr1);
	p_uthread_free (thr2);
	p_mutex_free (global_mutex);
}

BOOST_AUTO_TEST_SUITE_END()
