/*
 * Copyright (C) 2016-2017 Prism Skylabs
 */
#include "private/curl-session.h"
#include "rapidjson/document.h"
#include "private/const-strings.h"
#include "easylogging++.h"
#include "boost/noncopyable.hpp"
#include "boost/make_shared.hpp"
#include "private/PoolBasedCurlFactory.h"

namespace prism
{
namespace connect
{

CurlSessionPtr CurlSession::create(const std::string& token)
{
    CurlSessionPtr rv(new CurlSession());
    CurlSession& ref = *rv;

    if (ref.init(token))
        return boost::move(rv);

    return CurlSessionPtr();
}

CurlSession::~CurlSession()
{
}

bool CurlSession::init(const std::string& token)
{
    if ( !CurlWrapper::init(boost::make_shared<prism::PoolBasedCurlFactory>()) )
        return false;

    setHeader(std::string("Authorization: Token ").append(token));

    return true;
}

CURLcode CurlSession::performRequest(CString url)
{
    CURLcode rv = CurlWrapper::performRequest(url);

    errorMessage_.clear();
    long responseCode = getResponseCode();

    if (responseCode >= 400  &&  responseCode < 500)
        parseResponseForMessage();

    return rv;
}

void CurlSession::parseResponseForMessage()
{
    rapidjson::Document doc;

    if (doc.Parse(getResponseBodyAsString().c_str()).HasParseError())
    {
        LERROR << "Error parsing response for message";
        errorMessage_ = "Failed to parse response body";
        return;
    }

    if (doc.HasMember(kStrErrorMessages) && doc[kStrErrorMessages].IsArray())
    {
        rapidjson::Value& messages = doc[kStrErrorMessages];

        for (rapidjson::SizeType i = 0; i < messages.Size(); ++i)
            if (messages[i].IsString())
            {
                if (i > 0)
                    errorMessage_ += '\n';

                errorMessage_ += messages[i].GetString();
            }
    }
    else
        errorMessage_ = "Failed to get error message from response body";
}

}
}
