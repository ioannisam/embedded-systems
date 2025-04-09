#ifndef QUEUE_H
#define QUEUE_H

#include <pthread.h>
#include <sys/time.h>

#define QUEUESIZE 10

typedef struct workFunction {
    void* (*work)(void*);
    void* arg;
    struct timeval enqueue_time;
} workFunction;

typedef struct {
    workFunction* buf[QUEUESIZE];
    long head, tail;
    int full, empty;
    pthread_mutex_t* mut;
    pthread_cond_t *notFull, *notEmpty;
} queue;

void queueInit(queue* fifo);
void queueDelete(queue* fifo);
void queueAdd(queue* fifo, workFunction*  item);
void queueDel(queue* fifo, workFunction** item);

#endif