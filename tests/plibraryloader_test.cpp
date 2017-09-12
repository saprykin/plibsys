/*
 * Copyright (C) 2015-2017 Alexander Saprykin <xelfium@gmail.com>
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

P_TEST_MODULE_INIT ();

static int g_argc    = 0;
static char **g_argv = NULL;

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

P_TEST_CASE_BEGIN (plibraryloader_nomem_test)
{
	p_libsys_init ();

	if (P_UNLIKELY (p_library_loader_is_ref_counted () == FALSE)) {
		p_libsys_shutdown ();
		P_TEST_CASE_RETURN ();
	}

	/* We assume that 3rd argument is ourself library path */
	P_TEST_REQUIRE (g_argc > 1);

	/* Cleanup from previous run */
	p_file_remove ("." P_DIR_SEPARATOR "p_empty_file.txt", NULL);

	FILE *file = fopen ("." P_DIR_SEPARATOR "p_empty_file.txt", "w");
	P_TEST_CHECK (file != NULL);
	P_TEST_CHECK (fclose (file) == 0);

	PMemVTable vtable;

	vtable.free    = pmem_free;
	vtable.malloc  = pmem_alloc;
	vtable.realloc = pmem_realloc;

	P_TEST_CHECK (p_mem_set_vtable (&vtable) == TRUE);

#ifdef P_OS_WIN
	SetErrorMode (SEM_FAILCRITICALERRORS);
#endif

	P_TEST_CHECK (p_library_loader_new ("." P_DIR_SEPARATOR "p_empty_file.txt") == NULL);
	P_TEST_CHECK (p_library_loader_new (g_argv[g_argc - 1]) == NULL);

#ifdef P_OS_WIN
	SetErrorMode (0);
#endif

	p_mem_restore_vtable ();

	P_TEST_CHECK (p_file_remove ("." P_DIR_SEPARATOR "p_empty_file.txt", NULL) == TRUE);

	p_libsys_shutdown ();
}
P_TEST_CASE_END ()

P_TEST_CASE_BEGIN (plibraryloader_general_test)
{
	PLibraryLoader	*loader;
	pchar		*err_msg;
	void		(*shutdown_func) (void);

	p_libsys_init ();

	/* We assume that 3rd argument is ourself library path */
	P_TEST_REQUIRE (g_argc > 1);

	/* Invalid usage */
	P_TEST_CHECK (p_library_loader_new (NULL) == NULL);
	P_TEST_CHECK (p_library_loader_new ("./unexistent_file.nofile") == NULL);
	P_TEST_CHECK (p_library_loader_get_symbol (NULL, NULL) == NULL);
	P_TEST_CHECK (p_library_loader_get_symbol (NULL, "unexistent_symbol") == NULL);

	p_library_loader_free (NULL);

	/* General tests */

	/* At least not on HP-UX it should be true */
#if !defined (P_OS_HPUX)
	P_TEST_CHECK (p_library_loader_is_ref_counted () == TRUE);
#else
	p_library_loader_is_ref_counted ();
#endif

	err_msg = p_library_loader_get_last_error (NULL);
	p_free (err_msg);

	if (P_UNLIKELY (p_library_loader_is_ref_counted () == FALSE)) {
		p_libsys_shutdown ();
		P_TEST_CASE_RETURN ();
	}

	loader = p_library_loader_new (g_argv[g_argc - 1]);
	P_TEST_REQUIRE (loader != NULL);

	P_TEST_CHECK (p_library_loader_get_symbol (loader, "there_is_no_such_a_symbol") == (PFuncAddr) NULL);

	err_msg = p_library_loader_get_last_error (loader);
	P_TEST_CHECK (err_msg != NULL);
	p_free (err_msg);

	shutdown_func = (void (*) (void)) p_library_loader_get_symbol (loader, "p_libsys_shutdown");

	if (shutdown_func == NULL)
		shutdown_func = (void (*) (void)) p_library_loader_get_symbol (loader, "_p_libsys_shutdown");

	/* For Watcom C */

	if (shutdown_func == NULL)
		shutdown_func = (void (*) (void)) p_library_loader_get_symbol (loader, "p_libsys_shutdown_");

	P_TEST_REQUIRE (shutdown_func != NULL);

	err_msg = p_library_loader_get_last_error (loader);
	p_free (err_msg);

	p_library_loader_free (loader);

#ifdef P_OS_BEOS
	p_libsys_shutdown ();
#else
	/* We have already loaded reference to ourself library, it's OK */
	shutdown_func ();
#endif
}
P_TEST_CASE_END ()

P_TEST_SUITE_ARGS_BEGIN()
{
	g_argc = argc;
	g_argv = argv;

	P_TEST_SUITE_RUN_CASE (plibraryloader_nomem_test);
	P_TEST_SUITE_RUN_CASE (plibraryloader_general_test);
}
P_TEST_SUITE_END()
