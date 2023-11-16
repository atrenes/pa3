#ifndef RASPRED2_CLOCK_H
#define RASPRED2_CLOCK_H

#include "pa2345.h"

timestamp_t logical_time_increment(void);
timestamp_t logical_time_choose(timestamp_t new_time);

#endif //RASPRED2_CLOCK_H
