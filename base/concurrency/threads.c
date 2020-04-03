#include "base/concurrency/threads.h"

#include "base/math/rounding.h"
#include "base/data-struct/list.h"
#include "base/concurrency/latches.h"

#include <stdlib.h>
#include <stdbool.h>
#include <setjmp.h>

//-------------------------------------------
// Stack allocation
 
struct stack {
	void *mem;
	void *stack_start;
};

static inline size_t page_size_()
{
	static long ps = 0;

	if (!ps)
		ps = sysconf(_SC_PAGE_SIZE);
	
	return (size_t) ps;
}

static bool alloc_stack_(stack_size_k ss, struct stack *s)
{
	ss *= 1024;
	ss = round_up(ss, page_size_());
	ss += page_size_();

	if (posix_memalign(&s->mem, page_size_(), ss))
		return false;

	if (mprotect(s->mem, page_size_(), PROT_NONE)) {
		free(s->mem);
		return false;
	}

	// FIXME: Assumes the stack grows downwards, we should
	// catch architectures where this isn't the case.
	s->stack = s->mem + ss;
	return true;
}

static void free_stack_(struct stack *s)
{
	mprotect(s->mem, page_size_(), PROT_READ | PROT_WRITE);
	free(s->mem);
}

//-------------------------------------------
// Events

// Tasks need to wait for events, sometimes more than one at once. eg,
// a timeout and an IO, or multiple channels.
// 
// It's plausible for multiple tasks to be waiting for the same event
// (eg, IO).
//
// When an event triggers it will wake all waiters.  What the event was,
// and any error state has to be inferred from the wait_fn callback and
// the surrounding context of the waiter struct.

struct event {
	struct list waiters;
};

struct waiter;
typedef void (*wait_fn)(struct waiter *w);

// Waiter should be part of a larger struct that contains
// the context for what's being waited upon.
struct waiter {
	struct list list;
	wait_fn wake;
};

void event_init(struct event *e)
{
	list_init(&e->waiters);
}

void event_wake_all_(struct event *e)
{
	struct waiter *w, *tmp;
	list_iterate_items_safe (w, tmp, &e->waiters) {
		w->wake(w);
		list_del(&w->list);
	}
}

	 
void event_add_waiter_(struct event *e, struct waiter *w)
{
	list_add_tail(&e->waiters, &w->list);
}

// FIXME: Have to be careful of races here. eg, IO will almost certainly
// involve a separate OS thread.
void event_remove_waiter_(struct event *e, struct waiter *w)
{
	list_del(&w->list);
}

//-------------------------------------------
// Scheduler

// Threads can block on a subsystem.  The scheduler needs to
// poll regularly to see if they can be woken.
struct pollable {
	void *(*poll)(struct pollable *pbl);

};

struct scheduler {
	struct task *current_task;
	struct list running;
	struct list blocked;
}

enum task_state {
	T_RUNNING,
	T_BLOCKED
};

struct task {
	struct scheduler *sched;
	
	struct list list;
	struct stack stack;
	jmp_buf jb;
};

struct scheduler *sched_create()
{

}

void sched_destroy(struct scheduler *sched)
{

}

static int launch()
{

}

bool sched_start(struct scheduler *sched)
{
	// pthread create
}

bool sched_stop(struct scheduler *sched)
{

}

bool sched_add_thread(struct scheduler *sched, stack_size_k ss,
                      gt_work_fn fn, void *context)
{

}

void sched_wait(struct scheduler *sched)
{

}

//-------------------------------------------
// Thread fns

static bool poll_due_(struct scheduler *sched)
{

}

static void poll_subsystems_(struct scheduler *sched)
{
	// FIXME: finish

}

static struct task *next_runnable_(struct scheduler *sched)
{
	// FIXME: periodically poll *all* subsystems that
	// tasks are blocked against.
	if (list_empty(&sched->runnable) || poll_due_(sched))
		poll_subsystems_(sched);

	if (list_empty(&sched->runnable))
		return NULL;

	else {
		struct list *n = list_first(&t->sched->running);
		struct task *nt = container_of(n, struct task, list);
		list_del(&nt->list);
		return nt;
	}
}

// FIXME: can we break this into separate suspend/resume fns?
static struct task *context_switch__(struct task *old_t, struct task *new_t)
{
	if (_setjmp(old_t->jb)) {
		// ... and we're back :)
		return old_t;

	} else {
		// jump to new thread ... 
		_longjmp(new_t->jb);
		return new_t;
	}
}

static void context_switch_(struct scheduler *sched, struct task *new_t)
{
	if (sched->current_task != new_t) {
		struct task *old_t = sched->current_task;
		struct task *scheduled = context_switch__(old_t, new_t);
		
		list_add(&t->sched->runnable, &t->list);
		sched->current_task_ = scheduled;
	}
}

void gt_yield()
{
	struct task *t = current_task_, *n;

	n = next_runnable_(t->sched);
	if (n)
		context_switch_(t, n);
}

void gt_term()
{

}

bool gt_spawn(stack_size_k ss, gt_work_fn fn, void *context)
{

}

bool gt_sleep(unsigned milli)
{

}

//-------------------------------------------
// Channels

struct channel {
	struct chan_op *writer;
	struct chan_op *reader;
};

struct channel *chan_create()
{
	return zalloc(sizeof(struct channel));
}

void chan_destroy(struct channel *chan)
{
	assert(no_waiters_(&chan->ev));
	free(chan);
}

// Both reader and writer must be present.
static bool chan_complete__(struct channel *chan)
{
	if (chan->reader->len != chan->writer->len)
		return false;

	memcpy(chan->reader->data, chan->writer->data, chan->reader->len);
	chan->reader = chan->writer = NULL;

	return true;
}

static bool chan_complete_(struct channel *chan)
{
	bool r = chan_complete__(chan);
	event_wake_all_(&chan->ev);
	return r;
}

static bool wait_for_complete_(struct channel *chan)
{
	wait_on_event_(&chan->ev);
	return !chan->closed;
}

bool chan_submit(struct chan_op *op)
{
	struct chan *c = op->chan;

	switch (op->type) {
	case CHAN_READ:
		if (chan->reader)
			// already a reader
			return false;
		chan->reader = op;

		return chan->writer ? chan_complete_(chan) : wait_for_complete_(chan);

	case CHAN_WRITE:
		if (chan->writer)
			// already a writer
			return false;
		chan->writer = op;

		return chan->reader ? chan_complete_(chan) : wait_for_complete_(chan);
	}

	return true;
}

bool chan_submit_multiple(struct chan_op **ops, unsigned count)
{

}

bool chan_write(struct channel *chan, void const *data, size_t len)
{

}

bool chan_read(struct channel *chan, void *data, size_t len)
{

}

//-------------------------------------------

