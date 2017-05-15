/*
 * Copyright (C) 2017 Prism Skylabs
 */
#ifndef PRISM_POOLBASEDCURLFACTORY_H
#define PRISM_POOLBASEDCURLFACTORY_H

#include "boost/noncopyable.hpp"
#include "private/curl-wrapper.h"

namespace prism
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

class PoolBasedCurlFactory : public prism::connect::CurlFactory, boost::noncopyable
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

}

#endif // PRISM_POOLBASEDCURLFACTORY_H
