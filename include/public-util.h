#ifndef CONNECT_SDK_PUBLICUTIL_H
#define CONNECT_SDK_PUBLICUTIL_H

#include "domain-types.h"

namespace prism
{
namespace connect
{
    timestamp_t toTimestamp(const boost::chrono::system_clock::time_point& timePoint);
    std::string toString(const prism::connect::timestamp_t& t);
} // namespace connect
} // namespace prism

#endif // CONNECT_SDK_PUBLICUTIL_H
