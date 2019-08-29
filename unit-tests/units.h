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

#ifndef TEST_UNIT_UNITS_H
#define TEST_UNIT_UNITS_H

#include "framework.h"

//-----------------------------------------------------------------

// Declare the function that adds tests suites here ...
void dm_list_tests(struct dm_list *suites);
void io_manager_tests(struct dm_list *suites);
void io_manager_utils_tests(struct dm_list *suites);
void io_processor_tests(struct dm_list *suites);
void radix_tree_tests(struct dm_list *suites);
void range_set_tests(struct dm_list *suites);

// ... and call it in here.
static inline void register_all_tests(struct dm_list *suites)
{
	io_manager_tests(suites);
	io_manager_utils_tests(suites);
	io_processor_tests(suites);
	dm_list_tests(suites);
	radix_tree_tests(suites);
	range_set_tests(suites);
}

//-----------------------------------------------------------------

#endif
