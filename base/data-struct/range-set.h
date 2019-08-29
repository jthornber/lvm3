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
 
#ifndef BASE_DATA_STRUCT_RANGE_SET_H
#define BASE_DATA_STRUCT_RANGE_SET_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

//----------------------------------------------------------------

// The range set allows you to assign attributes to a range of values.
// For instance READ/WRITE properties to parts of a disk.

// e is the one-past-the-end value as usual.  ie. it is *not* in the range.
struct range {
	uint64_t b, e;
};

struct range_set;
struct range_set *range_set_create(void);
void range_set_destroy(struct range_set *rs);

bool range_set_add_attribute(struct range_set *rs, unsigned attr, struct range *r);

// This returns the *first* range in 'area'; you may need to perform
// multiple lookups to cover all of 'area'.
bool range_set_lookup(struct range_set *rs, unsigned attr,
                      struct range *area, struct range *result); 

//----------------------------------------------------------------

#endif
