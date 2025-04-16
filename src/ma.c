#include "ma.h"
#include "common.h"

#include <sys/timerfd.h>
#include <stdio.h>
#include <unistd.h>
#include <math.h>

static double pearson(double* x, double* y, int n) {

    if (n < 2) {
        return 0;
    }

    double sum_x  = 0, sum_y = 0, sum_xy = 0;
    double sum_x2 = 0, sum_y2 = 0;
    
    for(int i=0; i<n; i++) {
        sum_x  += x[i];
        sum_y  += y[i];
        sum_xy += x[i]*y[i];
        sum_x2 += x[i]*x[i];
        sum_y2 += y[i]*y[i];
    }
    
    double num = sum_xy - (sum_x*sum_y)/n;
    double den = sqrt( (sum_x2 - pow(sum_x, 2)/n)*(sum_y2 - pow(sum_y, 2)/n) );
    
    return (fabs(den) > 1e-9) ? num / den : 0.0;
}

void* ma_thread(void* arg) {
    int tfd = timerfd_create(CLOCK_REALTIME, 0);
    struct itimerspec its;
    struct timespec now;
    
    clock_gettime(CLOCK_REALTIME, &now);
    its.it_value.tv_sec = (now.tv_sec/60 + 1) * 60;  // next minute
    its.it_value.tv_nsec = 0;
    its.it_interval.tv_sec = 60;
    its.it_interval.tv_nsec = 0;
    timerfd_settime(tfd, TFD_TIMER_ABSTIME, &its, NULL);

    while(1) {
        uint64_t exp;
        read(tfd, &exp, sizeof(exp));  

        struct timespec processing_start, processing_end;
        clock_gettime(CLOCK_REALTIME, &processing_start);
        
        time_t current_time = time(NULL);
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

        // Second Pass: calculate correlations
        for(int i=0; i<SYMBOL_COUNT; i++) {
            pthread_mutex_lock(&symbol_histories[i].mutex);
            
            int points_needed = 8;
            if(symbol_histories[i].ma_count < points_needed) {
                pthread_mutex_unlock(&symbol_histories[i].mutex);
                continue;
            }

            double correlations[SYMBOL_COUNT] = {0}; // includes self-correlation
            int corr_idx = 0;
            double max_corr = -2.0; // out of valid range [-1, 1]
            char max_symbol[32] = "N/A";

            for (int j=0; j<SYMBOL_COUNT; j++) {
                if (j == i) {
                    // self-correlation
                    correlations[corr_idx] = 1.0;
                    corr_idx++;
                    continue;
                }
                
                pthread_mutex_lock(&symbol_histories[j].mutex);
                if (symbol_histories[j].ma_count >= points_needed) {
                    double x[points_needed], y[points_needed];
                    
                    // Populate x and y arrays
                    for (int k = 0; k < points_needed; k++) {
                        int idx_i = (symbol_histories[i].ma_index - points_needed + k + MA_HISTORY_SIZE) % MA_HISTORY_SIZE;
                        int idx_j = (symbol_histories[j].ma_index - points_needed + k + MA_HISTORY_SIZE) % MA_HISTORY_SIZE;
                        x[k] = symbol_histories[i].ma_history[idx_i];
                        y[k] = symbol_histories[j].ma_history[idx_j];
                    }
                    
                    double corr = pearson(x, y, points_needed);
                    correlations[corr_idx] = corr;
                    
                    // max correlation (excluding self)
                    if (corr > max_corr) {
                        max_corr = corr;
                        strncpy(max_symbol, symbols[j], sizeof(max_symbol) - 1);
                        max_symbol[sizeof(max_symbol) - 1] = '\0';
                    }
                } else {
                    correlations[corr_idx] = 0.0; // default for unavailable data
                }
                pthread_mutex_unlock(&symbol_histories[j].mutex);
                corr_idx++;
            }

            // Log to file
            char corr_filename[128];
            snprintf(corr_filename, sizeof(corr_filename), "logs/corr/%s.log", symbols[i]);
            FILE* cf = fopen(corr_filename, "a");
            if (cf) {
                fprintf(cf, "%llu,%s,%.4f", 
                    (unsigned long long)current_time,
                    max_symbol,
                    max_corr);
                
                // Append all correlations (including self)
                for (int k=0; k<SYMBOL_COUNT; k++) {
                    fprintf(cf, ",%.4f", correlations[k]);
                }
                fprintf(cf, "\n");
                fclose(cf);
            }
            pthread_mutex_unlock(&symbol_histories[i].mutex);
        }

        clock_gettime(CLOCK_REALTIME, &processing_end);
    
        // Log timing data
        FILE* timing_log = fopen("logs/timing/timing.csv", "a");
        if(timing_log) {
            fprintf(timing_log, "%lld.%03ld,%lld.%03ld,%lld.%03ld\n",
                (long long)processing_start.tv_sec, (long)(processing_start.tv_nsec / 1000000),
                (long long)processing_start.tv_sec, (long)(processing_start.tv_nsec / 1000000),
                (long long)processing_end.tv_sec, (long)(processing_end.tv_nsec / 1000000));
        }
        fclose(timing_log);
    }
    return NULL;
}