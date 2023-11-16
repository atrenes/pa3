#include "clock.h"

timestamp_t time = 0;

timestamp_t logical_time_choose(timestamp_t new_time) {
    if (time > new_time) {
        return time = time;
    }
    return time = new_time;
}

timestamp_t logical_time_increment(void) {
    time += 1;
    return time;
}

timestamp_t get_lamport_time(void) {
    return time;
}
