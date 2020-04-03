# Transaction model

## Persistent data structures

When thinp1 takes a snapshot it just bumps the reference count for
the root of the origin mapping tree, and the new snapshot shares
the metadata.

Pros: 

  - Provides transactionality without the need for a journal.

  - Snapshots are extremely quick to take, since the mapping tree is 100%
    shared with the origin at the start.

  - Snapshots are extremely space efficient.

  - Userland can access a metadata snapshot which allows it to do things like
    reading mappings of inactive thin volumes for backup.

Cons:

  - It's harder to work out which metadata blocks are allocated.  Thinp1
    keeps reference counts in the metadata space map.  This data
    structure is a bit difficult to reason about since it is itself
    stored in metadata, so updating the space map can cause the space
    map to be updated.  In practise we've not seen long lived loops;
    but the metadata sm is complicated code that I'd gladly lose.
    It's also the source of the 16G metadata limit.

    An alternative I've considered for thinp2 is to effectively garbage
    collect the metadata blocks.  That way we'd only need to keep track
    of whether an md block has been allocated or not, and the sweeper
    would return blocks that were no longer used to the free set.

  - Corruption of metadata can have very large repercussions to a whole
    family of thin devices.

  - More metadata to write per transaction (because copy-on-write).

  - Only one transaction can be open at once.  Updates to multiple
    thins all go into this single transaction.

  - Thins periodically need to commit before they can continue
    (REQ_FUA/REQ_FLUSH/REQ_DISCARD).  The more thin volumes active
    the more frequent these commits are.

  - Commits stop the world.  All IO is paused while:
    - Checksums are calculated for all dirty metadata.
    - All dirty metadata is synchronously written.
    - The new superblock is synchronously written.


I don't think we should continue with this transactional model in 
thinp2. 



## Database transaction model

Databases use a journal to provide transactionality for their btrees.

This journal records multiple, concurrent transactions.  For each
transaction it records:

  - Logical level locks.  eg, lock a mapping range for a particular thin.
    (this is equivalent to the bio_prison locks)
  - Page locks.  eg, lock page 12345.
  - *Reversible* details for each page update.  eg, Remove key/value,
    update key/value -> key/value, Insert key/value.  Page sequence
    numbers must be used to aid recovery.

Dirty pages must not be written until after the journal containing their
changes has hit the disk.  When a journal page is written it should include
a 'checkpoint' entry that delimits how much of the log is being committed.
This allow other transactions to continue writing to the log even during IO.

Once the journal write completes all locks held by complete transactions
in it can be dropped.  Dropping a page lock allows writeback to occur on
it.

Once dirty pages are written back older journal entries are no longer
neccessary.  So the journal acts as a ring buffer.

If there's a crash then the journal needs to be replayed up to the last 
checkpoint.  Page sequence numbers should be compared to ascertain which
page changes managed to hit the disk.

With this scheme a transaction commit requires waiting for the journal
to be written.  But other transactions are not held up.




