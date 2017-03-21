/*
 * Copyright (C) 2017 Prism Skylabs
 */
#ifndef CONNECT_SDK_PUBLICUTIL_H
#define CONNECT_SDK_PUBLICUTIL_H

#include "domain-types.h"

namespace prism
{
namespace connect
{

timestamp_t toTimestamp(const boost::chrono::system_clock::time_point& timePoint);
std::string toString(const prism::connect::timestamp_t& t);

// borrowed from http://stackoverflow.com/questions/25507858/c03-moving-a-vector-into-a-class-member-through-constructor-move-semantics
template <typename T> struct move_ref
{
    explicit move_ref(T & ref)
        : ref(ref)
    {
    }

    T & ref;
};

template <typename T> move_ref<T> move(T & t)
{
    return move_ref<T>(t);
}

inline bool isNetworkError(const Status& status)
{
    return status.isError() && status.getFacility() == Status::FACILITY_NETWORK;
}

void removeFile(const std::string& filePath);

} // namespace connect
} // namespace prism

#endif // CONNECT_SDK_PUBLICUTIL_H
