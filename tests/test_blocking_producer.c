#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <assert.h>
#include "../bounded_queue.h"

void* delayed_consumer(void* arg) {
    bounded_queue_t *q = (bounded_queue_t*)arg;
    printf("  [Consumer] Sleeping for 1s...\n");
    sleep(1);
    
    void *out;
    bq_dequeue(q, &out);
    printf("  [Consumer] Dequeued item, making space!\n");
    return NULL;
}

int main() {
    printf("--- Starting Blocking Producer Test ---\n");
    
    size_t cap = 2;
    bounded_queue_t *q = malloc(256); 
    bq_init(q, cap);

    // Fill to capacity
    bq_enqueue(q, "Task 1");
    bq_enqueue(q, "Task 2");
    printf("Queue is now full (2/2).\n");

    pthread_t consumer_tid;
    pthread_create(&consumer_tid, NULL, delayed_consumer, q);

    printf("Attempting to enqueue 3rd item (should block)...\n");
    bq_enqueue(q, "Task 3"); 
    printf("Successfully enqueued 3rd item!\n");

    assert(bq_size(q) == 2); // Should be full again

    pthread_join(consumer_tid, NULL);
    bq_destroy(q);
    free(q);

    printf("--- Blocking Producer Test Passed! ---\n");
    return 0;
}
