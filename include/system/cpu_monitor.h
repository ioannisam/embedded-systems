#pragma once

#include <time.h>

typedef struct {
    unsigned long user;
    unsigned long nice;
    unsigned long system;
    unsigned long idle;
    unsigned long iowait;
} CpuStats;

void  read_cpu_stats(CpuStats* stats);
float calculate_idle(const CpuStats* prev, const CpuStats* curr);