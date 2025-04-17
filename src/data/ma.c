#include "data/ma.h"

void ma_process(time_t current_time) {
    
    time_t cutoff = current_time - MA_WINDOW;
    
    // First Pass: update MA values
    for(int i=0; i<SYMBOL_COUNT; i++) {
        pthread_mutex_lock(&symbol_histories[i].mutex);
        
        // purge old trades
        size_t new_count = 0;
        for(size_t j=0; j<symbol_histories[i].count; j++) {
            if(symbol_histories[i].trades[j].timestamp >= cutoff) {
                symbol_histories[i].trades[new_count++] = symbol_histories[i].trades[j];
            }
        }
        symbol_histories[i].count = new_count;

        // calculate MA
        double sum_price = 0.0, sum_volume = 0.0;
        for(size_t j=0; j<symbol_histories[i].count; j++) {
            sum_price  += symbol_histories[i].trades[j].price;
            sum_volume += symbol_histories[i].trades[j].volume;
        }
        
        double current_ma = (symbol_histories[i].count > 0) ? 
            sum_price / symbol_histories[i].count : 0.0;

        // update MA history (circular buffer)
        symbol_histories[i].ma_history[symbol_histories[i].ma_index] = current_ma;
        symbol_histories[i].ma_timestamps[symbol_histories[i].ma_index] = current_time;
        symbol_histories[i].ma_index = (symbol_histories[i].ma_index + 1) % MA_HISTORY_SIZE;
        symbol_histories[i].ma_count = (symbol_histories[i].ma_count < MA_HISTORY_SIZE) ? 
                                    symbol_histories[i].ma_count + 1 : MA_HISTORY_SIZE;
        // log MA
        char ma_filename[128];
        snprintf(ma_filename, sizeof(ma_filename), "logs/ma/%s.log", symbols[i]);
        FILE* f = fopen(ma_filename, "a");
        if(f) {
            fprintf(f, "%llu,%.8f,%.8f\n",
                (unsigned long long)current_time,
                current_ma,
                sum_volume);
            fclose(f);
        }
        pthread_mutex_unlock(&symbol_histories[i].mutex);
    }
}