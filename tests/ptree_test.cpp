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

#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

P_TEST_MODULE_INIT ();

#define PTREE_STRESS_ITERATIONS	20
#define PTREE_STRESS_NODES	10000
#define PTREE_STRESS_ROOT_MIN	10000
#define PTREE_STRESS_TRAVS	30

typedef struct _TreeData {
	pint	cmp_counter;
	pint	key_destroy_counter;
	pint	value_destroy_counter;
	pint	traverse_counter;
	pint	traverse_thres;
	pint	key_sum;
	pint	value_sum;
	pint	last_key;
	pint	key_order_errors;
} TreeData;

static TreeData tree_data = {0, 0, 0, 0, 0, 0, 0, 0, 0};

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

static pint
tree_complexity (PTree *tree)
{
	if (tree == NULL || p_tree_get_nnodes (tree) == 0)
		return 0;

	switch (p_tree_get_type (tree)) {
	case P_TREE_TYPE_BINARY:
		return p_tree_get_nnodes (tree);
	case P_TREE_TYPE_RB:
		return 2 * ((pint) (log ((double) p_tree_get_nnodes (tree) + 1) / log (2.0)));
	case P_TREE_TYPE_AVL:
	{
		double phi = (1 + sqrt (5.0)) / 2.0;
		return (pint) (log (sqrt (5.0) * (p_tree_get_nnodes (tree) + 2)) / log (phi) - 2);
	}
	default:
		return p_tree_get_nnodes (tree);
	}
}

static pint
compare_keys (pconstpointer a, pconstpointer b)
{
	int p1 = PPOINTER_TO_INT (a);
	int p2 = PPOINTER_TO_INT (b);

	if (p1 < p2)
		return -1;
	else if (p1 > p2)
		return 1;
	else
		return 0;
}

static pint
compare_keys_data (pconstpointer a, pconstpointer b, ppointer data)
{
	int p1 = PPOINTER_TO_INT (a);
	int p2 = PPOINTER_TO_INT (b);

	if (data != NULL)
		((TreeData *) data)->cmp_counter++;

	if (p1 < p2)
		return -1;
	else if (p1 > p2)
		return 1;
	else
		return 0;
}

static void
key_destroy_notify (ppointer data)
{
	tree_data.key_destroy_counter++;
	tree_data.key_sum += PPOINTER_TO_INT (data);
}

static void
value_destroy_notify (ppointer data)
{
	tree_data.value_destroy_counter++;
	tree_data.value_sum += PPOINTER_TO_INT (data);
}

static pboolean
tree_traverse (ppointer key, ppointer value, ppointer data)
{
	TreeData* tdata = ((TreeData *) data);

	tdata->key_sum	 += PPOINTER_TO_INT (key);
	tdata->value_sum += PPOINTER_TO_INT (value);
	tdata->traverse_counter++;

	if (tdata->last_key >= PPOINTER_TO_INT (key))
		tdata->key_order_errors++;

	tdata->last_key = PPOINTER_TO_INT (key);

	return FALSE;
}

static pboolean
tree_traverse_thres (ppointer key, ppointer value, ppointer data)
{
	TreeData* tdata = ((TreeData *) data);

	tree_traverse (key, value, data);

	return tdata->traverse_counter >= tdata->traverse_thres ? TRUE : FALSE;
}

static bool
check_tree_data_is_zero ()
{
	return tree_data.cmp_counter           == 0 &&
	       tree_data.key_destroy_counter   == 0 &&
	       tree_data.value_destroy_counter == 0 &&
	       tree_data.traverse_counter      == 0 &&
	       tree_data.traverse_thres	       == 0 &&
	       tree_data.key_sum               == 0 &&
	       tree_data.value_sum             == 0 &&
	       tree_data.last_key              == 0 &&
	       tree_data.key_order_errors      == 0;
}

static bool
general_tree_test (PTree *tree, PTreeType type, bool check_cmp, bool check_notify)
{
	memset (&tree_data, 0, sizeof (tree_data));

	P_TEST_REQUIRE (tree != NULL);
	P_TEST_CHECK (p_tree_get_nnodes (tree) == 0);
	P_TEST_CHECK (p_tree_get_type (tree) == type);
	P_TEST_CHECK (p_tree_lookup (tree, NULL) == NULL);
	P_TEST_CHECK (p_tree_remove (tree, NULL) == FALSE);

	p_tree_insert (tree, NULL, PINT_TO_POINTER (10));
	P_TEST_CHECK (p_tree_get_nnodes (tree) == 1);
	P_TEST_CHECK (p_tree_lookup (tree, NULL) == PINT_TO_POINTER (10));
	P_TEST_CHECK (p_tree_lookup (tree, PINT_TO_POINTER (2)) == NULL);
	P_TEST_CHECK (p_tree_remove (tree, NULL) == TRUE);
	P_TEST_CHECK (p_tree_get_nnodes (tree) == 0);

	p_tree_foreach (tree, (PTraverseFunc) tree_traverse, &tree_data);
	P_TEST_CHECK (tree_data.traverse_counter == 0);
	P_TEST_CHECK (tree_data.key_order_errors == 0);

	/* Because we have NULL-key node */
	P_TEST_CHECK (tree_data.key_sum == 0);

	if (check_notify)
		P_TEST_CHECK (tree_data.value_sum == 10);
	else
		P_TEST_CHECK (tree_data.value_sum == 0);

	memset (&tree_data, 0, sizeof (tree_data));

	p_tree_insert (tree, PINT_TO_POINTER (4), PINT_TO_POINTER (40));
	p_tree_insert (tree, PINT_TO_POINTER (1), PINT_TO_POINTER (10));
	p_tree_insert (tree, PINT_TO_POINTER (5), PINT_TO_POINTER (50));
	p_tree_insert (tree, PINT_TO_POINTER (2), PINT_TO_POINTER (20));
	p_tree_insert (tree, PINT_TO_POINTER (6), PINT_TO_POINTER (60));
	p_tree_insert (tree, PINT_TO_POINTER (3), PINT_TO_POINTER (30));

	P_TEST_CHECK (p_tree_get_nnodes (tree) == 6);

	p_tree_insert (tree, PINT_TO_POINTER (1), PINT_TO_POINTER (100));
	p_tree_insert (tree, PINT_TO_POINTER (5), PINT_TO_POINTER (500));

	P_TEST_CHECK (p_tree_get_nnodes (tree) == 6);

	p_tree_insert (tree, PINT_TO_POINTER (1), PINT_TO_POINTER (10));
	p_tree_insert (tree, PINT_TO_POINTER (5), PINT_TO_POINTER (50));

	P_TEST_CHECK (p_tree_get_nnodes (tree) == 6);

	if (check_cmp)
		P_TEST_CHECK (tree_data.cmp_counter > 0);
	else
		P_TEST_CHECK (tree_data.cmp_counter == 0);

	if (check_notify) {
		P_TEST_CHECK (tree_data.key_sum   == 12);
		P_TEST_CHECK (tree_data.value_sum == 660);
	} else {
		P_TEST_CHECK (tree_data.key_sum   == 0);
		P_TEST_CHECK (tree_data.value_sum == 0);
	}

	P_TEST_CHECK (tree_data.traverse_counter == 0);
	P_TEST_CHECK (tree_data.key_order_errors == 0);

	memset (&tree_data, 0, sizeof (tree_data));

	p_tree_foreach (tree, (PTraverseFunc) tree_traverse, &tree_data);
	P_TEST_CHECK (p_tree_get_nnodes (tree) == 6);

	P_TEST_CHECK (tree_data.cmp_counter      == 0);
	P_TEST_CHECK (tree_data.key_sum          == 21);
	P_TEST_CHECK (tree_data.value_sum        == 210);
	P_TEST_CHECK (tree_data.traverse_counter == 6);
	P_TEST_CHECK (tree_data.key_order_errors == 0);

	memset (&tree_data, 0, sizeof (tree_data));

	for (int i = 0; i < 7; ++i)
		P_TEST_CHECK (p_tree_lookup (tree, PINT_TO_POINTER (i)) == PINT_TO_POINTER (i * 10));

	if (check_cmp)
		P_TEST_CHECK (tree_data.cmp_counter > 0);
	else
		P_TEST_CHECK (tree_data.cmp_counter == 0);

	P_TEST_CHECK (tree_data.key_sum          == 0);
	P_TEST_CHECK (tree_data.value_sum        == 0);
	P_TEST_CHECK (tree_data.key_order_errors == 0);

	tree_data.cmp_counter = 0;

	P_TEST_CHECK (p_tree_remove (tree, PINT_TO_POINTER (7)) == FALSE);

	if (check_cmp)
		P_TEST_CHECK (tree_data.cmp_counter > 0 &&
			     tree_data.cmp_counter <= tree_complexity (tree));
	else
		P_TEST_CHECK (tree_data.cmp_counter == 0);

	if (check_notify) {
		P_TEST_CHECK (tree_data.key_sum   == 0);
		P_TEST_CHECK (tree_data.value_sum == 0);
	}

	tree_data.cmp_counter = 0;

	for (int i = 0; i < 7; ++i)
		P_TEST_CHECK (p_tree_lookup (tree, PINT_TO_POINTER (i)) == PINT_TO_POINTER (i * 10));

	if (check_cmp)
		P_TEST_CHECK (tree_data.cmp_counter > 0);
	else
		P_TEST_CHECK (tree_data.cmp_counter == 0);

	P_TEST_CHECK (tree_data.key_sum          == 0);
	P_TEST_CHECK (tree_data.value_sum        == 0);
	P_TEST_CHECK (tree_data.key_order_errors == 0);

	memset (&tree_data, 0, sizeof (tree_data));

	tree_data.traverse_thres = 5;

	p_tree_foreach (tree, (PTraverseFunc) tree_traverse_thres, &tree_data);
	P_TEST_CHECK (p_tree_get_nnodes (tree) == 6);

	P_TEST_CHECK (tree_data.cmp_counter      == 0);
	P_TEST_CHECK (tree_data.key_sum          == 15);
	P_TEST_CHECK (tree_data.value_sum        == 150);
	P_TEST_CHECK (tree_data.traverse_counter == 5);
	P_TEST_CHECK (tree_data.key_order_errors == 0);

	memset (&tree_data, 0, sizeof (tree_data));

	tree_data.traverse_thres = 3;

	p_tree_foreach (tree, (PTraverseFunc) tree_traverse_thres, &tree_data);
	P_TEST_CHECK (p_tree_get_nnodes (tree) == 6);

	P_TEST_CHECK (tree_data.cmp_counter      == 0);
	P_TEST_CHECK (tree_data.key_sum          == 6);
	P_TEST_CHECK (tree_data.value_sum        == 60);
	P_TEST_CHECK (tree_data.traverse_counter == 3);
	P_TEST_CHECK (tree_data.key_order_errors == 0);

	memset (&tree_data, 0, sizeof (tree_data));

	P_TEST_CHECK (p_tree_remove (tree, PINT_TO_POINTER (1)) == TRUE);
	P_TEST_CHECK (p_tree_remove (tree, PINT_TO_POINTER (6)) == TRUE);
	P_TEST_CHECK (p_tree_lookup (tree, PINT_TO_POINTER (1)) == NULL);
	P_TEST_CHECK (p_tree_lookup (tree, PINT_TO_POINTER (6)) == NULL);

	if (check_cmp)
		P_TEST_CHECK (tree_data.cmp_counter > 0);
	else
		P_TEST_CHECK (tree_data.cmp_counter == 0);

	if (check_notify) {
		P_TEST_CHECK (tree_data.key_sum   == 7);
		P_TEST_CHECK (tree_data.value_sum == 70);
	} else {
		P_TEST_CHECK (tree_data.key_sum   == 0);
		P_TEST_CHECK (tree_data.value_sum == 0);
	}

	tree_data.cmp_counter = 0;

	for (int i = 2; i < 6; ++i)
		P_TEST_CHECK (p_tree_lookup (tree, PINT_TO_POINTER (i)) == PINT_TO_POINTER (i * 10));

	if (check_cmp)
		P_TEST_CHECK (tree_data.cmp_counter > 0);
	else
		P_TEST_CHECK (tree_data.cmp_counter == 0);

	if (check_notify) {
		P_TEST_CHECK (tree_data.key_sum   == 7);
		P_TEST_CHECK (tree_data.value_sum == 70);
	} else {
		P_TEST_CHECK (tree_data.key_sum   == 0);
		P_TEST_CHECK (tree_data.value_sum == 0);
	}

	P_TEST_CHECK (tree_data.key_order_errors == 0);

	tree_data.cmp_counter = 0;

	p_tree_foreach (tree, NULL, NULL);

	P_TEST_CHECK (tree_data.cmp_counter      == 0);
	P_TEST_CHECK (tree_data.key_order_errors == 0);

	p_tree_clear (tree);

	P_TEST_CHECK (tree_data.cmp_counter      == 0);
	P_TEST_CHECK (tree_data.key_order_errors == 0);

	if (check_notify) {
		P_TEST_CHECK (tree_data.key_sum   == 21);
		P_TEST_CHECK (tree_data.value_sum == 210);
	} else {
		P_TEST_CHECK (tree_data.key_sum   == 0);
		P_TEST_CHECK (tree_data.value_sum == 0);
	}

	P_TEST_CHECK (p_tree_get_nnodes (tree) == 0);

	return true;
}

static bool
stress_tree_test (PTree *tree, int node_count)
{
	P_TEST_REQUIRE (tree != NULL);
	P_TEST_REQUIRE (node_count > 0);
	P_TEST_CHECK (p_tree_get_nnodes (tree) == 0);

	srand ((unsigned int) time (NULL));

	int counter = 0;

	memset (&tree_data, 0, sizeof (tree_data));

	pint *keys   = (pint *) p_malloc0 ((psize) node_count * sizeof (pint));
	pint *values = (pint *) p_malloc0 ((psize) node_count * sizeof (pint));

	P_TEST_REQUIRE (keys != NULL);
	P_TEST_REQUIRE (values != NULL);

	while (counter != node_count) {
		pint rand_number = rand ();

		if (counter == 0 && rand_number < PTREE_STRESS_ROOT_MIN)
			continue;

		memset (&tree_data, 0, sizeof (tree_data));

		if (p_tree_lookup (tree, PINT_TO_POINTER (rand_number)) != NULL)
			continue;

		if (counter > 0)
			P_TEST_CHECK (tree_data.cmp_counter > 0 &&
				     tree_data.cmp_counter <= tree_complexity (tree));

		memset (&tree_data, 0, sizeof (tree_data));

		keys[counter]   = rand_number;
		values[counter] = rand () + 1;

		p_tree_insert (tree, PINT_TO_POINTER (keys[counter]), PINT_TO_POINTER (values[counter]));

		if (counter > 0)
			P_TEST_CHECK (tree_data.cmp_counter > 0 &&
				     tree_data.cmp_counter <= tree_complexity (tree));

		++counter;
	}

	for (int i = 0; i < PTREE_STRESS_TRAVS; ++i) {
		memset (&tree_data, 0, sizeof (tree_data));

		tree_data.traverse_thres = i + 1;
		tree_data.last_key       = -1;

		p_tree_foreach (tree, (PTraverseFunc) tree_traverse_thres, &tree_data);

		P_TEST_CHECK (tree_data.traverse_counter == i + 1);
		P_TEST_CHECK (tree_data.key_order_errors == 0);
	}

	for (int i = 0; i < node_count; ++i) {
		memset (&tree_data, 0, sizeof (tree_data));

		P_TEST_CHECK (p_tree_lookup (tree, PINT_TO_POINTER (keys[i])) ==
			     PINT_TO_POINTER (values[i]));

		P_TEST_CHECK (tree_data.cmp_counter > 0 &&
			     tree_data.cmp_counter <= tree_complexity (tree));

		P_TEST_CHECK (p_tree_remove (tree, PINT_TO_POINTER (keys[i])) == TRUE);
		P_TEST_CHECK (p_tree_lookup (tree, PINT_TO_POINTER (keys[i])) == NULL);
	}

	P_TEST_CHECK (p_tree_get_nnodes (tree) == 0);

	for (int i = 0; i < node_count; ++i)
		p_tree_insert (tree, PINT_TO_POINTER (keys[i]), PINT_TO_POINTER (values[i]));

	P_TEST_CHECK (p_tree_get_nnodes (tree) == node_count);

	p_tree_clear (tree);

	P_TEST_CHECK (p_tree_get_nnodes (tree) == 0);

	p_free (keys);
	p_free (values);

	return true;
}

P_TEST_CASE_BEGIN (ptree_nomem_test)
{
	p_libsys_init ();

	PMemVTable vtable;

	for (int i = (int) P_TREE_TYPE_BINARY; i <= (int) P_TREE_TYPE_AVL; ++i) {
		PTree *tree = p_tree_new ((PTreeType) i, (PCompareFunc) compare_keys);
		P_TEST_CHECK (tree != NULL);

		vtable.free    = pmem_free;
		vtable.malloc  = pmem_alloc;
		vtable.realloc = pmem_realloc;

		P_TEST_CHECK (p_mem_set_vtable (&vtable) == TRUE);

		P_TEST_CHECK (p_tree_new ((PTreeType) i, (PCompareFunc) compare_keys) == NULL);
		p_tree_insert (tree, PINT_TO_POINTER (1), PINT_TO_POINTER (10));
		P_TEST_CHECK (p_tree_get_nnodes (tree) == 0);

		p_mem_restore_vtable ();

		p_tree_free (tree);
	}

	p_libsys_shutdown ();
}
P_TEST_CASE_END ()

P_TEST_CASE_BEGIN (ptree_invalid_test)
{
	p_libsys_init ();

	for (int i = (int) P_TREE_TYPE_BINARY; i <= (int) P_TREE_TYPE_AVL; ++i) {
		/* Invalid usage */
		P_TEST_CHECK (p_tree_new ((PTreeType) i, NULL) == NULL);
		P_TEST_CHECK (p_tree_new ((PTreeType) -1, (PCompareFunc) compare_keys) == NULL);
		P_TEST_CHECK (p_tree_new ((PTreeType) -1, NULL) == NULL);

		P_TEST_CHECK (p_tree_new_with_data ((PTreeType) i, NULL, NULL) == NULL);
		P_TEST_CHECK (p_tree_new_with_data ((PTreeType) -1, (PCompareDataFunc) compare_keys, NULL) == NULL);
		P_TEST_CHECK (p_tree_new_with_data ((PTreeType) -1, NULL, NULL) == NULL);

		P_TEST_CHECK (p_tree_new_full ((PTreeType) i,
					      NULL,
					      NULL,
					      NULL,
					      NULL) == NULL);
		P_TEST_CHECK (p_tree_new_full ((PTreeType) -1,
					      (PCompareDataFunc) compare_keys,
					      NULL,
					      NULL,
					      NULL) == NULL);
		P_TEST_CHECK (p_tree_new_full ((PTreeType) -1,
					      NULL,
					      NULL,
					      NULL,
					      NULL) == NULL);

		P_TEST_CHECK (p_tree_remove (NULL, NULL) == FALSE);
		P_TEST_CHECK (p_tree_lookup (NULL, NULL) == NULL);
		P_TEST_CHECK (p_tree_get_type (NULL) == (PTreeType) -1);
		P_TEST_CHECK (p_tree_get_nnodes (NULL) == 0);

		p_tree_insert (NULL, NULL, NULL);
		p_tree_foreach (NULL, NULL, NULL);
		p_tree_clear (NULL);
		p_tree_free (NULL);
	}

	p_libsys_shutdown ();
}
P_TEST_CASE_END ()

P_TEST_CASE_BEGIN (ptree_general_test)
{
	PTree *tree;

	p_libsys_init ();

	for (int i = (int) P_TREE_TYPE_BINARY; i <= (int) P_TREE_TYPE_AVL; ++i) {
		/* Test 1 */
		tree = p_tree_new ((PTreeType) i, (PCompareFunc) compare_keys);

		P_TEST_CHECK (general_tree_test (tree, (PTreeType) i, false, false) == true);

		memset (&tree_data, 0, sizeof (tree_data));
		p_tree_free (tree);

		P_TEST_CHECK (check_tree_data_is_zero () == true);

		/* Test 2 */
		tree = p_tree_new_with_data ((PTreeType) i,
					     (PCompareDataFunc) compare_keys_data,
					     &tree_data);

		P_TEST_CHECK (general_tree_test (tree, (PTreeType) i, true, false) == true);

		memset (&tree_data, 0, sizeof (tree_data));
		p_tree_free (tree);

		P_TEST_CHECK (check_tree_data_is_zero () == true);

		/* Test 3 */
		tree = p_tree_new_full ((PTreeType) i,
					(PCompareDataFunc) compare_keys_data,
					&tree_data,
					(PDestroyFunc) key_destroy_notify,
					(PDestroyFunc) value_destroy_notify);
		P_TEST_CHECK (general_tree_test (tree, (PTreeType) i, true, true) == true);

		memset (&tree_data, 0, sizeof (tree_data));
		p_tree_free (tree);

		P_TEST_CHECK (check_tree_data_is_zero () == true);
	}

	p_libsys_shutdown ();
}
P_TEST_CASE_END ()

P_TEST_CASE_BEGIN (ptree_stress_test)
{
	PTree *tree;

	p_libsys_init ();

	for (int i = (int) P_TREE_TYPE_BINARY; i <= (int) P_TREE_TYPE_AVL; ++i) {
		tree = p_tree_new_full ((PTreeType) i,
					(PCompareDataFunc) compare_keys_data,
					&tree_data,
					(PDestroyFunc) key_destroy_notify,
					(PDestroyFunc) value_destroy_notify);

		for (int j = 0; j < PTREE_STRESS_ITERATIONS; ++j)
			P_TEST_CHECK (stress_tree_test (tree, PTREE_STRESS_NODES) == true);

		p_tree_free (tree);
	}

	p_libsys_shutdown ();
}
P_TEST_CASE_END ()

P_TEST_SUITE_BEGIN()
{
	P_TEST_SUITE_RUN_CASE (ptree_nomem_test);
	P_TEST_SUITE_RUN_CASE (ptree_invalid_test);
	P_TEST_SUITE_RUN_CASE (ptree_general_test);
	P_TEST_SUITE_RUN_CASE (ptree_stress_test);
}
P_TEST_SUITE_END()
