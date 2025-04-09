#include "queue.h"

#include <stdlib.h>
#include <stdio.h>

void queueInit(queue* fifo) {
    fifo->empty = 1;
    fifo->full = 0;
    fifo->head = 0;
    fifo->tail = 0;
    fifo->mut = (pthread_mutex_t *) malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init(fifo->mut, NULL);
    fifo->notFull = (pthread_cond_t *) malloc(sizeof(pthread_cond_t));
    pthread_cond_init(fifo->notFull, NULL);
    fifo->notEmpty = (pthread_cond_t *) malloc(sizeof(pthread_cond_t));
    pthread_cond_init(fifo->notEmpty, NULL);
}

void queueDelete(queue* fifo) {
    pthread_mutex_destroy(fifo->mut);
    free(fifo->mut);
    pthread_cond_destroy(fifo->notFull);
    free(fifo->notFull);
    pthread_cond_destroy(fifo->notEmpty);
    free(fifo->notEmpty);
}

void queueAdd(queue* fifo, int item) {
    fifo->buf[fifo->tail] = item;
    fifo->tail++;
    if (fifo->tail == QUEUESIZE)
        fifo->tail = 0;
    if (fifo->tail == fifo->head)
        fifo->full = 1;
    fifo->empty = 0;
}

void queueDel(queue* fifo, int* item) {
    *item = fifo->buf[fifo->head];
    fifo->head++;
    if (fifo->head == QUEUESIZE)
        fifo->head = 0;
    if (fifo->head == fifo->tail)
        fifo->empty = 1;
    fifo->full = 0;
}