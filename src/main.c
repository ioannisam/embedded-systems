#include "queue.h"
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>

void* consumer(void* q);
void* producer(void* q);

int main(int argc, char *argv[]) {

    if (argc != 3) {
        fprintf(stderr, "Usage: %s <PRODUCERS> <CONSUMERS>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    int PRODUCERS = atoi(argv[1]);
    int CONSUMERS = atoi(argv[2]);

    queue q;
    pthread_t *producers, *consumers;
    queueInit(&q);

    producers = malloc(PRODUCERS*sizeof(pthread_t));
    consumers = malloc(CONSUMERS*sizeof(pthread_t));
    for (int i=0; i<CONSUMERS; i++) {
        pthread_create(&consumers[i], NULL, consumer, &q);
    }
    for (int i=0; i<PRODUCERS; i++) {
        pthread_create(&producers[i], NULL, producer, &q);
    }
    for (int i=0; i<PRODUCERS; i++) {
        pthread_join(producers[i], NULL);
    }
    // Poison pills
    for (int i=0; i<CONSUMERS; i++) {
        workFunction* wf = malloc(sizeof(workFunction));
        wf->work = NULL;
        wf->arg  = NULL;
        pthread_mutex_lock(q.mut);
        while (q.full) pthread_cond_wait(q.notFull, q.mut);
        queueAdd(&q, wf);
        pthread_mutex_unlock(q.mut);
        pthread_cond_signal(q.notEmpty);
    }
    for (int i=0; i<CONSUMERS; i++) {
        pthread_join(consumers[i], NULL);
    }

    queueDelete(&q);
    free(producers);
    free(consumers);
    return 0;
}