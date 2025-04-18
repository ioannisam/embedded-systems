#include "system/timing.h"
#include "core/common.h"

#include <sys/timerfd.h>
#include <unistd.h>

static int tfd;

void timing_init() {
    struct itimerspec its;
    struct timespec now;
    clock_gettime(CLOCK_REALTIME, &now);
    
    tfd = timerfd_create(CLOCK_REALTIME, 0);
    its.it_value.tv_sec = (now.tv_sec / 60 + 1) * 60; // next minute
    its.it_value.tv_nsec = 0;
    its.it_interval.tv_sec = 60;
    its.it_interval.tv_nsec = 0;
    timerfd_settime(tfd, TFD_TIMER_ABSTIME, &its, NULL);
}

void timing_wait_next() {
    uint64_t exp;
    read(tfd, &exp, sizeof(exp));
}

void timing_log(const struct timespec* start, const struct timespec* end) {
    FILE* f = fopen("logs/timing.csv", "a");
    if (f) {
        fprintf(f, "%lld.%03ld,%lld.%03ld,%lld.%03ld\n",
            (long long)start->tv_sec, (long)(start->tv_nsec / 1000000),
            (long long)start->tv_sec + ((end->tv_sec - start->tv_sec) / 2),
            (long)(start->tv_nsec / 1000000),
            (long long)end->tv_sec, (long)(end->tv_nsec / 1000000));
        fclose(f);
    }
}