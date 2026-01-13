#pragma once

#include <stddef.h>



typedef struct bounded_queue bounded_queue_t;


/*
 * Initialize a bounded queue with fixed capacity.
 *
 * Returns:
 *  0 on success
 * -1 on failure (e.g., invalid capacity, allocation failure)
 */
int bq_init(bounded_queue_t *q, size_t capacity);

/*
 * Destroy the queue and free internal resources.
 *
 * Undefined behavior if called while other threads
 * are blocked in enqueue or dequeue.
 */
void bq_destroy(bounded_queue_t *q);

/*
 * Enqueue an item into the queue.
 * Undefined behavior if called while other threads
 * are blocked in enqueue or dequeue.

 * Blocks if the queue is full.
 * The caller must ensure all producer and consumer threads
 * have terminated or stopped using the queue before destruction.

 *
 * Returns:
 *  0 on success
 * -1 on failure (future extension)
 */
int bq_enqueue(bounded_queue_t *q, void *item);

/*
 * Dequeue an item from the queue.
 *
 * Blocks if the queue is empty.
 * The dequeued item is stored in *item.
 *
 * Returns:
 *  0 on success
 * -1 on failure (future extension)
 */
int bq_dequeue(bounded_queue_t *q, void **item);

/*
 * Return the current number of items in the queue.
 * Intended for debugging and testing.
 */
size_t bq_size(bounded_queue_t *q);

/*
 * Return the fixed capacity of the queue.
 * The returned value may be immediately outdated if called concurrently.
 */
size_t bq_capacity(bounded_queue_t *q);

//int bq_close(bounded_queue *q);

bounded_queue_t* bq_create(size_t capacity);

