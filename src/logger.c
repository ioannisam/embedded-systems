#include "logger.h"
#include "common.h"
#include "queue.h"

#include <stdio.h>
#include <time.h>

void* logger_thread(void* arg) {
    
    TradeQueue* q = (TradeQueue*)arg;
    TradeData trade;
    
    while(1) {
        queue_pop(q, &trade);
        
        char filename[128];
        snprintf(filename, sizeof(filename), "logs/%s.log", trade.symbol);
        
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