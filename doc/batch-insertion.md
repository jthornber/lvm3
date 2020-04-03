*** Batch insertion

Batch inserting entries into a btree can result in much better btrees
than when you insert in random order (or whatever your IO pattern is).
The residency can be maxed out, and you can make better decisions
about how to compress individual nodes.  Think of this as defragging
your metadata.

So there would be a benefit to periodically rebuilding the btrees.
The smaller the metadata the quicker this rebuild process will be.

If we know that we're periodically rebuilding, then we can start
to relax some rules and let the btrees get into a more messy state.
For instance, the btree_remove() code is probably the most complicated
bit of thinp; it never let's a btree node have fewer than 50% residency,
instead merging entries with its immediate neighbours.  Over the years
we've had 2 or 3 issues in this code.  If we're periodically rebuilding
we should drop the minimum 50% residency and have the remove function
do no rebalancing and only delete the node when it is completely empty.


