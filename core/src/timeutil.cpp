#include "timeutil.h"

#include <chrono>

tu_time_t TIME_STARTED_MICROS = std::chrono::time_point_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now()).time_since_epoch().count();


tu_time_t tu_clock_micros() {
    tu_time_t now = std::chrono::time_point_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now()).time_since_epoch().count();;

    return now - TIME_STARTED_MICROS;
}