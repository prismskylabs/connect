/*
 * Copyright (C) 2016-2017 Prism Skylabs
 */
#ifndef CONNECT_SDK_COMMON_TYPES_H
#define CONNECT_SDK_COMMON_TYPES_H

#include <boost/move/unique_ptr.hpp>
#include <boost/chrono/system_clocks.hpp>

namespace prism
{
namespace connect
{

template <typename T>
struct unique_ptr
{
    typedef boost::movelib::unique_ptr<T> t;
};

// number of milliseconds since epoch, local time
typedef int64_t timestamp_t;

}
}

#endif // CONNECT_SDK_COMMON_TYPES_H
