/*
 * The MIT License
 *
 * Copyright (C) 2013-2017 Alexander Saprykin <saprykin.spb@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * 'Software'), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED 'AS IS', WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include "plibsys.h"
#include "ptestmacros.h"

#include <string.h>

P_TEST_MODULE_INIT ();

typedef struct _TestData {
	pint test_array[3];
	pint index;
} TestData;

extern "C" ppointer pmem_alloc (psize nbytes)
{
	P_UNUSED (nbytes);
	return (ppointer) NULL;
}

extern "C" ppointer pmem_realloc (ppointer block, psize nbytes)
{
	P_UNUSED (block);
	P_UNUSED (nbytes);
	return (ppointer) NULL;
}

extern "C" void pmem_free (ppointer block)
{
	P_UNUSED (block);
}

static void foreach_test_func (ppointer data, ppointer user_data)
{
	if (user_data == NULL)
		return;

	TestData *test_data = (TestData *) user_data;

	if (test_data->index < 0 || test_data->index > 2)
		return;

	test_data->test_array[test_data->index] = P_POINTER_TO_INT (data);
	++test_data->index;
}

P_TEST_CASE_BEGIN (plist_nomem_test)
{
	p_libsys_init ();

	PMemVTable vtable;

	vtable.free    = pmem_free;
	vtable.malloc  = pmem_alloc;
	vtable.realloc = pmem_realloc;

	P_TEST_CHECK (p_mem_set_vtable (&vtable) == TRUE);

	P_TEST_CHECK (p_list_append (NULL, PINT_TO_POINTER (10)) == NULL);
	P_TEST_CHECK (p_list_prepend (NULL, PINT_TO_POINTER (10)) == NULL);

	p_mem_restore_vtable ();

	p_libsys_shutdown ();
}
P_TEST_CASE_END ()

P_TEST_CASE_BEGIN (plist_invalid_test)
{
	p_libsys_init ();

	P_TEST_CHECK (p_list_remove (NULL, NULL) == NULL);
	P_TEST_CHECK (p_list_last (NULL) == NULL);
	P_TEST_CHECK (p_list_length (NULL) == 0);
	P_TEST_CHECK (p_list_reverse (NULL) == NULL);

	p_list_free (NULL);
	p_list_foreach (NULL, NULL, NULL);

	p_libsys_shutdown ();
}
P_TEST_CASE_END ()

P_TEST_CASE_BEGIN (plist_general_test)
{
	PList		*list = NULL;
	TestData	test_data;

	p_libsys_init ();

	/* Testing append */
	list = p_list_append (list, P_INT_TO_POINTER (32));
	list = p_list_append (list, P_INT_TO_POINTER (64));

	P_TEST_REQUIRE (list != NULL);
	P_TEST_CHECK (p_list_length (list) == 2);

	/* Testing data access */
	P_TEST_CHECK (P_POINTER_TO_INT (list->data) == 32);
	P_TEST_CHECK (P_POINTER_TO_INT (p_list_last(list)->data) == 64);

	/* Testing prepend */
	list = p_list_prepend (list, P_INT_TO_POINTER (128));
	P_TEST_REQUIRE (list != NULL);
	P_TEST_CHECK (p_list_length (list) == 3);
	P_TEST_CHECK (P_POINTER_TO_INT (list->data) == 128);
	P_TEST_CHECK (P_POINTER_TO_INT (p_list_last(list)->data) == 64);

	/* Testing for each loop */
	memset (&test_data, 0, sizeof (test_data));

	P_TEST_REQUIRE (test_data.test_array[0] == 0);
	P_TEST_REQUIRE (test_data.test_array[1] == 0);
	P_TEST_REQUIRE (test_data.test_array[2] == 0);
	P_TEST_REQUIRE (test_data.index == 0);

	p_list_foreach (list, (PFunc) foreach_test_func, (ppointer) &test_data);

	P_TEST_CHECK (test_data.index == 3);
	P_TEST_CHECK (test_data.test_array[0] == 128);
	P_TEST_CHECK (test_data.test_array[1] == 32);
	P_TEST_CHECK (test_data.test_array[2] == 64);

	/* Testing reverse */

	list = p_list_reverse (list);

	P_TEST_CHECK (list != NULL);
	P_TEST_CHECK (p_list_length (list) == 3);
	P_TEST_CHECK (P_POINTER_TO_INT (list->data) == 64);
	P_TEST_CHECK (P_POINTER_TO_INT (p_list_last(list)->data) == 128);

	/* Testing for each loop */
	memset (&test_data, 0, sizeof (test_data));

	P_TEST_REQUIRE (test_data.test_array[0] == 0);
	P_TEST_REQUIRE (test_data.test_array[1] == 0);
	P_TEST_REQUIRE (test_data.test_array[2] == 0);
	P_TEST_REQUIRE (test_data.index == 0);

	p_list_foreach (list, (PFunc) foreach_test_func, (ppointer) &test_data);

	P_TEST_CHECK (test_data.index == 3);
	P_TEST_CHECK (test_data.test_array[0] == 64);
	P_TEST_CHECK (test_data.test_array[1] == 32);
	P_TEST_CHECK (test_data.test_array[2] == 128);

	/* Testing remove */
	list = p_list_remove (list, P_INT_TO_POINTER (32));
	P_TEST_REQUIRE (list != NULL);
	P_TEST_CHECK (p_list_length (list) == 2);

	list = p_list_remove (list, P_INT_TO_POINTER (128));
	P_TEST_REQUIRE (list != NULL);
	P_TEST_CHECK (p_list_length (list) == 1);

	list = p_list_remove (list, P_INT_TO_POINTER (256));
	P_TEST_REQUIRE (list != NULL);
	P_TEST_CHECK (p_list_length (list) == 1);

	list = p_list_remove (list, P_INT_TO_POINTER (64));
	P_TEST_REQUIRE (list == NULL);
	P_TEST_CHECK (p_list_length (list) == 0);

	p_list_free (list);

	p_libsys_shutdown ();
}
P_TEST_CASE_END ()

P_TEST_SUITE_BEGIN()
{
	P_TEST_SUITE_RUN_CASE (plist_nomem_test);
	P_TEST_SUITE_RUN_CASE (plist_invalid_test);
	P_TEST_SUITE_RUN_CASE (plist_general_test);
}
P_TEST_SUITE_END()
