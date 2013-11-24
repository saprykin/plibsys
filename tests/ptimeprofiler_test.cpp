#define BOOST_TEST_MODULE ptimeprofiler_test

#include "plib.h"

#include <string.h>

#include <boost/test/included/unit_test.hpp>

BOOST_AUTO_TEST_SUITE (BOOST_TEST_MODULE)

BOOST_AUTO_TEST_CASE (ptimeprofiler_general_test)
{
	PTimeProfiler	*profiler = NULL;
	pint64		prev_val, val;

	BOOST_CHECK (p_time_profiler_elapsed_usecs (profiler) == 0);
	p_time_profiler_reset (profiler);
	p_time_profiler_free (profiler);

	profiler = p_time_profiler_new ();
	BOOST_REQUIRE (profiler != NULL);

	p_uthread_sleep (10);
	prev_val = p_time_profiler_elapsed_usecs (profiler);
	BOOST_CHECK (prev_val > 0);

	p_uthread_sleep (100);
	val = p_time_profiler_elapsed_usecs (profiler);
	BOOST_CHECK (val > prev_val);
	prev_val = val;

	p_uthread_sleep (1000);
	val = p_time_profiler_elapsed_usecs (profiler);
	BOOST_CHECK (val > prev_val);

	p_time_profiler_reset (profiler);

	p_uthread_sleep (15);
	prev_val = p_time_profiler_elapsed_usecs (profiler);
	BOOST_CHECK (prev_val > 0);

	p_uthread_sleep (178);
	val = p_time_profiler_elapsed_usecs (profiler);
	BOOST_CHECK (val > prev_val);

	p_time_profiler_free (profiler);
}

BOOST_AUTO_TEST_SUITE_END()
