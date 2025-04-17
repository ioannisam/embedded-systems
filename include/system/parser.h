#pragma once

#include "core/common.h"
#include "core/queue.h"

void parse_and_queue(const char *json_str, size_t len, TradeQueue* queue);