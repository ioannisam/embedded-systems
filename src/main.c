#include "queue.h"

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

// Thread function declarations
void* producer(void *q);
void* consumer(void *q);

int main() {
    
    queue q;
    pthread_t pro, con;

    queueInit(&q);
    
    pthread_create(&con, NULL, consumer, &q);
    pthread_create(&pro, NULL, producer, &q);
    
    pthread_join(pro, NULL);
    pthread_join(con, NULL);
    
    queueDelete(&q);
    
    return 0;
}