#define BOOST_TEST_MODULE psemaphore_test

#include "plib.h"

#include <boost/test/included/unit_test.hpp>

static int semaphore_test_val = 10;

void * semaphore_test_thread (void *)
{
	PSemaphore	*sem;
	int		i;

	sem = p_semaphore_new ("p_semaphore_test_object", 1, P_SEM_ACCESS_OPEN);

	if (sem == NULL)
		p_uthread_exit (1);

	for (i = 0; i < 1000; ++i) {
		if (!p_semaphore_acquire (sem))
			p_uthread_exit (1);

		if (semaphore_test_val == 10)
			--semaphore_test_val;
		else
			++semaphore_test_val;

		p_uthread_sleep (1);

		if (!p_semaphore_release (sem))
			p_uthread_exit (1);
	}

	p_semaphore_free (sem);
	p_uthread_exit (0);
}

BOOST_AUTO_TEST_SUITE (BOOST_TEST_MODULE)

BOOST_AUTO_TEST_CASE (psemaphore_general_test)
{
	PSemaphore	*sem;
	int		i;

	sem = p_semaphore_new ("p_semaphore_test_object", 10, P_SEM_ACCESS_CREATE);
	BOOST_REQUIRE (sem != NULL);

	for (i = 0; i < 10; ++i)
		BOOST_CHECK (p_semaphore_acquire (sem));

	for (i = 0; i < 10; ++i)
		BOOST_CHECK (p_semaphore_release (sem));

	for (i = 0; i < 1000; ++i) {
		BOOST_CHECK (p_semaphore_acquire (sem));
		BOOST_CHECK (p_semaphore_release (sem));
	}

	p_semaphore_free (sem);
}

BOOST_AUTO_TEST_CASE (psemaphore_thread_test)
{
	PUThread	*thr1, *thr2;

	thr1 = p_uthread_create ((PUThreadFunc) semaphore_test_thread, NULL, true);
	BOOST_REQUIRE (thr1 != NULL);

	thr2 = p_uthread_create ((PUThreadFunc) semaphore_test_thread, NULL, true);
	BOOST_REQUIRE (thr2 != NULL);

	BOOST_CHECK (p_uthread_join (thr1) == 0);
	BOOST_CHECK (p_uthread_join (thr2) == 0);

	BOOST_REQUIRE (semaphore_test_val == 10);

	p_uthread_free (thr1);
	p_uthread_free (thr2);
}

BOOST_AUTO_TEST_SUITE_END()
