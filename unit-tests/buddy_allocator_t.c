
/*
 * Copyright (C) 2018 Red Hat, Inc. All rights reserved.
 *
 * This file is part of LVM2.
 *
 * This copyrighted material is made available to anyone wishing to use,
 * modify, copy, or redistribute it subject to the terms and conditions
 * of the GNU General Public License v.2.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "base/memory/buddy-allocator.h"

#include "framework.h"
#include "units.h"

//----------------------------------------------------------------

static void test_create(void *fixture)
{
	struct buddy_allocator *ba = create_buddy_allocator(4096, 0);
	T_ASSERT(ba);
	destroy_buddy_allocator(ba);
}

//----------------------------------------------------------------

#define T(path, desc, fn) register_test(ts, "/base/device/io-manager/core/" path, desc, fn)

static struct test_suite *_no_fixture_tests(void)
{
	struct test_suite *ts = test_suite_create(NULL, NULL);
	if (!ts) {
		fprintf(stderr, "out of memory\n");
		exit(1);
	}

	T("create-destroy", "simple create/destroy", test_create);

	return ts;
}

void buddy_allocator_tests(struct dm_list *all_tests)
{
        dm_list_add(all_tests, &_no_fixture_tests()->list);
}

//----------------------------------------------------------------
