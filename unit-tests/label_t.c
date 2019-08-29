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

#include "label.h"

#include "framework.h"
#include "units.h"

//----------------------------------------------------------------

struct fixture {

};

static void *_fix_init(void)
{
        struct fixture *f = malloc(sizeof(*f));
        return f;
}

static void _fix_exit(void *fixture)
{
	free(fixture);
}

//----------------------------------------------------------------

static void _test_no_test(void *fixture)
{

}

//----------------------------------------------------------------

static struct test_suite *_tests(void)
{
        struct test_suite *ts = test_suite_create(_fix_init, _fix_exit);
        if (!ts) {
                fprintf(stderr, "out of memory\n");
                exit(1);
        };

#define T(path, desc, fn) register_test(ts, "/metadata/label/" path, desc, fn)
	T("empty-test", "do nothing test", _test_no_test);
#undef T

	return ts;
}

void label_tests(struct dm_list *all_tests)
{
	dm_list_add(all_tests, &_tests()->list);
}

//----------------------------------------------------------------
