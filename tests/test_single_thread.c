#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "../bounded_queue.h"

int main() {
    printf("--- Starting Single Thread Tests ---\n");

    size_t cap = 3;
    bounded_queue_t *q = malloc(256); // Using a buffer size to be safe since struct is hidden
    
    // Invariant 1: Init state
    printf("[1/4] Testing Initialization...\n");
    assert(bq_init(q, cap) == 0);
    assert(bq_size(q) == 0);
    assert(bq_capacity(q) == cap);

    // Invariant 6 & 15: FIFO and Opaque Pointers
    printf("[2/4] Testing Saturation and FIFO...\n");
    char *msg1 = "Work A";
    char *msg2 = "Work B";
    char *msg3 = "Work C";

    bq_enqueue(q, msg1);
    bq_enqueue(q, msg2);
    bq_enqueue(q, msg3);
    assert(bq_size(q) == 3);

    void *out;
    bq_dequeue(q, &out); assert(out == msg1);
    bq_dequeue(q, &out); assert(out == msg2);
    bq_dequeue(q, &out); assert(out == msg3);
    assert(bq_size(q) == 0);

    // Invariant 2 & 3: Circular Indices (Wrap-around)
    printf("[3/4] Testing Circular Wrap-around...\n");
    // After the previous dequeue, head and tail are both at index 0 (or wherever they landed)
    // We do multiple cycles to force the pointers to wrap over the 'cap' boundary
    for(int i = 0; i < 10; i++) {
        bq_enqueue(q, msg1);
        assert(bq_size(q) == 1);
        bq_dequeue(q, &out);
        assert(out == msg1);
        assert(bq_size(q) == 0);
    }

    // Invariant 16: Cleanup
    printf("[4/4] Testing Destruction...\n");
    bq_destroy(q);
    free(q);

    printf("--- All Single Thread Tests Passed! ---\n");
    return 0;
}
