/*
 * The MIT License
 *
 * Copyright (C) 2015-2017 Alexander Saprykin <saprykin.spb@gmail.com>
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

#define PDIR_ENTRY_DIR		"test_2"
#define PDIR_ENTRY_FILE		"test_file.txt"
#define PDIR_TEST_DIR		"." P_DIR_SEPARATOR "pdir_test_dir"
#define PDIR_TEST_DIR_IN	"." P_DIR_SEPARATOR "pdir_test_dir" P_DIR_SEPARATOR "test_2"
#define PDIR_TEST_FILE		"." P_DIR_SEPARATOR "pdir_test_dir" P_DIR_SEPARATOR "test_file.txt"

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

P_TEST_CASE_BEGIN (pdir_nomem_test)
{
	p_libsys_init ();

	PMemVTable vtable;

	vtable.free    = pmem_free;
	vtable.malloc  = pmem_alloc;
	vtable.realloc = pmem_realloc;

	P_TEST_CHECK (p_mem_set_vtable (&vtable) == TRUE);

	/* Cleanup previous run */
	p_dir_remove (PDIR_TEST_DIR_IN, NULL);
	p_dir_remove (PDIR_TEST_DIR, NULL);

	P_TEST_REQUIRE (p_dir_create (PDIR_TEST_DIR, 0777, NULL) == TRUE);
	P_TEST_REQUIRE (p_dir_create (PDIR_TEST_DIR_IN, 0777, NULL) == TRUE);

	P_TEST_CHECK (p_dir_new (PDIR_TEST_DIR"/", NULL) == NULL);

	/* Revert memory management back */
	p_mem_restore_vtable ();

	/* Try out of memory when iterating */
	PDir *dir = p_dir_new (PDIR_TEST_DIR"/", NULL);
	P_TEST_CHECK (dir != NULL);

	vtable.free    = pmem_free;
	vtable.malloc  = pmem_alloc;
	vtable.realloc = pmem_realloc;

	P_TEST_CHECK (p_mem_set_vtable (&vtable) == TRUE);

	P_TEST_CHECK (p_dir_get_next_entry (dir, NULL) == NULL);

	/* Cleanup */
	p_mem_restore_vtable ();

	p_dir_free (dir);

	P_TEST_CHECK (p_dir_remove (PDIR_TEST_DIR_IN, NULL) == TRUE);
	P_TEST_CHECK (p_dir_remove (PDIR_TEST_DIR, NULL) == TRUE);

	p_libsys_shutdown ();
}
P_TEST_CASE_END ()

P_TEST_CASE_BEGIN (pdir_general_test)
{
	p_libsys_init ();

	P_TEST_CHECK (p_dir_new (NULL, NULL) == NULL);
	P_TEST_CHECK (p_dir_new ("." P_DIR_SEPARATOR "pdir_test_dir_new", NULL) == NULL);
	P_TEST_CHECK (p_dir_create (NULL, -1, NULL) == FALSE);
#ifndef P_OS_VMS
	P_TEST_CHECK (p_dir_create ("." P_DIR_SEPARATOR "pdir_test_dir_new" P_DIR_SEPARATOR "test_dir", -1, NULL) == FALSE);
#endif
	P_TEST_CHECK (p_dir_remove (NULL, NULL) == FALSE);
	P_TEST_CHECK (p_dir_remove ("." P_DIR_SEPARATOR "pdir_test_dir_new", NULL) == FALSE);
	P_TEST_CHECK (p_dir_is_exists (NULL) == FALSE);
	P_TEST_CHECK (p_dir_is_exists ("." P_DIR_SEPARATOR "pdir_test_dir_new") == FALSE);
	P_TEST_CHECK (p_dir_get_path (NULL) == NULL);
	P_TEST_CHECK (p_dir_get_next_entry (NULL, NULL) == NULL);
	P_TEST_CHECK (p_dir_rewind (NULL, NULL) == FALSE);

	p_dir_entry_free (NULL);
	p_dir_free (NULL);

	/* Cleanup previous run */
	p_dir_remove (PDIR_TEST_DIR_IN, NULL);
	p_dir_remove (PDIR_TEST_DIR, NULL);

	P_TEST_REQUIRE (p_dir_create (PDIR_TEST_DIR, 0777, NULL) == TRUE);
	P_TEST_REQUIRE (p_dir_create (PDIR_TEST_DIR, 0777, NULL) == TRUE);
	P_TEST_REQUIRE (p_dir_create (PDIR_TEST_DIR_IN, 0777, NULL) == TRUE);
	P_TEST_REQUIRE (p_dir_create (PDIR_TEST_DIR_IN, 0777, NULL) == TRUE);

	FILE *file = fopen (PDIR_TEST_FILE, "w");
	P_TEST_REQUIRE (file != NULL);
	P_TEST_REQUIRE (p_file_is_exists (PDIR_TEST_FILE) == TRUE);

	fprintf (file, "This is a test file string\n");

	P_TEST_CHECK (fclose (file) == 0);

	P_TEST_CHECK (p_dir_is_exists (PDIR_TEST_DIR) == TRUE);
	P_TEST_CHECK (p_dir_is_exists (PDIR_TEST_DIR_IN) == TRUE);

	PDir *dir = p_dir_new (PDIR_TEST_DIR"/", NULL);

	P_TEST_CHECK (dir != NULL);

	pint dir_count	= 0;
	pint file_count	= 0;
	pboolean has_entry_dir	= FALSE;
	pboolean has_entry_file	= FALSE;

	PDirEntry *entry;

	while ((entry = p_dir_get_next_entry (dir, NULL)) != NULL) {
		P_TEST_CHECK (entry->name != NULL);

		switch (entry->type) {
		case P_DIR_ENTRY_TYPE_DIR:
			++dir_count;
			break;
		case P_DIR_ENTRY_TYPE_FILE:
			++file_count;
			break;
		case P_DIR_ENTRY_TYPE_OTHER:
		default:
			break;
		}

		if (strcmp (entry->name, PDIR_ENTRY_DIR) == 0)
			has_entry_dir = TRUE;
		else if (strcmp (entry->name, PDIR_ENTRY_FILE) == 0)
			has_entry_file = TRUE;

		p_dir_entry_free (entry);
	}

	P_TEST_CHECK (dir_count > 0 && dir_count < 4);
	P_TEST_CHECK (file_count == 1);
	P_TEST_CHECK (has_entry_dir == TRUE);
	P_TEST_CHECK (has_entry_file == TRUE);

	P_TEST_CHECK (p_dir_rewind (dir, NULL) == TRUE);

	pint dir_count_2  = 0;
	pint file_count_2 = 0;
	has_entry_dir	= FALSE;
	has_entry_file	= FALSE;

	while ((entry = p_dir_get_next_entry (dir, NULL)) != NULL) {
		P_TEST_CHECK (entry->name != NULL);

		switch (entry->type) {
		case P_DIR_ENTRY_TYPE_DIR:
			++dir_count_2;
			break;
		case P_DIR_ENTRY_TYPE_FILE:
			++file_count_2;
			break;
		case P_DIR_ENTRY_TYPE_OTHER:
		default:
			break;
		}

		if (strcmp (entry->name, PDIR_ENTRY_DIR) == 0)
			has_entry_dir = TRUE;
		else if (strcmp (entry->name, PDIR_ENTRY_FILE) == 0)
			has_entry_file = TRUE;

		p_dir_entry_free (entry);
	}

	P_TEST_CHECK (dir_count_2 > 0 && dir_count_2 < 4);
	P_TEST_CHECK (file_count_2 == 1);
	P_TEST_CHECK (has_entry_dir == TRUE);
	P_TEST_CHECK (has_entry_file == TRUE);

	/* Compare two previous attempts */
	P_TEST_CHECK (dir_count == dir_count_2);
	P_TEST_CHECK (file_count == file_count_2);

	/* Remove all stuff */
	P_TEST_CHECK (p_file_remove (PDIR_TEST_FILE, NULL) == TRUE);
	P_TEST_CHECK (p_dir_remove (PDIR_TEST_DIR, NULL) == FALSE);
	P_TEST_CHECK (p_dir_remove (PDIR_TEST_DIR_IN, NULL) == TRUE);
	P_TEST_CHECK (p_dir_remove (PDIR_TEST_DIR, NULL) == TRUE);

	P_TEST_CHECK (p_dir_is_exists (PDIR_TEST_DIR_IN) == FALSE);
	P_TEST_CHECK (p_dir_is_exists (PDIR_TEST_DIR) == FALSE);

	pchar *orig_path = p_dir_get_path (dir);
	P_TEST_CHECK (strcmp (orig_path, PDIR_TEST_DIR"/") == 0);
	p_free (orig_path);

	p_dir_free (dir);

	p_libsys_shutdown ();
}
P_TEST_CASE_END ()

P_TEST_SUITE_BEGIN()
{
	P_TEST_SUITE_RUN_CASE (pdir_nomem_test);
	P_TEST_SUITE_RUN_CASE (pdir_general_test);
}
P_TEST_SUITE_END()
