#include "core/queue.h"
#include "core/common.h"

TradeQueue trade_queue;

void queue_init(TradeQueue* q, size_t size) {

    q->data = malloc(size*sizeof(TradeData));
    q->size = size;
    q->head = q->tail = 0;
    pthread_mutex_init(&q->lock, NULL);
    pthread_cond_init(&q->not_empty, NULL);
}

void queue_push(TradeQueue* q, TradeData* trade) {

    pthread_mutex_lock(&q->lock);
    q->data[q->tail] = *trade;
    q->tail = (q->tail+1) % q->size;
    pthread_cond_signal(&q->not_empty);
    pthread_mutex_unlock(&q->lock);
}

void queue_pop(TradeQueue* q, TradeData* trade) {
    
    pthread_mutex_lock(&q->lock);
    while(q->head == q->tail) {
        pthread_cond_wait(&q->not_empty, &q->lock);
    }
    *trade = q->data[q->head];
    q->head = (q->head+1) % q->size;
    pthread_mutex_unlock(&q->lock);
}