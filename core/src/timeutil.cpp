#include "timeutil.h"

#include <chrono>

tu_time_t TIME_STARTED_MICROS = std::chrono::time_point_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now()).time_since_epoch().count();

static tu_time_t timeOverride = -1UL;

tu_time_t tu_clock_micros() {
    if (timeOverride != -1UL) return timeOverride;
    tu_time_t now = std::chrono::time_point_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now()).time_since_epoch().count();;

    return now - TIME_STARTED_MICROS;
}

void tu_set_override(tu_time_t destTime) {
    timeOverride = destTime;
}