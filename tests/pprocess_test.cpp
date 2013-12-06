#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE pprocess_test

#include "plib.h"

#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE (BOOST_TEST_MODULE)

BOOST_AUTO_TEST_CASE (pprocess_general_test)
{
	puint32	pid;

	p_lib_init ();

	pid = p_process_get_current_pid ();
	BOOST_CHECK (pid > 0);
	BOOST_REQUIRE (p_process_is_running (pid) == TRUE);

	p_lib_shutdown ();
}

BOOST_AUTO_TEST_SUITE_END()
