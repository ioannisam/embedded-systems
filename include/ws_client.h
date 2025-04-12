#ifndef WS_CLIENT_H
#define WS_CLIENT_H

#include <libwebsockets.h>

// connection parameters
#define OKX_WS_HOST "ws.okx.com"
#define OKX_WS_PORT 8443
#define OKX_WS_PATH "/ws/v5/public"

int ws_callback(struct lws* wsi, enum lws_callback_reasons reason, void* user, void* in, size_t len);
struct lws_context* ws_init_context();
struct lws* ws_connect(struct lws_context* context);
int ws_service_loop(struct lws_context* context);

#endif