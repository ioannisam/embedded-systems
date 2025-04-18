#include "core/common.h"
#include "core/websocket.h"
#include "core/queue.h"
#include "data/logger.h"
#include "system/scheduler.h"

#include <signal.h>
#include <string.h>

const char* symbols[SYMBOL_COUNT] = {
    "BTC-USDT", "ADA-USDT", "ETH-USDT", "DOGE-USDT",
    "XRP-USDT", "SOL-USDT", "LTC-USDT", "BNB-USDT"
};

atomic_bool exit_requested = ATOMIC_VAR_INIT(false);
static volatile sig_atomic_t signal_count = 0;

void handle_signal(int sig) {
    (void)sig;
    if (++signal_count == 1) {
        atomic_store(&exit_requested, true);
        printf("\nGraceful shutdown initiated. Press Ctrl+C again to force exit.\n");
        fflush(stdout);
    } else {
        printf("\nForcing exit.\n");
        fflush(stdout);
        _exit(EXIT_FAILURE);
    }
}

int main() {

    // Set up signal handlers
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = handle_signal;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);

    struct lws_context* context = ws_init_context();
    if (!context) {
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

    pthread_t logger, scheduler;
    pthread_create(&logger, NULL, logger_thread, &trade_queue);
    pthread_create(&scheduler, NULL, scheduler_thread, NULL);

    ws_service_loop(context);
    
    lws_context_destroy(context);
    pthread_join(logger, NULL);
    pthread_join(scheduler, NULL);
    
    // cleanup histories
    for(int i=0; i<SYMBOL_COUNT; i++) {
        free(symbol_histories[i].trades);
    }
    
    return EXIT_SUCCESS;
}