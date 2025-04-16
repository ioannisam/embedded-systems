#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>

typedef struct {
    unsigned long user;
    unsigned long nice;
    unsigned long system;
    unsigned long idle;
    unsigned long iowait;
} CpuStats;

static void read_cpu_stats(CpuStats* stats) {
    FILE* fp = fopen("/proc/stat", "r");
    if(!fp) return;

    char line[256];
    if(fgets(line, sizeof(line), fp)) {
        sscanf(line, "cpu %lu %lu %lu %lu %lu",
               &stats->user, &stats->nice, &stats->system,
               &stats->idle, &stats->iowait);
    }
    fclose(fp);
}

static float calculate_idle(const CpuStats* prev, const CpuStats* curr) {
    const unsigned long prev_total = prev->user + prev->nice + prev->system +
                                    prev->idle + prev->iowait;
    const unsigned long curr_total = curr->user + curr->nice + curr->system +
                                    curr->idle + curr->iowait;
    
    const unsigned long total_diff = curr_total - prev_total;
    if(total_diff == 0) return 0.0f;
    
    const unsigned long idle_diff = curr->idle - prev->idle;
    return ((float)idle_diff / total_diff) * 100.0f;
}

void* cpu_monitor_thread(void* arg) {
    CpuStats prev = {0}, curr = {0};
    
    read_cpu_stats(&prev);
    
    while(1) {
        sleep(CPU_MONITOR_INTERVAL);
        
        read_cpu_stats(&curr);
        
        // idle percentage
        float idle_pct = calculate_idle(&prev, &curr);
        prev = curr;
        
        // Log to file
        time_t now = time(NULL);
        FILE* f = fopen(CPU_LOG_PATH, "a");
        if(f) {
            fprintf(f, "%ld %.2f\n", now, idle_pct);
            fclose(f);
        }
    }
    
    return NULL;
}