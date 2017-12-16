/*
 * Copyright (C) 2017 Jean-Damien Durand <jeandamiendurand@gmail.com>
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, see <http://www.gnu.org/licenses/>.
 */

#include "plibsys.h"
#include "ptestmacros.h"

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

P_TEST_MODULE_INIT ();

static void f (int i, ...) {
  p_va_list args1, args2;

  p_va_start (args1, i);
  p_va_copy(args2, args1);

  P_TEST_CHECK (va_arg (args1, int) == 42);
  P_TEST_CHECK (va_arg (args2, int) == 42);

  p_va_end (args1);
  p_va_end (args2);
}

P_TEST_CASE_BEGIN (pstdarg_general_test)
{
	p_libsys_init ();

        f(0 ,42);

	p_libsys_shutdown ();
}
P_TEST_CASE_END ()

P_TEST_SUITE_BEGIN()
{
	P_TEST_SUITE_RUN_CASE (pstdarg_general_test);
}
P_TEST_SUITE_END()
