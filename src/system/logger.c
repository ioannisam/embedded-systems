#include "data/logger.h"
#include "core/common.h"
#include "core/queue.h"

void* logger_thread(void* arg) {

    TradeQueue* q = (TradeQueue*)arg;
    TradeData trade;
    
    while(1) {
        queue_pop(q, &trade);
        
        char filename[128];
        snprintf(filename, sizeof(filename), "logs/raw/%s.log", trade.symbol);
        
        FILE* f = fopen(filename, "a");
        if(f) {
            fprintf(f, "%llu,%.8f,%.8f\n",
                (unsigned long long)trade.timestamp,
                trade.price,
                trade.volume);
            fclose(f);
        }
    }
    
    return NULL;
}