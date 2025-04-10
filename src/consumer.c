#include "queue.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <math.h>

void* compute_sine(void* arg) {
    
    double *angle = (double *)arg;
    fprintf(stderr, "Sine(%f) = %f\n", *angle, sin(*angle));
    return NULL;
}

void* consumer(void* q) {

    queue* fifo = (queue*)q;
    while (1) {
        pthread_mutex_lock(fifo->mut);
        while (fifo->empty) {
            pthread_cond_wait(fifo->notEmpty, fifo->mut);
        }

        workFunction* wf;
        queueDel(fifo, &wf);
        pthread_mutex_unlock(fifo->mut);
        pthread_cond_signal(fifo->notFull);

        // poison pill
        if (wf->work == NULL) {
            free(wf);
            break;
        }

        struct timeval dequeue_time;
        gettimeofday(&dequeue_time, NULL);
        long long wait_time = (dequeue_time.tv_sec - wf->enqueue_time.tv_sec) * 1000000LL + 
                             (dequeue_time.tv_usec - wf->enqueue_time.tv_usec);
        fprintf(stdout, "%lld\n", wait_time);

        wf->work(wf->arg);
        free(wf->arg);
        free(wf);
    }
    return NULL;
}