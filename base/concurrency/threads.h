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

// We often want to specify the stack size for our thread.  The
// actual stack used will always have an extra page that can't be
// written to so we crash hard with a seg fault if we blow the stack 
// rather than scribbling across memory.  It may be worth always linking
// with muscl rather than glibc to minimise stack use.
typedef unsigned stack_size_k;

// Add a thread from outside the scheduler.  Use the spawn() call from
// within a green thread.
typedef void (*gt_work_fn)(void *context);
bool sched_add_thread(struct scheduler *sched, stack_size_k ss,
                      gt_work_fn fn, void *context);

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

// Terminate the current thread.
void gt_term();

// Kick off a new thread.
bool gt_spawn(stack_size_k ss, gt_work_fn fn, void *context);

bool gt_sleep(unsigned milli);

//-------------------------------------------

// Channels allow threads to communicate in a controlled way, by passing a
// chunk of binary data.  There is no buffering; the writer will block
// until the reader receives the value.  This is a synchronisation
// primitive rather than a message queue.

struct channel;
struct channel *chan_create();

// Nobody may be waiting on the channel at this point.  Be careful to
// ensure the lifetime of the chan is longer than that of the threads
// using it.
void chan_destroy(struct channel *chan);

enum chan_op_type {
	CHAN_READ,
	CHAN_WRITE
};

enum chan_result {
	CHAN_E_COMPLETE,
	CHAN_E_INCOMPLETE,
	
	CHAN_E_SIZE_MISMATCH,
	CHAN_E_ALREADY_READER,
	CHAN_E_ALREADY_WRITER,
	CHAN_E_CLOSED
};

struct chan_op {
	struct channel *chan;
	enum chan_op_type type;
	void *data;
	size_t len;

	enum chan_result res;
};

// Returns true if the exchange completed, otherwise examine
// op->res for details.
bool chan_submit(struct chan_op *op);

// Sometimes we want to use multiple channels concurrently.  This
// is more efficient than iterating across ops and calling submit, since
// it knows which channels have threads waiting at the other end.
// Returns true if at least one of the ops completed.
bool chan_submit_multiple(struct chan_op **ops, unsigned count);

// Wrappers around chan_submit()
bool chan_write(struct channel *chan, void const *data, size_t len);
bool chan_read(struct channel *chan, void *data, size_t len);

// Any waiter on the channel will have their chan_submit() call fail.
void chan_close(struct channel *chan);

//-------------------------------------------

#endif

