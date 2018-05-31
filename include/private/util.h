/*
 * Copyright (C) 2016-2017 Prism Skylabs
 *
 * This header is internal to SDK and isn't intended for use by SDK users.
 */
#ifndef CONNECT_SDK_UTIL_H
#define CONNECT_SDK_UTIL_H

#include "domain-types.h"

namespace prism
{
namespace connect
{
    std::string toJsonString(const Feed&);
    std::string toJsonString(const Background&, const std::string& /*contentType*/);
    std::string toJsonString(const Flipbook&, const std::string& /*contentType*/);
    std::string toJsonString(const ObjectSnapshot&, const std::string& /*contentType*/);
    std::string toJsonString(const Track&, const std::string& /*contentType*/);
    std::string toJsonString(const TimeSeries&, const std::string& /*contentType*/);

    std::string toString(int value);

    std::string mimeTypeFromFilePath(const std::string& fileName);
    std::string toIsoTimeString(const timestamp_t& timestamp);

    std::string toString(const Payload& payload);
    std::string toString(const Background& background);
    std::string toString(const Flipbook& flipbook);
    std::string toString(const Track& track);
    std::string toString(const ObjectSnapshot& snapshot);
    std::string toString(const TimeSeries& series);

    inline Status makeSuccess(int code = Status::SUCCESS, int facility = Status::FACILITY_NONE)
    {
        return Status(code, false, facility);
    }

    inline Status makeError(int code = Status::FAILURE, int facility = Status::FACILITY_NONE)
    {
        return Status(code, true, facility);
    }

    inline Status makeWebapiError(int code = Status::FAILURE)
    {
        return makeError(code, Status::FACILITY_WEBAPI);
    }

    inline Status makeNetworkError(int code = Status::FAILURE)
    {
        return makeError(code, Status::FACILITY_NETWORK);
    }

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
