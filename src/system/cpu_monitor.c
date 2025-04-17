#include "system/cpu_monitor.h"
#include "core/common.h"

#include <unistd.h>

void read_cpu_stats(CpuStats* stats) {

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

float calculate_idle(const CpuStats* prev, const CpuStats* curr) {

    const unsigned long prev_total = prev->user + prev->nice + prev->system +
                                    prev->idle + prev->iowait;
    const unsigned long curr_total = curr->user + curr->nice + curr->system +
                                    curr->idle + curr->iowait;
    
    const unsigned long total_diff = curr_total - prev_total;
    if(total_diff == 0) return 0.0f;
    
    const unsigned long idle_diff = curr->idle - prev->idle;
    return ((float)idle_diff / total_diff) * 100.0f;
}