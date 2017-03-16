/*
 * Copyright (C) 2017 Prism Skylabs
 */
#include "public-util.h"
#include "boost/date_time/posix_time/posix_time.hpp"
#include "boost/date_time/local_time/local_time.hpp"
#include "boost/filesystem.hpp"
#include "easylogging++.h"

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

void removeFile(const std::string& filePath)
{
    try
    {
        boost::filesystem::remove(filePath);
    }
    catch (const std::exception& e)
    {
        LOG(ERROR) << "Error removing file " << filePath << ": " << e.what();
    }
    catch (...)
    {
        LOG(ERROR) << "Error removing file " << filePath;
    }
}

} // namespce connect
} // namespace prism
