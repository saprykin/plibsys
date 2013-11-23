#define BOOST_TEST_MODULE pmem_test

#include "plib.h"

#include <string.h>

#include <boost/test/included/unit_test.hpp>

BOOST_AUTO_TEST_SUITE (BOOST_TEST_MODULE)

BOOST_AUTO_TEST_CASE (pmem_general_test)
{
	PMemVTable	vtable;
	ppointer	ptr = NULL;
	pint		i;

	vtable.free = NULL;
	vtable.malloc = NULL;
	vtable.realloc = NULL;

	BOOST_CHECK (p_malloc (0) == NULL);
	BOOST_CHECK (p_malloc0 (0) == NULL);
	BOOST_CHECK (p_realloc (0, 0) == NULL);
	BOOST_CHECK (p_mem_set_vtable (NULL) == FALSE);
	BOOST_CHECK (p_mem_set_vtable (&vtable) == FALSE);
	p_free (NULL);

	ptr = p_mem_mmap (0);
	BOOST_CHECK (ptr == NULL);

	ptr = p_mem_mmap (1024);
	BOOST_REQUIRE (ptr != NULL);

	for (i = 0; i < 1024; ++i)
		*(((pchar *) ptr) + i) = i % 127;

	for (i = 0; i < 1024; ++i)
		BOOST_CHECK (*(((pchar *) ptr) + i) == i % 127);

	BOOST_CHECK (p_mem_munmap (NULL, 1024) == FALSE);
	BOOST_CHECK (p_mem_munmap (ptr, 1024) == TRUE);
}

BOOST_AUTO_TEST_SUITE_END()
