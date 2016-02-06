/*
 * Copyright (C) 2013-2016 Alexander Saprykin <xelfium@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA.
 */

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE ptimeprofiler_test

#include "plib.h"

#ifdef PLIB_TESTS_STATIC
#  include <boost/test/included/unit_test.hpp>
#else
#  include <boost/test/unit_test.hpp>
#endif

BOOST_AUTO_TEST_SUITE (BOOST_TEST_MODULE)

BOOST_AUTO_TEST_CASE (ptimeprofiler_general_test)
{
	PTimeProfiler	*profiler = NULL;
	pint64		prev_val, val;

	p_lib_init ();

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

	p_lib_shutdown ();
}

BOOST_AUTO_TEST_SUITE_END()
