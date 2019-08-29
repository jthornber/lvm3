#ifndef IO_IO_PARALLEL_H
#define IO_IO_PARALLEL_H

#include "io/io-manager.h"

//----------------------------------------------------------------

// io-manager utility that let you run a common task on many
// io-manager blocks in parallel.  This doesn't use multiple threads
// but it does take care to prefetch data in parallel, so you will
// get a big speed up over a simple get/process/put loop.

struct io_processor;
typedef void (*io_task_fn)(void *context, void *data, uint64_t len);
typedef void (*io_error_fn)(void *context);


struct io_processor *io_processor_create(struct io_manager *iom,
                                         io_task_fn t, io_error_fn err);
void io_processor_destroy(struct io_processor *iop);

// path is copied. start and len are in bytes.
bool io_processor_add(struct io_processor *iop, const char *path, uint64_t start,
                      uint64_t len, void *context);
void io_processor_exec(struct io_processor *iop);


//----------------------------------------------------------------
// For unit testing
 
struct processor_ops {
	void (*destroy)(struct processor_ops *ops);
	unsigned (*batch_size)(struct processor_ops *ops);
	void *(*get_dev)(struct processor_ops *ops, const char *path, unsigned flags);
	void (*put_dev)(struct processor_ops *ops, void *dev);
	// returns the number of blocks covered
	unsigned (*prefetch_bytes)(struct processor_ops *ops, void *dev, uint64_t start, size_t len);
	bool (*read_bytes)(struct processor_ops *ops, void *dev, uint64_t start, size_t len, void *data);
};

struct io_processor *io_processor_create_internal(struct processor_ops *ops,
                                                  io_task_fn t, io_error_fn err);

//----------------------------------------------------------------

#endif
