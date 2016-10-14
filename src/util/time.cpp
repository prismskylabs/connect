#include "util/time.h"

#include <chrono>
#include <ctime>
#include <string>

namespace prism {
namespace connect {
namespace util {

static const char* const FULL_TIME_FORMAT = "%Y-%m-%dT%H:%M:%S";
static const size_t FULL_TIME_STRING_LENGTH = 20; // Length of "2016-02-08T16:15:20\0"

std::string IsoTime(const std::chrono::system_clock::time_point& time_point) {
    const auto time = std::chrono::system_clock::to_time_t(time_point);
    const auto utc_time = std::gmtime(&time);
    char time_buffer[FULL_TIME_STRING_LENGTH];
    strftime(time_buffer, FULL_TIME_STRING_LENGTH, FULL_TIME_FORMAT, utc_time);
    auto time_string = std::string{time_buffer};
    auto milliseconds = (time_point.time_since_epoch().count() / 1000LL) % 1000LL;
    time_string.append(".");
    time_string.append(std::to_string(milliseconds));;
    return time_string;
}

} // namespace util
} // namespace connect
} // namespace prism
