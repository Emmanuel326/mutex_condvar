# Bounded Queue using pthread mutex + condition variables

A minimal bounded work queue implemented with `pthread_mutex_t` and
`pthread_cond_t`.

This implementation prioritizes **correctness, clarity, and well-defined
blocking semantics under concurrency**, rather than maximum throughput or
lock-free behavior.

---

## Overview

The queue provides a classic producer–consumer abstraction with bounded
capacity and blocking semantics.

- Producers block when the queue is full
- Consumers block when the queue is empty
- Items are dequeued in FIFO order
- No busy waiting

The queue stores opaque pointers (`void *`) and does **not** manage object
lifetime; memory ownership remains with the caller.

---

## Design

- A single mutex protects all shared queue state
  (`size`, `head`, `tail`, and the backing buffer)
- Two condition variables are used:
  - `not_full`: producers wait while the queue is full
  - `not_empty`: consumers wait while the queue is empty
- All condition waits are performed in `while` loops to correctly handle
  spurious wakeups

This design favors simplicity and ease of reasoning over aggressive
parallelism.

---

## Invariants

The following invariants are maintained at all times, including under
concurrent access.

### Structural
- `0 <= size <= capacity`
- `head` and `tail` remain within `[0, capacity)`
- Queue storage is never accessed out of bounds

### Behavioral
- Enqueue blocks if and only if the queue is full
- Dequeue blocks if and only if the queue is empty
- Items are dequeued in FIFO order
- No item is lost or duplicated

### Concurrency & Visibility
- All queue mutations are mutually exclusive
- Memory written by a producer before enqueue is visible to the consumer
  after dequeue (via mutex and condition variable synchronization)

### Progress & Liveness
- No deadlock, assuming producers and consumers continue to make progress
- A dequeue unblocks at least one producer when the queue was full
- An enqueue unblocks at least one consumer when the queue was empty
- No fairness or starvation guarantees beyond those provided by the OS scheduler

---

## Tradeoffs

- Straightforward and easy to reason about
- Threads may sleep, introducing wakeup latency under contention
- Throughput and fairness depend on the OS scheduler

This implementation is intended as a **correct baseline**, not a
high-performance or lock-free queue.

---

## Testing & Verification

The queue has been exercised under contention and checked with dynamic
analysis tools:

- **Functional Tests**
  - Single-threaded tests verify FIFO ordering and ring-buffer wrap-around
- **Stress Test**
  - 4 producers, 4 consumers, 40,000 items
  - Sum-check verification ensures zero data loss or duplication
- **Dynamic Analysis**
  - **ASan + UBSan**: no memory errors or undefined behavior detected
  - **TSan**: no data races detected under concurrent execution

---

## Building & Running Tests

```sh
clang -Wall -Wextra -I. -I./common \
    bounded_queue.c tests/test_stress.c \
    -lpthread

