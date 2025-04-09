#ifndef QUEUE_H
#define QUEUE_H

#include <pthread.h>

#define QUEUESIZE 10
#define LOOP 20

typedef struct {
    int buf[QUEUESIZE];
    long head, tail;
    int full, empty;
    pthread_mutex_t* mut;
    pthread_cond_t *notFull, *notEmpty;
} queue;

// Queue operations
void queueInit(queue* fifo);
void queueDelete(queue* fifo);
void queueAdd(queue* fifo, int  item);
void queueDel(queue* fifo, int* item);

#endif