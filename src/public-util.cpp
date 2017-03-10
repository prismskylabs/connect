/*
 * Copyright (C) 2017 Prism Skylabs
 */
#include "public-util.h"

namespace prism
{
namespace connect
{

timestamp_t toTimestamp(const boost::chrono::system_clock::time_point& timePoint)
{
    return boost::chrono::duration_cast<boost::chrono::milliseconds>(
                timePoint.time_since_epoch()).count();
}

} // namespce connect
} // namespace prism
