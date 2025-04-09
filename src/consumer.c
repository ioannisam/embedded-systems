#include "queue.h"

#include <stdio.h>
#include <unistd.h>

void* consumer(void* q) {
    queue* fifo;
    int i, d;
    
    fifo = (queue*)q;
    
    // First loop with 200ms delay
    for (i=0; i<LOOP; i++) {
        pthread_mutex_lock(fifo->mut);
        while (fifo->empty) {
            printf("consumer: queue EMPTY.\n");
            pthread_cond_wait(fifo->notEmpty, fifo->mut);
        }
        queueDel(fifo, &d);
        pthread_mutex_unlock(fifo->mut);
        pthread_cond_signal(fifo->notFull);
        printf("consumer: received %d.\n", d);
        usleep(200000);
    }
    
    // Second loop with 50ms delay
    for (i=0; i<LOOP; i++) {
        pthread_mutex_lock(fifo->mut);
        while (fifo->empty) {
            printf("consumer: queue EMPTY.\n");
            pthread_cond_wait(fifo->notEmpty, fifo->mut);
        }
        queueDel(fifo, &d);
        pthread_mutex_unlock(fifo->mut);
        pthread_cond_signal(fifo->notFull);
        printf("consumer: received %d.\n", d);
        usleep(50000);
    }
    
    return NULL;
}