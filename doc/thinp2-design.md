Design notes for thin provisioning v2


# Thinp1 issues

## Pool wide commit.

  - Commit stops the world whilst superblock is written.
  - FLUSH/FUA triggers commit.  The more thins you have active the more 
    frequent these are.

## Big read/write semaphore.

  - provisioning or breaking sharing holds a pool wide write lock, and so
    delays normal IO on all active thins.

## Nested data structures all have common root

  superblock -> dev tree -> mappings
             |
             +> device details

  So there's contention.


## Metadata sharing.

This serves two purposes.

  i) Allowing us to take very fast snapshots and save duplicating metadata
     for identical mappings.

  ii) Provide atomicity by having all metadata be a child of the superblock.

We should keep (i), but (ii) has drawbacks as discussed above.  Switching
to a journal based transactional model does not preclude using metadata
sharing.




* Thinp 2

We must keep metadata sharing at the mapping tree level.  Sharing of
device details btree and the top level 'metadata snapshot' will go.
Instead we'll provide a way of querying the root of an inactive
mapping tree.  So long as it's inactive (eg, because it's a snapshot)
the metadata can be safely read from userland.

Switch to journalling transactions.

Compress mapping btrees.

Separate device detail btrees, and mapping btrees.  This will simplify
the btree code since it will no longer need to support recursively
nested trees.

Metadata space map does not need to be shared.  Journal changes.  On disk
structure can be a simple array.

We need a new space map/allocator for data blocks that can provide
locality and allocate ranges, and keep reference counts.  Again, this
doesn't need to be shared.  Journal changes.

So the on disk data structures are:

  - Superblock.  No sharing.  Possibly need to add space for query results.
  - Journal.
  - Metadata space map, no sharing.  Ref counts for fixed sized blocks.
  - Data space map.  No sharing.
  - Device details tree.  No sharing.  No compression needed.
  - Mapping trees.  Sharing.  Compression tricks.


