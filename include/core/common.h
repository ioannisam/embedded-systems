#pragma once

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <jansson.h>
#include <time.h>

#define SYMBOL_COUNT 8
#define QUEUE_SIZE 1024
#define MA_WINDOW 900  // 15 minutes in seconds
#define MA_HISTORY_SIZE 8
#define CPU_LOG_PATH "logs/cpu.log"
#define CPU_MONITOR_INTERVAL 1 // seconds
#define OKX_WS_HOST "ws.okx.com"
#define OKX_WS_PORT 8443
#define OKX_WS_PATH "/ws/v5/public"
#define OKX_WS_ORIGIN "https://www.okx.com"

struct TradeQueue;
typedef struct TradeQueue TradeQueue;

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

    // last 8 MA values
    double ma_history[MA_HISTORY_SIZE];
    time_t ma_timestamps[MA_HISTORY_SIZE];   
    int ma_index;
    int ma_count;
} SymbolHistory;

extern TradeQueue trade_queue;
extern SymbolHistory symbol_histories[SYMBOL_COUNT];
extern const char* symbols[SYMBOL_COUNT];
extern atomic_bool exit_requested;