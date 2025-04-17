#pragma once

#include <time.h>

void timing_init();
void timing_wait_next();
void timing_log(const struct timespec* start, const struct timespec* end);