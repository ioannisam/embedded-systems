#pragma once
#include <libwebsockets.h>
#include <stddef.h>

struct lws_context* ws_init_context();
struct lws* ws_connect(struct lws_context* context);
int ws_service_loop(struct lws_context* context);
int ws_callback(struct lws* wsi, enum lws_callback_reasons reason, void* user, void* in, size_t len);