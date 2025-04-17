#include "system/timing.h"
#include "system/cpu_monitor.h"
#include "data/ma.h"
#include "data/corr.h"

void* scheduler_thread(void* arg) {

    timing_init();
    CpuStats prev = {0}, curr = {0};
    read_cpu_stats(&prev);
    
    while (1) {

        // wait for next minute
        timing_wait_next(); 

        struct timespec start, end;
        clock_gettime(CLOCK_REALTIME, &start);
        time_t current_time = time(NULL);
        
        ma_process(current_time);
        corr_process(current_time);
        
        clock_gettime(CLOCK_REALTIME, &end);
        timing_log(&start, &end);  // Log timing data

        read_cpu_stats(&curr);
        float idle_pct = calculate_idle(&prev, &curr);
        prev = curr;
        FILE* f = fopen(CPU_LOG_PATH, "a");
        if (f) {
            fprintf(f, "%ld %.2f\n", time(NULL), idle_pct);
            fclose(f);
        }
    }
    return NULL;
}