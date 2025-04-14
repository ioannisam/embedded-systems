#include "websocket.h"
#include "common.h"
#include "parser.h"

#include <string.h>

static int interrupted = 0;

int ws_callback(struct lws* wsi, enum lws_callback_reasons reason, void* user, void* in, size_t len) {
    switch (reason) {
        case LWS_CALLBACK_CLIENT_ESTABLISHED: {
            const char* msg = "{\"op\":\"subscribe\",\"args\":["
                "{\"channel\":\"trades\",\"instId\":\"BTC-USDT\"},"
                "{\"channel\":\"trades\",\"instId\":\"ADA-USDT\"},"
                "{\"channel\":\"trades\",\"instId\":\"ETH-USDT\"},"
                "{\"channel\":\"trades\",\"instId\":\"DOGE-USDT\"},"
                "{\"channel\":\"trades\",\"instId\":\"XRP-USDT\"},"
                "{\"channel\":\"trades\",\"instId\":\"SOL-USDT\"},"
                "{\"channel\":\"trades\",\"instId\":\"LTC-USDT\"},"
                "{\"channel\":\"trades\",\"instId\":\"BNB-USDT\"}"
            "]}";
            unsigned char buf[LWS_PRE + 1024];
            memcpy(&buf[LWS_PRE], msg, strlen(msg));
            lws_write(wsi, &buf[LWS_PRE], strlen(msg), LWS_WRITE_TEXT);
            break;
        }
        case LWS_CALLBACK_CLIENT_RECEIVE:
            parse_and_queue((char*)in, len, &trade_queue);
            break;
        case LWS_CALLBACK_CLIENT_CONNECTION_ERROR:
        case LWS_CALLBACK_CLOSED:
            interrupted = 1;
            break;
        default:
            break;
    }
    return 0;
}

struct lws_context* ws_init_context() {
    struct lws_protocols protocols[] = {
        {"okx-protocol", ws_callback, 0, 512},
        {NULL, NULL, 0, 0}
    };

    struct lws_context_creation_info info;
    memset(&info, 0, sizeof(info));
    info.port = CONTEXT_PORT_NO_LISTEN;
    info.protocols = protocols;
    info.options |= LWS_SERVER_OPTION_DO_SSL_GLOBAL_INIT;
    info.client_ssl_ca_filepath = "/etc/ssl/certs/ca-certificates.crt";

    return lws_create_context(&info);
}

struct lws* ws_connect(struct lws_context* context) {
    struct lws_client_connect_info ccinfo = {0};
    ccinfo.context = context;
    ccinfo.address = OKX_WS_HOST;
    ccinfo.port = OKX_WS_PORT;
    ccinfo.path = OKX_WS_PATH;
    ccinfo.host = OKX_WS_HOST;
    ccinfo.ssl_connection = LCCSCF_USE_SSL;
    ccinfo.protocol = "okx-protocol";

    return lws_client_connect_via_info(&ccinfo);
}

int ws_service_loop(struct lws_context* context) {
    while (!interrupted) {
        lws_service(context, 1000);
    }
    return 0;
}