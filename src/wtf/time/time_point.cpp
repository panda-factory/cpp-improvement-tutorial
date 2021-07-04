//
// Created by admin on 2021/1/10.
//

#include "time_point.h"

namespace wtf {

TimePoint TimePoint::Now() {
    // The base time is arbitrary; use the clock epoch for convenience.
    const auto elapsed_time = std::chrono::steady_clock::now().time_since_epoch();
    return TimePoint(
            std::chrono::duration_cast<std::chrono::nanoseconds>(elapsed_time)
                    .count());
}
} // namespace wtf