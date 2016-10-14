#ifndef PRISM_CONNECT_UTIL_Time_H_
#define PRISM_CONNECT_UTIL_Time_H_

#include <chrono>
#include <string>

namespace prism {
namespace connect {
namespace util {

std::string IsoTime(const std::chrono::system_clock::time_point& time_point);

} // namespace util
} // namespace connect
} // namespace prism

#endif // PRISM_CONNECT_UTIL_Time_H_
