/*
 * Copyright (C) 2016-2017 Prism Skylabs
 */
#include "private/curl-session.h"
#include "rapidjson/document.h"
#include "private/const-strings.h"
#include "easylogging++.h"
#include "boost/noncopyable.hpp"
#include "boost/make_shared.hpp"

namespace prism
{
namespace connect
{

class CurlHandlesPool : boost::noncopyable
{
public:
    // maxPoolSize of zero means no limit
    explicit CurlHandlesPool(size_t maxPoolSize = 0)
        : numExistingHandles_(0)
        , maxPoolSize_(maxPoolSize)
    {
        if (maxPoolSize > 0)
            availableHandles_.reserve(maxPoolSize);
    }

    ~CurlHandlesPool()
    {
        // it may be too late to clear here as libCURL may be already uninitialized
        clear();
    }

    CURL* acquireHandle()
    {
        if (maxPoolSize_  &&  numExistingHandles_ >= maxPoolSize_)
        {
            LOG(INFO) << __FUNCTION__ << ": pool is full and has "
                      << numExistingHandles_  << " items";
            return 0;
        }

        if (availableHandles_.empty())
        {
            CURL* rv = curl_easy_init();

            if (rv)
                ++numExistingHandles_;

            return rv;
        }

        CURL* rv = availableHandles_.back();
        availableHandles_.pop_back();

        return rv;
    }

    void returnHandle(CURL* handle)
    {
        if (!handle)
            return;

        if (numExistingHandles_ < 1)
            LOG(INFO) << __FUNCTION__ << ": unexpected handle return";

        curl_easy_reset(handle);
        availableHandles_.push_back(handle);
    }

    void clear()
    {
        if (availableHandles_.size() != numExistingHandles_)
            LOG(ERROR) << __FUNCTION__ << ": not all handles were returned"
                       << ": " << numExistingHandles_ << " were created "
                       << ", " << availableHandles_.size() << " were returned";

        while (!availableHandles_.empty())
        {
            curl_easy_cleanup(availableHandles_.back());
            availableHandles_.pop_back();
        }

        numExistingHandles_ = 0;
    }

private:
    std::vector<CURL*> availableHandles_;
    size_t numExistingHandles_;
    size_t maxPoolSize_;
};

class PoolBasedCurlFactory : public CurlFactory, boost::noncopyable
{
public:
    virtual CURL* create()
    {
        return pool_.acquireHandle();
    }

    virtual void destroy(CURL* handle)
    {
        pool_.returnHandle(handle);
    }

private:
    CurlHandlesPool pool_;
};

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
    if ( !CurlWrapper::init(boost::make_shared<PoolBasedCurlFactory>()) )
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
