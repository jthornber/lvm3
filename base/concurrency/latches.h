#ifndef BASE_CONCURRENCY_LATCHES_H
#define BASE_CONCURRENCY_LATCHES_H

#include <pthread.h>

//-------------------------------------------

// This project uses terminology from the database implementation
// world, since we use many of the same techniques to implement btrees
// and transactionality.  'latches' are the traditional fine grain
// synchronisation primitives such as mutexes and semaphores.  You should
// hold them for as short a time as possible.  They are quick to acquire
// as long as there's no contention.  Whereas 'locks' are much higher
// constructs that will be held for long periods, and may well take time
// to acquire.

// FIXME: wrap pthread to provide some extra debugging.  eg, we can check
// if any latches are held when a green thread blocks/yields.  For now
// just use the pthreads api.

//-------------------------------------------

#endif

