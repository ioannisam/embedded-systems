#include "queue.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void* producer(void* q) {
    queue* fifo;
    int i;
    
    fifo = (queue*)q;
    
    for (i=0; i<LOOP; i++) {
        pthread_mutex_lock(fifo->mut);
        while (fifo->full) {
            printf("producer: queue FULL.\n");
            pthread_cond_wait(fifo->notFull, fifo->mut);
        }
        queueAdd(fifo, i);
        pthread_mutex_unlock(fifo->mut);
        pthread_cond_signal(fifo->notEmpty);
        usleep(100000);
    }

    for (i=0; i<LOOP; i++) {
        pthread_mutex_lock (fifo->mut);
        while (fifo->full) {
            printf ("producer: queue FULL.\n");
            pthread_cond_wait (fifo->notFull, fifo->mut);
        }
        queueAdd (fifo, i);
        pthread_mutex_unlock (fifo->mut);
        pthread_cond_signal (fifo->notEmpty);
        usleep (200000);
    }
    
    return NULL;
}