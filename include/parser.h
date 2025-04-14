#pragma once
#include "common.h"
#include "queue.h"
#include <stddef.h>

void parse_and_queue(const char *json_str, size_t len, TradeQueue* queue);