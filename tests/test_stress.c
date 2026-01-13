#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <pthread.h>
#include <stdint.h>
#include "../bounded_queue.h"

#define NUM_PRODUCERS 4
#define NUM_CONSUMERS 4
#define ITEMS_PER_PRODUCER 10000
#define TOTAL_ITEMS (NUM_PRODUCERS * ITEMS_PER_PRODUCER)

/**
 * PRODUCER: Puts items INTO the queue.
 * Each producer pushes numbers 1 through 10,000.
 */
void* producer(void* arg) {
    bounded_queue_t *q = (bounded_queue_t*)arg;
    for (uintptr_t i = 1; i <= ITEMS_PER_PRODUCER; i++) {
        bq_enqueue(q, (void*)i);
    }
    return NULL;
}

/**
 * CONSUMER: Takes items OUT of the queue.
 * Each consumer pulls a fair share of the total items and sums them.
 */
void* consumer(void* arg) {
    bounded_queue_t *q = (bounded_queue_t*)arg;
    uintptr_t local_sum = 0;
    
    for (int i = 0; i < TOTAL_ITEMS / NUM_CONSUMERS; i++) {
        void* item;
        bq_dequeue(q, &item);
        local_sum += (uintptr_t)item;
    }
    return (void*)local_sum;
}

int main(void) {
    printf("Starting Stress Test: %d producers, %d consumers...\n", 
            NUM_PRODUCERS, NUM_CONSUMERS);

    // Small capacity (100) forces heavy use of condition variables
    bounded_queue_t *q = bq_create(100);
    if (!q) {
        fprintf(stderr, "Failed to create queue\n");
        return 1;
    }

    pthread_t prods[NUM_PRODUCERS];
    pthread_t cons[NUM_CONSUMERS];

    // Launch all threads
    for (int i = 0; i < NUM_PRODUCERS; i++) 
        pthread_create(&prods[i], NULL, producer, q);
    
    for (int i = 0; i < NUM_CONSUMERS; i++) 
        pthread_create(&cons[i], NULL, consumer, q);

    // Wait for producers to finish filling the queue
    for (int i = 0; i < NUM_PRODUCERS; i++) 
        pthread_join(prods[i], NULL);

    // Wait for consumers to finish draining the queue and collect sums
    long total_sum = 0;
    for (int i = 0; i < NUM_CONSUMERS; i++) {
        void* res;
        pthread_join(cons[i], &res);
        total_sum += (uintptr_t)res;
    }

    // Mathematical verification:
    // Sum of 1..n is n(n+1)/2. We have NUM_PRODUCERS sets of these.
    long n = ITEMS_PER_PRODUCER;
    long expected_sum = (long)NUM_PRODUCERS * (n * (n + 1) / 2);

    printf("Expected Sum: %ld\n", expected_sum);
    printf("Actual Sum:   %ld\n", total_sum);

    // Final checks
    assert(total_sum == expected_sum);
    assert(bq_size(q) == 0);

    bq_destroy(q);
    printf("Stress Test Passed! No data lost or corrupted.\n");

    return 0;
}
