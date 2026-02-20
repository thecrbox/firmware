#pragma once

#include "esphome/components/time/real_time_clock.h"

namespace time_helpers {

inline bool is_day_mode(esphome::ESPTime time, esphome::ESPTime day_start, esphome::ESPTime night_start) {
    if (!time.is_valid()) {
        return true; // Assume day mode if time is not valid
    }
    if (day_start.hour < night_start.hour || (day_start.hour == night_start.hour && day_start.minute < night_start.minute)) {
        // Normal case: day starts before night (e.g., 07:00-22:00)
        return (time.hour > day_start.hour || (time.hour == day_start.hour && time.minute >= day_start.minute)) &&
               (time.hour < night_start.hour || (time.hour == night_start.hour && time.minute < night_start.minute));
    } else {
        // Special case: day starts after night (overnight, e.g., 22:00-07:00 is night)
        return (time.hour > day_start.hour || (time.hour == day_start.hour && time.minute >= day_start.minute)) ||
               (time.hour < night_start.hour || (time.hour == night_start.hour && time.minute < night_start.minute));
    }
}

} // namespace time_helpers
