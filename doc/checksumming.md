# Checksumming

Thinp1 checksums every metadata block as it's read into the block-manager,
and calculates a new checksum whenever it's written out.  This checksum
is combined with a magic nr that identifies the use of that block in
very broad terms (btree-node, space-map-index, superblock).

Over the years this has caught lot's of bugs during development.
And detected issues early with customers (eg, faulty hardware, or
activation on host and guest).

Because the checksum is combined with the block use it's really two
checks.  Which catch separate classes of bug:

## Checksum

The checksum is checked as soon as the block is read into memory, and
recalculated just before it's written out.  It's useful for spotting
external hardware failure, or if the metadata has been stomped on by
a third party process.

I know of only one RH support case where thinp was failing because of
hardware and the checksum spotted it.  Metadata being stomped is more
common, but normally that would trash the block use magic nr too.

The checksum is very expensive to calculate.  Also writebacks are usually
triggered by a commit, so the checksums will all be calculated in a single
thread, and will hold up the whole commit operation.

With large metadata the userland tools (ie. thin_check) can spend a very
long time calculating checksums.


## Block use

This is a 64bit nr that identifies what the metadata block is used for.
It most commonly catches bugs within thinp that occur when the same
block is given two different concurrent uses due to a bug in the metadata
space map reference counting.  It also detects the multiple pool activation
issues that we had.  In addition it's used heavily when trying to repair a
damaged pool.

The cost of this field is negligable.

## Conclusion

I think checksumming should be removed in thinp2.

An alternative would be to add a couple of parity bits to each btree value.

The block use technique should be made more specific about the use of
the block to aid repair.  In addition I think every metadata block should
contain a pool uuid to avoid confusion with metadata accidentally present
on the disk from previous pools.

