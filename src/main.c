#include "ws_client.h"
#include <stdlib.h>

int main(void) {

    struct lws_context* context = ws_init_context();
    if (!context) {
        return EXIT_FAILURE;
    }

    if (!ws_connect(context)) {
        lws_context_destroy(context);
        return EXIT_FAILURE;
    }

    ws_service_loop(context);
    
    lws_context_destroy(context);
    return EXIT_SUCCESS;
}