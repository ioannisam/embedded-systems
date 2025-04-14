#pragma once

#include <libwebsockets.h>
#include <pthread.h>
#include <stdint.h>
#include <string.h>
#include <jansson.h>
#include <stdlib.h>
#include <time.h>

#define SYMBOL_COUNT 8
#define OKX_WS_HOST "ws.okx.com"
#define OKX_WS_PORT 8443
#define OKX_WS_PATH "/ws/v5/public"
#define QUEUE_SIZE 1024
#define MA_WINDOW 900  // 15 minutes in seconds

typedef struct {
    char symbol[16];
    double price;
    double volume;
    uint64_t timestamp;
} TradeData;

typedef struct {
    TradeData* trades;
    size_t count;
    size_t capacity;
    pthread_mutex_t mutex;
} SymbolHistory;

struct TradeQueue;
typedef struct TradeQueue TradeQueue;

extern TradeQueue trade_queue;
extern SymbolHistory symbol_histories[SYMBOL_COUNT];
extern const char* symbols[SYMBOL_COUNT];