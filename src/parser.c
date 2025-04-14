#include "parser.h"
#include "common.h"

void parse_and_queue(const char* json_str, size_t len, TradeQueue* queue) {
    
    json_error_t error;
    json_t* root = json_loadb(json_str, len, 0, &error);
    
    if(root && json_is_object(root)) {
        json_t* data = json_object_get(root, "data");
        if(json_is_array(data)) {
            size_t index;
            json_t* trade;
            
            json_array_foreach(data, index, trade) {
                TradeData tdata;
                const char *symbol = json_string_value(json_object_get(trade, "instId"));
                const char *px = json_string_value(json_object_get(trade, "px"));
                const char *sz = json_string_value(json_object_get(trade, "sz"));
                const char *ts = json_string_value(json_object_get(trade, "ts"));
                
                if(symbol && px && sz && ts) {
                    strncpy(tdata.symbol, symbol, sizeof(tdata.symbol) - 1);
                    tdata.symbol[sizeof(tdata.symbol) - 1] = '\0';
                    tdata.price = atof(px);
                    tdata.volume = atof(sz);
                    tdata.timestamp = strtoull(ts, NULL, 10);
                    queue_push(queue, &tdata);
                }
            }
        }
        json_decref(root);
    }
}