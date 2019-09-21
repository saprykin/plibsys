/*
 * The MIT License
 *
 * Copyright (C) 2013-2019 Alexander Saprykin <saprykin.spb@gmail.com>
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

#include <stdlib.h>
#include <time.h>

P_TEST_MODULE_INIT ();

static void * shm_test_thread (void *arg)
{
	pint		rand_num;
	psize		shm_size;
	ppointer	addr;
	PShm		*shm;

	if (arg == NULL)
		p_uthread_exit (1);

	shm = (PShm *) arg;
	rand_num = rand () % 127;
	shm_size = p_shm_get_size (shm);
	addr = p_shm_get_address (shm);

	if (shm_size == 0 || addr == NULL)
		p_uthread_exit (1);

	if (!p_shm_lock (shm, NULL))
		p_uthread_exit (1);

	for (puint i = 0; i < shm_size; ++i)
		*(((pchar *) addr) + i) = (pchar) rand_num;

	if (!p_shm_unlock (shm, NULL))
		p_uthread_exit (1);

	p_uthread_exit (0);

	return NULL;
}

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

P_TEST_CASE_BEGIN (pshm_nomem_test)
{
	p_libsys_init ();

	PMemVTable vtable;

	vtable.free    = pmem_free;
	vtable.malloc  = pmem_alloc;
	vtable.realloc = pmem_realloc;

	P_TEST_CHECK (p_mem_set_vtable (&vtable) == TRUE);

	P_TEST_CHECK (p_shm_new ("p_shm_test_memory_block", 1024, P_SHM_ACCESS_READWRITE, NULL) == NULL);

	p_mem_restore_vtable ();

	p_libsys_shutdown ();
}
P_TEST_CASE_END ()

P_TEST_CASE_BEGIN (pshm_invalid_test)
{
	p_libsys_init ();

	P_TEST_CHECK (p_shm_new (NULL, 0, P_SHM_ACCESS_READWRITE, NULL) == NULL);
	P_TEST_CHECK (p_shm_lock (NULL, NULL) == FALSE);
	P_TEST_CHECK (p_shm_unlock (NULL, NULL) == FALSE);
	P_TEST_CHECK (p_shm_get_address (NULL) == NULL);
	P_TEST_CHECK (p_shm_get_size (NULL) == 0);
	p_shm_take_ownership (NULL);

	PShm *shm = p_shm_new ("p_shm_invalid_test", 0, P_SHM_ACCESS_READWRITE, NULL);
	p_shm_take_ownership (shm);
	p_shm_free (shm);

	shm = p_shm_new ("p_shm_invalid_test", 10, (PShmAccessPerms) -1, NULL);
	p_shm_take_ownership (shm);
	p_shm_free (shm);

	p_libsys_shutdown ();
}
P_TEST_CASE_END ()

P_TEST_CASE_BEGIN (pshm_general_test)
{
	PShm		*shm = NULL;
#ifndef P_OS_HPUX
	PShm		*shm2 = NULL;
#endif
	ppointer	addr, addr2;
	pint		i;

	p_libsys_init ();

	shm = p_shm_new ("p_shm_test_memory_block", 1024, P_SHM_ACCESS_READWRITE, NULL);
	P_TEST_REQUIRE (shm != NULL);
	p_shm_take_ownership (shm);
	p_shm_free (shm);

	shm = p_shm_new ("p_shm_test_memory_block", 1024, P_SHM_ACCESS_READWRITE, NULL);
	P_TEST_REQUIRE (shm != NULL);
	P_TEST_REQUIRE (p_shm_get_size (shm) == 1024);

	addr = p_shm_get_address (shm);
	P_TEST_REQUIRE (addr != NULL);

#ifndef P_OS_HPUX
	shm2 = p_shm_new ("p_shm_test_memory_block", 1024, P_SHM_ACCESS_READONLY, NULL);

	if (shm2 == NULL) {
		/* OK, some systems may want exactly the same permissions */
		shm2 = p_shm_new ("p_shm_test_memory_block", 1024, P_SHM_ACCESS_READWRITE, NULL);
	}

	P_TEST_REQUIRE (shm2 != NULL);
	P_TEST_REQUIRE (p_shm_get_size (shm2) == 1024);

	addr2 = p_shm_get_address (shm2);
	P_TEST_REQUIRE (addr2 != NULL);
#endif

	for (i = 0; i < 512; ++i) {
		P_TEST_CHECK (p_shm_lock (shm, NULL));
		*(((pchar *) addr) + i) = 'a';
		P_TEST_CHECK (p_shm_unlock (shm, NULL));
	}

#ifndef P_OS_HPUX
	for (i = 0; i < 512; ++i) {
		P_TEST_CHECK (p_shm_lock (shm2, NULL));
		P_TEST_CHECK (*(((pchar *) addr) + i) == 'a');
		P_TEST_CHECK (p_shm_unlock (shm2, NULL));
	}
#else
	for (i = 0; i < 512; ++i) {
		P_TEST_CHECK (p_shm_lock (shm, NULL));
		P_TEST_CHECK (*(((pchar *) addr) + i) == 'a');
		P_TEST_CHECK (p_shm_unlock (shm, NULL));
	}
#endif

	for (i = 0; i < 1024; ++i) {
		P_TEST_CHECK (p_shm_lock (shm, NULL));
		*(((pchar *) addr) + i) = 'b';
		P_TEST_CHECK (p_shm_unlock (shm, NULL));
	}

#ifndef P_OS_HPUX
	for (i = 0; i < 1024; ++i) {
		P_TEST_CHECK (p_shm_lock (shm2, NULL));
		P_TEST_CHECK (*(((pchar *) addr) + i) != 'c');
		P_TEST_CHECK (p_shm_unlock (shm2, NULL));
	}

	for (i = 0; i < 1024; ++i) {
		P_TEST_CHECK (p_shm_lock (shm2, NULL));
		P_TEST_CHECK (*(((pchar *) addr) + i) == 'b');
		P_TEST_CHECK (p_shm_unlock (shm2, NULL));
	}
#else
	for (i = 0; i < 1024; ++i) {
		P_TEST_CHECK (p_shm_lock (shm, NULL));
		P_TEST_CHECK (*(((pchar *) addr) + i) != 'c');
		P_TEST_CHECK (p_shm_unlock (shm, NULL));
	}

	for (i = 0; i < 1024; ++i) {
		P_TEST_CHECK (p_shm_lock (shm, NULL));
		P_TEST_CHECK (*(((pchar *) addr) + i) == 'b');
		P_TEST_CHECK (p_shm_unlock (shm, NULL));
	}
#endif

	p_shm_free (shm);

	shm = p_shm_new ("p_shm_test_memory_block_2", 1024, P_SHM_ACCESS_READWRITE, NULL);
	P_TEST_REQUIRE (shm != NULL);
	P_TEST_REQUIRE (p_shm_get_size (shm) == 1024);

	addr = p_shm_get_address (shm);
	P_TEST_REQUIRE (addr != NULL);

	for (i = 0; i < 1024; ++i) {
		P_TEST_CHECK (p_shm_lock (shm, NULL));
		P_TEST_CHECK (*(((pchar *) addr) + i) != 'b');
		P_TEST_CHECK (p_shm_unlock (shm, NULL));
	}

	p_shm_free (shm);

#ifndef P_OS_HPUX
	p_shm_free (shm2);
#endif

	p_libsys_shutdown ();
}
P_TEST_CASE_END ()

P_TEST_CASE_BEGIN (pshm_thread_test)
{
	PShm		*shm;
	PUThread	*thr1, *thr2, *thr3;
	ppointer	addr;
	pint		i, val;
	pboolean	test_ok;

	p_libsys_init ();

	srand ((puint) time (NULL));

	shm = p_shm_new ("p_shm_test_memory_block", 1024 * 1024, P_SHM_ACCESS_READWRITE, NULL);
	P_TEST_REQUIRE (shm != NULL);
	p_shm_take_ownership (shm);
	p_shm_free (shm);

	shm = p_shm_new ("p_shm_test_memory_block", 1024 * 1024, P_SHM_ACCESS_READWRITE, NULL);
	P_TEST_REQUIRE (shm != NULL);

	if (p_shm_get_size (shm) != 1024 * 1024) {
		p_shm_free (shm);
		shm = p_shm_new ("p_shm_test_memory_block", 1024 * 1024, P_SHM_ACCESS_READWRITE, NULL);
		P_TEST_REQUIRE (shm != NULL);
	}

	P_TEST_REQUIRE (p_shm_get_size (shm) == 1024 * 1024);

	addr = p_shm_get_address (shm);
	P_TEST_REQUIRE (addr != NULL);

	thr1 = p_uthread_create ((PUThreadFunc) shm_test_thread, (ppointer) shm, TRUE, NULL);
	P_TEST_REQUIRE (thr1 != NULL);

	thr2 = p_uthread_create ((PUThreadFunc) shm_test_thread, (ppointer) shm, TRUE, NULL);
	P_TEST_REQUIRE (thr2 != NULL);

	thr3 = p_uthread_create ((PUThreadFunc) shm_test_thread, (ppointer) shm, TRUE, NULL);
	P_TEST_REQUIRE (thr3 != NULL);

	P_TEST_CHECK (p_uthread_join (thr1) == 0);
	P_TEST_CHECK (p_uthread_join (thr2) == 0);
	P_TEST_CHECK (p_uthread_join (thr3) == 0);

	test_ok = TRUE;
	val = *((pchar *) addr);

	for (i = 1; i < 1024 * 1024; ++i)
		if (*(((pchar *) addr) + i) != val) {
			test_ok = FALSE;
			break;
		}

	P_TEST_REQUIRE (test_ok == TRUE);

	p_uthread_unref (thr1);
	p_uthread_unref (thr2);
	p_uthread_unref (thr3);
	p_shm_free (shm);

	p_libsys_shutdown ();
}
P_TEST_CASE_END ()

P_TEST_SUITE_BEGIN()
{
	P_TEST_SUITE_RUN_CASE (pshm_nomem_test);
	P_TEST_SUITE_RUN_CASE (pshm_invalid_test);
	P_TEST_SUITE_RUN_CASE (pshm_general_test);
	P_TEST_SUITE_RUN_CASE (pshm_thread_test);
}
P_TEST_SUITE_END()
