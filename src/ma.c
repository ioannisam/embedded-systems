#include "ma.h"
#include "common.h"

#include <sys/timerfd.h>
#include <stdio.h>
#include <unistd.h>

void* ma_thread(void* arg) {

    int tfd = timerfd_create(CLOCK_REALTIME, 0);
    struct itimerspec its;
    struct timespec now;
    
    clock_gettime(CLOCK_REALTIME, &now);
    its.it_value.tv_sec = (now.tv_sec/60 + 1) * 60;  // next minute
    its.it_value.tv_nsec = 0;
    its.it_interval.tv_sec = 60;  // every minute
    its.it_interval.tv_nsec = 0;
    timerfd_settime(tfd, TFD_TIMER_ABSTIME, &its, NULL);

    while(1) {

        // wait for next minute
        uint64_t exp;
        read(tfd, &exp, sizeof(exp));  
        
        time_t current_time = time(NULL);
        time_t cutoff = current_time - MA_WINDOW;
        
        for(int i=0; i<SYMBOL_COUNT; i++) {
            pthread_mutex_lock(&symbol_histories[i].mutex);
            
            // remove old trades
            size_t new_count = 0;
            for(size_t j = 0; j < symbol_histories[i].count; j++) {
                if(symbol_histories[i].trades[j].timestamp >= cutoff) {
                    symbol_histories[i].trades[new_count++] = symbol_histories[i].trades[j];
                }
            }
            symbol_histories[i].count = new_count;

            double sum_price = 0.0, sum_volume = 0.0;
            for(size_t j=0; j<symbol_histories[i].count; j++) {
                sum_price  += symbol_histories[i].trades[j].price;
                sum_volume += symbol_histories[i].trades[j].volume;
            }
            
            char filename[128];
            snprintf(filename, sizeof(filename), "logs/ma/%s.log", symbols[i]);
            FILE* f = fopen(filename, "a");
            if(f) {
                fprintf(f, "%llu,%.8f,%.8f\n",
                    (unsigned long long)current_time,
                    sum_price / symbol_histories[i].count,
                    sum_volume);
                fclose(f);
            }

            pthread_mutex_unlock(&symbol_histories[i].mutex);
        }
    }
    return NULL;
}