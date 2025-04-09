#include "queue.h"
#include <stdlib.h>
#include <math.h>
#include <sys/time.h>

#define PRODUCER_ITERATIONS 100000

void* compute_sine(void *arg);

void* producer(void *q) {
    queue* fifo = (queue*)q;
    for (int i=0; i<PRODUCER_ITERATIONS; i++) {
        pthread_mutex_lock(fifo->mut);
        while (fifo->full) {
            pthread_cond_wait(fifo->notFull, fifo->mut);
        }

        workFunction* wf = malloc(sizeof(workFunction));
        gettimeofday(&wf->enqueue_time, NULL);
        wf->work = compute_sine;
        double* angle = malloc(sizeof(double));
        *angle = (i%10)*0.1*M_PI;
        wf->arg = angle;

        queueAdd(fifo, wf);
        pthread_mutex_unlock(fifo->mut);
        pthread_cond_signal(fifo->notEmpty);
    }
    return NULL;
}