#include "queue.h"

#include <pthread.h>
#include <stdlib.h>

#define NUM_PRODUCERS 2
#define NUM_CONSUMERS 3

void* consumer(void* q);
void* producer(void* q);

int main() {

    queue q;
    pthread_t producers[NUM_PRODUCERS], consumers[NUM_CONSUMERS];
    queueInit(&q);

    for (int i=0; i<NUM_CONSUMERS; i++)
        pthread_create(&consumers[i], NULL, consumer, &q);
    for (int i=0; i<NUM_PRODUCERS; i++)
        pthread_create(&producers[i], NULL, producer, &q);

    for (int i=0; i<NUM_PRODUCERS; i++)
        pthread_join(producers[i], NULL);

    // poison pills
    for (int i=0; i<NUM_CONSUMERS; i++) {
        workFunction* wf = malloc(sizeof(workFunction));
        wf->work = NULL;
        wf->arg = NULL;
        pthread_mutex_lock(q.mut);
        while (q.full) pthread_cond_wait(q.notFull, q.mut);
        queueAdd(&q, wf);
        pthread_mutex_unlock(q.mut);
        pthread_cond_signal(q.notEmpty);
    }

    for (int i=0; i<NUM_CONSUMERS; i++)
        pthread_join(consumers[i], NULL);

    queueDelete(&q);
    return 0;
}