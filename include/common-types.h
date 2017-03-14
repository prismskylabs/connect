/*
 * Copyright (C) 2016-2017 Prism Skylabs
 */
#ifndef CONNECT_SDK_COMMON_TYPES_H
#define CONNECT_SDK_COMMON_TYPES_H

#include <boost/move/unique_ptr.hpp>
#include <boost/chrono/system_clocks.hpp>
#include "boost/shared_ptr.hpp"
#include <vector>

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

typedef std::vector<uint8_t> ByteBuffer;
typedef boost::shared_ptr<ByteBuffer> ByteBufferPtr;

} // namespace connect
} // namespace prism

#endif // CONNECT_SDK_COMMON_TYPES_H
