#include "common.h"
#include "websocket.h"
#include "logger.h"
#include "queue.h"
#include "ma.h"

#include <stdlib.h>
#include <pthread.h>

const char* symbols[SYMBOL_COUNT] = {
    "BTC-USDT", "ADA-USDT", "ETH-USDT", "DOGE-USDT",
    "XRP-USDT", "SOL-USDT", "LTC-USDT", "BNB-USDT"
};

int main() {
    struct lws_context* context = ws_init_context();
    if (!context) {
        return EXIT_FAILURE;
    }
    if (!ws_connect(context)) {
        lws_context_destroy(context);
        return EXIT_FAILURE;
    }

    queue_init(&trade_queue, QUEUE_SIZE);

    // initialize symbol histories
    for(int i=0; i<SYMBOL_COUNT; i++) {
        symbol_histories[i] = (SymbolHistory){
            .trades = NULL,
            .count = 0,
            .capacity = 0,
            .mutex = PTHREAD_MUTEX_INITIALIZER,
            .ma_index = 0,
            .ma_history = {0},
            .ma_timestamps = {0}
        };
    }

    pthread_t logger, ma;
    pthread_create(&logger, NULL, logger_thread, &trade_queue);
    pthread_create(&ma, NULL, ma_thread, NULL);

    ws_service_loop(context);
    
    lws_context_destroy(context);
    pthread_join(logger, NULL);
    pthread_join(ma, NULL);
    
    // Cleanup histories
    for(int i=0; i<SYMBOL_COUNT; i++) {
        free(symbol_histories[i].trades);
    }
    
    return EXIT_SUCCESS;
}