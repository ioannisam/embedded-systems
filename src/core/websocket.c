#include <pthread.h>
#include <stdbool.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>

#include "core/websocket.h"
#include "core/common.h"
#include "system/parser.h"

atomic_bool connection_lost = ATOMIC_VAR_INIT(true);

static pthread_mutex_t log_mutex = PTHREAD_MUTEX_INITIALIZER;
static time_t last_activity = 0;
static time_t last_ping     = 0;

static struct lws* current_wsi = NULL;

void log_connection(const char* event, const char* reason) {
    pthread_mutex_lock(&log_mutex);
    FILE* f = fopen("logs/connections.log", "a");
    if (f) {
        time_t now = time(NULL);
        struct tm* t = localtime(&now);
        char buf[20];
        strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", t);
        fprintf(f, "[%s] %-12s", buf, event);
        if (reason) fprintf(f, " - %s", reason);
        fprintf(f, "\n");
        fclose(f);
    }
    pthread_mutex_unlock(&log_mutex);
}

static void send_subscribe_message(struct lws *wsi) {
    const char* msg =
      "{\"op\":\"subscribe\",\"args\":["
        "{\"channel\":\"trades\",\"instId\":\"BTC-USDT\"},"
        "{\"channel\":\"trades\",\"instId\":\"ADA-USDT\"},"
        "{\"channel\":\"trades\",\"instId\":\"ETH-USDT\"},"
        "{\"channel\":\"trades\",\"instId\":\"DOGE-USDT\"},"
        "{\"channel\":\"trades\",\"instId\":\"XRP-USDT\"},"
        "{\"channel\":\"trades\",\"instId\":\"SOL-USDT\"},"
        "{\"channel\":\"trades\",\"instId\":\"LTC-USDT\"},"
        "{\"channel\":\"trades\",\"instId\":\"BNB-USDT\"}"
      "]}";
    size_t len = strlen(msg);
    unsigned char buf[LWS_PRE + len];
    memcpy(&buf[LWS_PRE], msg, len);

    int ret = lws_write(wsi, &buf[LWS_PRE], len, LWS_WRITE_TEXT);
    if (ret < 0) {
        log_connection("ERROR", "subscribe failed");
        atomic_store(&connection_lost, true);
    } else {
        // mark that we've sent the sub
        bool* subscribed = lws_wsi_user(wsi);
        *subscribed = true;
    }
}

int ws_callback(struct lws* wsi, enum lws_callback_reasons reason, void* user, void* in, size_t len) {
    
    bool* subscribed = (bool*)user;
    time_t now = time(NULL);
    switch (reason) {
      case LWS_CALLBACK_CLIENT_ESTABLISHED:
        atomic_store(&connection_lost, false);
        log_connection("ESTABLISHED", NULL);
        last_activity = last_ping = now;
        *subscribed = false;
        lws_callback_on_writable(wsi);
        break;

      case LWS_CALLBACK_CLIENT_WRITEABLE:
        if (!*subscribed) {
          send_subscribe_message(wsi);
        } else if (now - last_ping >= 60) {
          // send a ping frame
          unsigned char ping_buf[LWS_PRE];
          int ret = lws_write(wsi, &ping_buf[LWS_PRE], 0, LWS_WRITE_PING);
          if (ret < 0) {
            log_connection("ERROR", "ping failed");
            atomic_store(&connection_lost, true);
          } else {
            last_ping = now;
          }
        }
        break;

      case LWS_CALLBACK_CLIENT_RECEIVE: {
        // ensure null‑terminated for parser
        char* p = malloc(len+1);
        if (p) {
          memcpy(p, in, len);
          p[len] = '\0';
          parse_and_queue(p, len, &trade_queue);
          free(p);
        }
        last_activity = now;
        break;
      }

      case LWS_CALLBACK_CLIENT_RECEIVE_PONG:
        log_connection("PONG", NULL);
        last_activity = now;
        break;

      case LWS_CALLBACK_CLIENT_CONNECTION_ERROR:
        log_connection("ERROR", in ? (char*)in : "unknown");
        atomic_store(&connection_lost, true);
        break;

      case LWS_CALLBACK_CLOSED:
        log_connection("DISCONNECTED", "clean");
        atomic_store(&connection_lost, true);
        break;

      default:
        break;
    }

    return 0;
}

struct lws_context* ws_init_context(void) {
    
    static struct lws_protocols protocols[] = {
      {
        .name                = "okx-protocol",
        .callback            = ws_callback,
        .per_session_data_size = sizeof(bool),
        .rx_buffer_size      = 0,        // default 4096
      },
      { NULL, NULL, 0, 0 }
    };

    struct lws_context_creation_info info;
    memset(&info, 0, sizeof(info));
    info.port                     = CONTEXT_PORT_NO_LISTEN;
    info.protocols                = protocols;
    info.options                 |= LWS_SERVER_OPTION_DO_SSL_GLOBAL_INIT;
    info.options                 |= LWS_SERVER_OPTION_PEER_CERT_NOT_REQUIRED;
    info.client_ssl_ca_filepath   = "/etc/ssl/certs/ca-certificates.crt";
    info.ka_time                  = 10;
    info.ka_interval              = 5;
    info.ka_probes                = 3;

    return lws_create_context(&info);
}

struct lws* ws_connect(struct lws_context *context) {
    struct lws_client_connect_info ccinfo = {0};
    ccinfo.context      = context;
    ccinfo.address      = OKX_WS_HOST;
    ccinfo.port         = OKX_WS_PORT;
    ccinfo.path         = OKX_WS_PATH;
    ccinfo.host         = OKX_WS_HOST;
    ccinfo.origin       = OKX_WS_ORIGIN;
    ccinfo.ssl_connection = LCCSCF_USE_SSL;
    ccinfo.protocol     = "okx-protocol";
    return lws_client_connect_via_info(&ccinfo);
}

int ws_service_loop(struct lws_context* context) {
    time_t last_connect = 0;
    int    backoff      = 2;
    const int max_backoff = 60;

    last_activity = time(NULL); // initial value should be reasonable

    while (!atomic_load(&exit_requested)) {
        time_t now = time(NULL);

        // reconnect if needed
        if (!current_wsi && atomic_load(&connection_lost)) {
            if (now - last_connect >= backoff) {
                current_wsi = ws_connect(context);
                last_connect = now;
                if (current_wsi) {
                    log_connection("CONNECTING", NULL);
                    last_activity = now;  // reset timer on connect attempt
                } else {
                    log_connection("RETRY", "connect failed");
                    backoff = backoff < max_backoff ? backoff * 2 : max_backoff;
                }
            }
        }

        if (current_wsi) {
            int n = lws_service(context, 50);
            if (n < 0) {
                log_connection("ERROR", "service");
                atomic_store(&connection_lost, true);
                current_wsi = NULL;
                continue;
            }

            // if no activity for 90s -> timeout
            if (now - last_activity > 90) {
                log_connection("ERROR", "no activity");
                atomic_store(&connection_lost, true);
                current_wsi = NULL;
            }
        } else {
            usleep(100000);  // 100 ms backoff for responsiveness
        }
    }

    return 0;
}