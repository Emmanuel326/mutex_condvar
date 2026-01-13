# Bounded Queue using pthread mutex + condition variables

A minimal bounded work queue implemented using `pthread_mutex_t` and
`pthread_cond_t`.

The goal of this implementation is correctness, clarity, and well-defined
blocking semantics under concurrency.

---

## Design Overview

- A single mutex protects all shared queue state (`size`, `head`, `tail`, buffer)
- Two condition variables are used:
  - `not_full`: producers wait when the queue is full
  - `not_empty`: consumers wait when the queue is empty
- All condition waits are performed in `while` loops to correctly handle
  spurious wakeups

The queue stores opaque pointers (`void *`) and does not manage the lifetime
of enqueued objects.

---

## Invariants

The following invariants are maintained at all times, including under
concurrent access:

### Structural
- `0 <= size <= capacity`
- `head` and `tail` indices remain within `[0, capacity)`
- Queue storage is never accessed out of bounds

### Behavioral
- Enqueue blocks iff the queue is full
- Dequeue blocks iff the queue is empty
- Items are dequeued in FIFO order
- No item is lost or duplicated

### Concurrency & Visibility
- All queue mutations are mutually exclusive
- Memory written by a producer before enqueue is visible to the consumer
  after dequeue (via mutex + condition variable synchronization)

### Progress Guarantees
- No deadlock, assuming producers and consumers continue to make progress
- A dequeue unblocks at least one producer when the queue was full
- An enqueue unblocks at least one consumer when the queue was empty
- No fairness or starvation guarantees beyond those provided by the OS scheduler

---

## Guarantees

- FIFO ordering
- Blocking enqueue when full
- Blocking dequeue when empty
- No busy waiting

---

## Tradeoffs

- Simple and easy to reason about
- Threads may sleep, introducing wakeup latency under contention
- Throughput and fairness depend on the OS scheduler

---

## Testing & Verification

This implementation has been tested under high contention and dynamic
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

