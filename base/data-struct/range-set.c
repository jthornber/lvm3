#include "base/data-struct/range-set.h"

#include "base/misc/util.h"
#include "base/memory/zalloc.h"

//----------------------------------------------------------------

// FIXME: this implementation is likely to degenerate to a linked list
// because people will commonly add ranges in order.  Switch to a balanced
// tree iff we are getting non trivial numbers of entries.

// We store the ranges in a 2 level binary tree.  With the first
// level discriminating on attribute, and second on range.
struct node {
	struct node *left;
	struct node *right;

	// Node either holds an attribute or a range
	union {
		struct range r;
		struct {
			unsigned attr;
			struct node *ranges;
		} attr;
	} val;
};

static bool _lookup_bottom_level(struct node *n, struct range *area, struct range *result)
{
	if (!n)
		return false;

	if (area->e < n->val.r.b)
		return _lookup_bottom_level(n->left, area, result);

	if (area->b >= n->val.r.e)
		return _lookup_bottom_level(n->right, area, result);

	result->b = max(area->b, n->val.r.b);
	result->e = min(area->e, n->val.r.e);
	return true;
}

static bool _lookup_top_level(struct node *n, unsigned attr, struct range *area,
	                      struct range *result)
{
	if (!n)
		return false;

	if (attr < n->val.attr.attr)
		return _lookup_top_level(n->left, attr, area, result);

	if (attr > n->val.attr.attr)
		return _lookup_top_level(n->right, attr, area, result);

	return _lookup_bottom_level(n->val.attr.ranges, area, result);
}

static void _merge_ranges_left(struct range *lhs, struct range *rhs)
{
	lhs->b = min(lhs->b, rhs->b);
	lhs->e = max(lhs->e, rhs->e);
}

static bool _insert_bottom_level(struct node **n, struct range *area)
{
	if (!(*n)) {
		*n = zalloc(sizeof(**n));
		(*n)->val.r.b = area->b;
		(*n)->val.r.e = area->e;
		return true;
	}

	if (area->e < (*n)->val.r.b)
		return _insert_bottom_level(&(*n)->left, area);

	if (area->b >= (*n)->val.r.e)
		return _insert_bottom_level(&(*n)->right, area);

	// FIXME: handle merging ranges
	// I think this works, but can result in a badly formed tree with
	// the range extended past the start of the next entry.  We get 
	// away with it because we never remove ranges.
	_merge_ranges_left(&(*n)->val.r, area);
	return true;
}

static bool _insert_top_level(struct node **n, unsigned attr, struct range *area)
{
	if (!(*n)) {
		*n = zalloc(sizeof(**n));
		(*n)->val.attr.attr = attr;
		return _insert_bottom_level(&((*n)->val.attr.ranges), area);
	}

	if (attr < (*n)->val.attr.attr)
		return _insert_top_level(&(*n)->left, attr, area);

	if (attr > (*n)->val.attr.attr)
		return _insert_top_level(&(*n)->right, attr, area);

	return _insert_bottom_level(&(*n)->val.attr.ranges, area);
}

static void _free_bottom_level_nodes(struct node *n)
{
	if (n) {
		_free_bottom_level_nodes(n->left);
		_free_bottom_level_nodes(n->right);
		free(n);
	}
}

static void _free_top_level_nodes(struct node *n)
{
	if (n) {
		_free_top_level_nodes(n->left);
		_free_bottom_level_nodes(n->val.attr.ranges);
		_free_top_level_nodes(n->right);
		free(n);
	}
}

//----------------------------------------------------------------

struct range_set {
	struct node *root;
};

struct range_set *range_set_create(void)
{
	return zalloc(sizeof(struct range_set));
}

void range_set_destroy(struct range_set *rs)
{
	_free_top_level_nodes(rs->root);
	free(rs);
}

bool range_set_add_attribute(struct range_set *rs, unsigned attr, struct range *r)
{
	if (r->b >= r->e)
		// noop
		return true;

	return _insert_top_level(&rs->root, attr, r);
}

bool range_set_lookup(struct range_set *rs, unsigned attr,
                      struct range *area, struct range *result)
{
	return _lookup_top_level(rs->root, attr, area, result);
}

//----------------------------------------------------------------
