/*
 * Copyright (C) 2017 Prism Skylabs
 */
#ifndef CONNECT_SDK_CURL_SESSION_H
#define CONNECT_SDK_CURL_SESSION_H

#include "private/curl-wrapper.h"

namespace prism
{
namespace connect
{

class CurlSession;
typedef unique_ptr<CurlSession>::t CurlSessionPtr;

// tailored to work with connect web API
class CurlSession : public CurlWrapper
{
public:
    static CurlSessionPtr create(const std::string& token);

    virtual ~CurlSession();

    const std::string& getErrorMessage() const
    {
        return errorMessage_;
    }

    CURLcode performRequest(CString url);

private:
    bool init(const std::string& token);

    void parseResponseForMessage();

    std::string errorMessage_; // if response code is 4??
};

}
}

#endif // CONNECT_SDK_CURL_SESSION_H
