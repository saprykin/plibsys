#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE pmain_test

#include "plib.h"

#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE (BOOST_TEST_MODULE)

BOOST_AUTO_TEST_CASE (pmain_general_test)
{
	p_lib_init ();
	p_lib_shutdown ();
}

BOOST_AUTO_TEST_SUITE_END()
