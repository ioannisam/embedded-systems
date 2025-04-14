#pragma once

#include <libwebsockets.h>
#include <pthread.h>
#include <stdint.h>
#include <string.h>
#include <jansson.h>
#include <stdlib.h>

#define SYMBOL_COUNT 8
#define OKX_WS_HOST "ws.okx.com"
#define OKX_WS_PORT 8443
#define OKX_WS_PATH "/ws/v5/public"
#define QUEUE_SIZE 1024

typedef struct {
    char symbol[16];
    double price;
    double volume;
    uint64_t timestamp;
} TradeData;