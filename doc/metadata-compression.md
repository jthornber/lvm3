# Metadata compression

One of the bigger issues we have is with the size of the mapping trees.  These
are currently held as BTrees that map:

	virtual block (u64) -> origin block (u48), time stamp (u16)

Pages of these btrees are loaded into a cache on demand.  Obviously performance
tanks if those pages are not in memory when we need to do a lookup.

Some of our customers are hitting the 16GB metadata limit.

## Run length encoding

Estimated 10x metadata compression.

Allow us to remove the 'data chunk size' pool parameter, effectively
always using a 4k chunk size.  This is a big win because atm the user
is forced to choose between a chunk size suitable for snapshots, or
suitable for thin provisioning quickly.

## BTree node compression

Assuming run length encoding is used, our btrees need to store a
mapping:

	virtual block (u64) -> origin_block (u64), len (u64), timestamp (u16)

Using the techniques discussed below I estimate thinp2 btrees will
take up a 40th of the space of thinp1.

### Too much node splitting

In thinp1 a btree node can hold up to ~250 entries.  The ratio of actual
entries / max entries contained in a node is known as it's residency.
Keeping the average residency high for all the leaves in a btree is
essential.

The code splits a node as it becomes full.  There are common IO patterns
that can lead to this splitting being the final thing that happens to
a node, leading to an average residency of only 50%.  This is very common
in thinp1.

See *batch insertion* for more info.

 
### Internal nodes

  - Common prefix.  If all the keys in an internal node share a common prefix
    we can record that prefix once, and remove it from the front of all the
    keys.

  - Significant prefix.  If the decision of which child node to select can be
    done by looking at a prefix of the keys only, then we only need to store
    that prefix.

eg,  Say we have the following keys in an internal node:

    > 0x123a10303945
    > 0x123a99845300
    > 0x123b90293842
    > 0x123f09384354

Then we can store 0x123 in the header (common prefix), and just use the
first two bytes of the remainder as a significant prefix.

So the keys become:

    > 0xa1
    > 0xa9
    > 0xb9
    > 0xf0

  - Per node Variable value sizes

    An internal node points to a metadata block.  If we make this a
    u32 our maximum metadata device size is 2^44 bytes.  Which should
    be plenty.  Often a full u32 is overkill though, so we could have a
    flag in node header to drop the value size to u16.  This optimisation
    would be more frequently useful if we change the metadata allocators
    to keep metadata for a thin device close together.  If a value was
    inserted that didn't fit into the nodes current value size, then
    the node would be forced to split, and one of the new nodes to have
    a larger value size.

So for a u16 value type, we'd expect to hold:

	(4096 - header_size) / (significant_prefix + sizeof(u16))

entries.  eg, header_size = 128, significant_prefix = 2 => 992.  8 times what we get 
in thinp1 metadata.

With compression there will be very few internal nodes compared to leaf
nodes, so we should consider preloading and locking them in memory.


### Leaf nodes

For leaf nodes the value are larger (~18 bytes), potentially leading to
as few as 188 per node.  These are range entries, so we need to store
far fewer of them than thinp1.  But we would still like to pack many
more entries in.

   - Common prefix.  As for internal nodes.

   - Variable value sizes.  If we introduce another level indirection we
     can support variable value sizes.  eg,

     Instead of our node looking like:
 
     >  +--------+------+--------+
     >  | header | keys | values |
     >  +--------+------+--------+

     It would be:

     >  +--------+------+------------+--------+
     >  | header | keys | value ptrs | values |
     >  +--------+------+------------+--------+

    The value ptrs need to address fewer than 4096 bytes, so we'd probably
    use a u16, and use extra bits to encode the type of the value.
    Example value types:

       ORIGIN64_LEN32_TIME8
       ORIGIN32_LEN16_TIME0
       ... etc.

    Another way of encoding the values would be to put a reference value
    for the origin, len and time into the node header, and then store
    relative values.

    Close proximity of data blocks would allow smaller value types to
    be used.  So we need to improve our data allocators.

    I'd expect ORIGIN32_LEN16_TIME0 to be a common value size, assuming
    u16 for the value-ptrs, and a key length of 3 bytes.  That would
    give us a maximum of 360 entries.  So a doubling of density at the
    expense of a lot of code complexity.  I don't think this is worth it,
    we'd be better off using per node variable value sizes and losing
    the value-ptr array.

   - Sparse value entries.  To make variable value entries pay off you
     need to use big enough values so the overhead of the value ptr is
     insignificant compared to the space saving.  So how can we make our
     values bigger?  We're already storing ranges, effectively multiple
     entries.  How about if we store multiple ranges in a value?  This
     would add cpu overhead as a linear search would be needed through
     the value.  Probably not worth it; lookup performance is king.

