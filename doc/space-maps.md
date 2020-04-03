# Space maps

[assuming we switch to journalled transactions]

These hold reference counts for blocks, and act as allocators.

## Metadata space map

A 16Gb metadata area is 2^22 4k pages (~4 million).  At 32bits per
ref count this is just 16MB of disk space/ram.

Most of the reference counts will be <4.  So there is scope for
compression down to ~1MB of disk space/ram.  There's no locality
between pages, so run length encoding is unlikely to be useful.

Probably best to stick with the current scheme of 2 bit counts in an
array, and a sparse btree for the higher counts.


## Data space map

This needs to be much smarter than the space map in thinp1, since
it needs to:

  - Allocate ranges.
  - Allow thins to allocate near existing mappings (locality).

A form of buddy allocator seems best.  Different thins could be
assigned large chunks to carve up as they wish.

FIXME: read XFS allocator code.
FIXME: finish

