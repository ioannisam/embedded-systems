// File: src/main.c
#include "common.h"
#include "websocket.h"
#include "logger.h"
#include "queue.h"

#include <stdlib.h>
#include <pthread.h>

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

    pthread_t logger;
    pthread_create(&logger, NULL, logger_thread, &trade_queue);

    ws_service_loop(context);
    
    lws_context_destroy(context);
    pthread_join(logger, NULL);
    return EXIT_SUCCESS;
}