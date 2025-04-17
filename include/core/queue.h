#pragma once

#include "core/common.h"

typedef struct TradeQueue {
    TradeData* data;
    size_t size;
    size_t head;
    size_t tail;
    pthread_mutex_t lock;
    pthread_cond_t not_empty;
} TradeQueue;

void queue_init(TradeQueue* q, size_t size);
void queue_push(TradeQueue* q, TradeData* trade);
void queue_pop (TradeQueue* q, TradeData* trade);