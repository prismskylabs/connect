/*
 * Copyright (C) 2017 Prism Skylabs
 */
#include "public-util.h"
#include "boost/date_time/posix_time/posix_time.hpp"
#include "boost/date_time/local_time/local_time.hpp"

namespace prism
{
namespace connect
{

timestamp_t toTimestamp(const boost::chrono::system_clock::time_point& timePoint)
{
    return boost::chrono::duration_cast<boost::chrono::milliseconds>(
                timePoint.time_since_epoch()).count();
}

std::string toString(const prism::connect::timestamp_t& t)
{
    static const boost::posix_time::ptime epoch(boost::gregorian::date(1970, 1, 1));
    const boost::local_time::local_date_time time(epoch + boost::posix_time::milliseconds(t), boost::local_time::time_zone_ptr());

    return time.to_string();
}

} // namespce connect
} // namespace prism
