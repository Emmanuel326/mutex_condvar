#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "bounded_queue.h"
#include "./common/ring_buffer.h"


struct bounded_queue {
    struct ring_buffer rb;
    pthread_mutex_t lock;
    pthread_cond_t not_full;
    pthread_cond_t not_empty;
};






int bq_init(bounded_queue_t *q , size_t capacity)
{
        if(!q || capacity==0)
        {
                return -1;
        }

        q->rb.buffer=calloc(capacity, sizeof(void *));
        if(!q->rb.buffer)
        {return -1;}

        q->rb.capacity=capacity;
        q->rb.size=0;
        q->rb.head=0;
        q->rb.tail=0;
        

        if(pthread_mutex_init(&q->lock,NULL) !=0)
                goto err_mutex;
        if(pthread_cond_init(&q->not_full, NULL)!=0)
                goto err_not_full;
        if(pthread_cond_init(&q->not_empty, NULL)!=0)
                goto err_not_empty;

        return 0;

err_not_empty:
    pthread_cond_destroy(&q->not_full);
err_not_full:
    pthread_mutex_destroy(&q->lock);
err_mutex:
    free(q->rb.buffer);
    return -1;

}

void bq_destroy(bounded_queue_t *q)
{
        if(!q)
                return;

        pthread_cond_destroy(&q->not_empty);
        pthread_cond_destroy(&q->not_full);
        pthread_mutex_destroy(&q->lock);

        free(q->rb.buffer);
}

int bq_enqueue(bounded_queue_t *q, void *item)
{
        if(!q)
                return -1;
        pthread_mutex_lock(&q->lock);

        //block while queue is full
        while(q->rb.size==q->rb.capacity)
        {
                pthread_cond_wait(&q->not_full, &q->lock);
        }

        //invariants that must hold inside the critical section
        assert(q->rb.size<q->rb.capacity);
        assert(q->rb.tail<q->rb.capacity);

        //enque a single item
        q->rb.buffer[q->rb.tail]=item;
        q->rb.tail=(q->rb.tail + 1)%q->rb.capacity;
        q->rb.size++;

        //queue is no longer empty; wake one consumer
        pthread_cond_signal(&q->not_empty);

        pthread_mutex_unlock(&q->lock);
        return 0;

}

int bq_dequeue(bounded_queue_t *q, void **item)
{
        if(!q || !item)
                return -1;

        pthread_mutex_lock(&q->lock);

        //block while queue is empty
        while(q->rb.size==0)
        {
                pthread_cond_wait(&q->not_empty,&q->lock);
        }

        //invariants that must hold inside the critical section
        assert(q->rb.size>0);
        assert(q->rb.head<q->rb.capacity);
        
        //dequeu the item
        *item=q->rb.buffer[q->rb.head];
        q->rb.head=(q->rb.head+1)%q->rb.capacity;
        q->rb.size--;

        //queue is no longer full; wake one producer
        pthread_cond_signal(&q->not_full);

        pthread_mutex_unlock(&q->lock);
        return 0;

}

size_t bq_capacity(bounded_queue_t *q)
{
        if(!q)
                return 0;
        return q->rb.capacity;

}

size_t bq_size(bounded_queue_t *q)
{
        if(!q)
                return 0;
        pthread_mutex_lock(&q->lock);

        size_t size=q->rb.size;
        pthread_mutex_unlock(&q->lock);

        return size;
}

bounded_queue_t* bq_create(size_t capacity)
{
        bounded_queue_t *q=malloc(sizeof(struct bounded_queue));
        if(!q) return NULL;

        if(bq_init(q, capacity) != 0 )
        {
                free(q);
                return NULL;
        }
        return q;
}

