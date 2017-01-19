/*
 * Copyright (C) 2016-2017 Prism Skylabs
 *
 * This header is internal to SDK and isn't intended for use by SDK users.
 */
#ifndef CONNECT_SDK_UTIL_H
#define CONNECT_SDK_UTIL_H

#include "common-types.h"
#include "domain-types.h"

namespace prism
{
namespace connect
{
    std::string toJsonString(const Instrument&);
    std::string toJsonString(const Events&);
    std::string toJsonString(const ObjectStream&);

    std::string toString(int value);

    std::string mimeTypeFromFilePath(const std::string& fileName);
    std::string toIsoTimeString(const timestamp_t& timestamp);

    timestamp_t toTimestamp(const boost::chrono::system_clock::time_point& timePoint);

    // Prevents creating temporary std::string in case we need to pass const char*
    // into method/function. Also prevents using c_str() on caller side.
    // Example usage (note passing CString by value)
    // void myFunc(CString str);
    //
    // myFunc("C string");
    //
    // std::string str("std::string);
    // myFunc(str);
    class CString
    {
    public:
        CString(const char* str)
            : str_(str)
        {
        }

        CString(const std::string& str)
            : str_(str.c_str())
        {
        }

        operator const char*() const
        {
            return str_;
        }

        const char* ptr() const
        {
            return str_;
        }

    private:
        const char* str_;
    };
}
}

#endif // CONNECT_SDK_UTIL_H
