#ifndef CONCURRENCY_THREADS_H
#define CONCURRENCY_THREADS_H

#include "base/data-struct/list.h"

#include <stdbool.h>

//-------------------------------------------

// The scheduler contains a set of green threads.  Those green threads
// share a single OS thread between them.  If you want to use more than
// one core, you need to use multiple schedulers.
struct scheduler;

struct scheduler *sched_create();
void sched_destroy(struct scheduler *sched);

// Starts the scheduler using a *new* OS thread.
bool sched_start(struct scheduler *sched);

// Stops the scheduler, any running green threads will be
// paused.
bool sched_stop(struct scheduler *sched);

// Only call this if the scheduler is *not* running.  Otherwise
// use the spawn() call from within a green thread. 
typedef void (*gt_work_fn)(void *context);
bool sched_add_thread(struct scheduler *sched, gt_work_fn fn, void *context);

// Waits for all threads in a scheduler to complete.
void sched_wait(struct scheduler *sched);

// FIXME: we've got no way of killing or interrupting individual
// threads atm.

//-------------------------------------------

// These methods are all for use from within a green thread (where
// the surrounding scheduler context is implicit).

// Because the threads all share a single OS thread you do not need
// to protect critical sections with mutexes.  But you must not
// make a call that could block/yield within a critical section.
// eg, gt_yield(), gt_sleep() or any of the channel ops.

// Call this regularly from cpu intensive code to share the OS
// thread amoung the other green threads.
void gt_yield();

// Kick off a new thread.
bool gt_spawn(gt_work_fn fn, void *context);

bool gt_sleep(unsigned milli);

//-------------------------------------------

// Channels allow threads to communicate in a controlled way.  Similar to
// a pipe they allow one thread to write a value, and another to read it.
// There is no buffering; the writer will block until the reader receives
// the value.

struct channel;
struct channel *chan_create();

// Closes both ends of the channel and then frees the channel.
void chan_destroy(struct channel *chan);

enum chan_op_type {
	CHAN_READ,
	CHAN_WRITE
};

struct chan_op {
	struct channel *chan;
	enum chan_op_type type;
	void *data;
	size_t len;
};

bool chan_submit(struct chan_op *op);

// Wrappers around chan_submit()
bool chan_write(struct channel *chan, void const *data, size_t len);
bool chan_read(struct channel *chan, void *data, size_t len);

// Sometimes we want to use multiple channels concurrently.
bool chan_submit_multiple(struct chan_op **ops, unsigned count);

// closing the write end of the channel causes chan_read to fail.
void chan_close_write(struct channel *chan); 

// closing the read end of the channel causes chan_write to fail.
void chan_close_read(struct channel *chan); 

//-------------------------------------------

#endif

