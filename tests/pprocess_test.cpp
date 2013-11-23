#define BOOST_TEST_MODULE pprocess_test

#include "plib.h"

#include <string.h>

#include <boost/test/included/unit_test.hpp>

BOOST_AUTO_TEST_SUITE (BOOST_TEST_MODULE)

BOOST_AUTO_TEST_CASE (pprocess_general_test)
{
	puint32	pid;

	pid = p_process_get_current_pid ();
	BOOST_CHECK (pid > 0);
	BOOST_REQUIRE (p_process_is_running (pid) == TRUE);
}

BOOST_AUTO_TEST_SUITE_END()
