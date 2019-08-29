// Copyright (C) 2019 Red Hat, Inc. All rights reserved.
// 
// This file is part of LVM2.
//
// This copyrighted material is made available to anyone wishing to use,
// modify, copy, or redistribute it subject to the terms and conditions
// of the GNU Lesser General Public License v.2.1.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, write to the Free Software Foundation,
// Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 
#include "units.h"
#include "base/data-struct/range-set.h"
#include "base/memory/container_of.h"

#include <stdio.h>
#include <stdlib.h>

//----------------------------------------------------------------

static void *rs_init(void)
{
	struct range_set *rs = range_set_create();
	T_ASSERT(rs);
	return rs;
}

static void rs_exit(void *fixture)
{
	range_set_destroy(fixture);
}

static void test_create_destroy(void *fixture)
{
	T_ASSERT(fixture);
}

static void test_lookup_empty(void *fixture)
{
	struct range_set *rs = fixture;
	struct range area, result;

	area.b = 16;
	area.e = 32;
	T_ASSERT(!range_set_lookup(rs, 34, &area, &result));
}

static void test_lookup_single(void *fixture)
{
	struct range_set *rs = fixture;
	struct range area, result;

	area.b = 24;
	area.e = 29;
	T_ASSERT(range_set_add_attribute(rs, 34, &area));

	area.b = 0;
	area.e = 128;
	T_ASSERT(range_set_lookup(rs, 34, &area, &result));
	T_ASSERT_EQUAL(result.b, 24);
	T_ASSERT_EQUAL(result.e, 29);

	T_ASSERT(!range_set_lookup(rs, 33, &area, &result));
}

static void test_lookup_partial(void *fixture)
{
	struct range_set *rs = fixture;
	struct range area, result;

	area.b = 24;
	area.e = 29;
	T_ASSERT(range_set_add_attribute(rs, 34, &area));

	area.b = 16;
	area.e = 26;
	T_ASSERT(range_set_lookup(rs, 34, &area, &result));
	T_ASSERT_EQUAL(result.b, 24);
	T_ASSERT_EQUAL(result.e, 26);

	area.b = 26;
	area.e = 128;
	T_ASSERT(range_set_lookup(rs, 34, &area, &result));
	T_ASSERT_EQUAL(result.b, 26);
	T_ASSERT_EQUAL(result.e, 29);
}

static void test_lookup_multiple_attrs(void *fixture)
{
	struct range_set *rs = fixture;
	struct range area, result;
	unsigned i, count = 128;

	for (i = 1; i < count; i++) {
		area.b = i * 3;
		area.e = i * 4;
		T_ASSERT(range_set_add_attribute(rs, i, &area));
	}

	area.b = 0;
	area.e = count * 10;
	for (i = 1; i < count; i++) {
		T_ASSERT(range_set_lookup(rs, i, &area, &result));
		T_ASSERT_EQUAL(result.b, i * 3);
		T_ASSERT_EQUAL(result.e, i * 4);
	}
}

static void test_multiple_ranges(void *fixture)
{
	struct range_set *rs = fixture;
	struct range area, result;
	unsigned i, count = 128;

	for (i = 0; i < count; i++) {
		area.b = i * 3;
		area.e = i * 3 + 1;
		T_ASSERT(range_set_add_attribute(rs, 34, &area));
	}

	area.b = 0;
	area.e = count * 10;
	for (i = 0; i < count; i++) {
		T_ASSERT(range_set_lookup(rs, 34, &area, &result));
		T_ASSERT_EQUAL(result.b, i * 3);
		T_ASSERT_EQUAL(result.e, i * 3 + 1);

		area.b = result.e;
	}
}

static void test_overlapping_ranges(void *fixture)
{
	struct range_set *rs = fixture;
	struct range area, result;

	area.b = 8;
	area.e = 10;
	T_ASSERT(range_set_add_attribute(rs, 34, &area));

	area.b = 14;
	area.e = 16;
	T_ASSERT(range_set_add_attribute(rs, 34, &area));

	area.b = 9;
	area.e = 15;
	T_ASSERT(range_set_add_attribute(rs, 34, &area));

	area.b = 0;
	area.e = 128;
	T_ASSERT(range_set_lookup(rs, 34, &area, &result));

	// FIXME: makes assumptions about how rs does merge (which is
	// brain dead atm).
	T_ASSERT_EQUAL(result.b, 8);
	T_ASSERT_EQUAL(result.e, 15);

	area.b = 15;
	T_ASSERT(range_set_lookup(rs, 34, &area, &result));
	T_ASSERT_EQUAL(result.b, 15);
	T_ASSERT_EQUAL(result.e, 16);

	area.b = 16;
	T_ASSERT(!range_set_lookup(rs, 34, &area, &result));
}

//----------------------------------------------------------------

#define T(path, desc, fn) register_test(ts, "/base/data-struct/range-set/" path, desc, fn)

void range_set_tests(struct dm_list *all_tests)
{
	struct test_suite *ts = test_suite_create(rs_init, rs_exit);
	if (!ts) {
		fprintf(stderr, "out of memory\n");
		exit(1);
	}

	T("create-destroy", "create and destroy an empty set", test_create_destroy);
	T("lookup-empty", "any lookup in an empty set fails", test_lookup_empty);
	T("lookup-single", "lookup with a single entry succeeds", test_lookup_single);
	T("lookup-partial", "lookup with an overlapping area", test_lookup_partial);
	T("multiple-attrs", "many attributes can be supported", test_lookup_multiple_attrs);
	T("multiple-ranges", "many ranges can be supported", test_multiple_ranges);
	T("overlapping-ranges", "overlapping ranges do the right thing", test_overlapping_ranges);

	dm_list_add(all_tests, &ts->list);
}
//----------------------------------------------------------------
