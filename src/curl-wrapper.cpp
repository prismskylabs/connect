/*
 * Copyright (C) 2017 Prism Skylabs
 */
#include "private/curl-wrapper.h"
#include "easylogging++.h"

namespace prism
{
namespace connect
{

bool CurlWrapper::init(CurlFactoryPtr curlFactory)
{
    if (curl_)
    {
        LOG(INFO) << "CurlWrapper::init(): already inited";
        return true;
    }

    if (!curlFactory)
        return false;

    curl_ = curlFactory->create();

    if (!curl_)
        return false;

    curlFactory_ = curlFactory;

    curl_easy_setopt(curl_, CURLOPT_FOLLOWLOCATION, 1);
    curl_easy_setopt(curl_, CURLOPT_MAXREDIRS, 10);

    curl_easy_setopt(curl_, CURLOPT_WRITEFUNCTION, writeFunctionThunk);
    curl_easy_setopt(curl_, CURLOPT_WRITEDATA, (CurlCallbacks*)this);
    curl_easy_setopt(curl_, CURLOPT_HEADERFUNCTION, headerFunctionThunk);
    curl_easy_setopt(curl_, CURLOPT_HEADERDATA, (CurlCallbacks*)this);

    return true;
}

CurlWrapper::~CurlWrapper()
{
    if (curl_)
    {
        curlFactory_->destroy(curl_);
        curl_ = 0;
    }

    if (httpHeader_)
    {
        curl_slist_free_all(httpHeader_);
        httpHeader_ = 0;
    }
}

CURLcode CurlWrapper::httpPostForm(const std::string& url)
{
    curl_easy_setopt(curl_, CURLOPT_HTTPPOST, post_);
    CURLcode rv = performRequest(url);
    curl_formfree(post_);
    last_ = post_ = 0;
    return rv;
}

struct CurlPerformance
{
    CURLINFO info;
    const char* description;
};

CurlPerformance curlPerf[] =
{
    {CURLINFO_NAMELOOKUP_TIME, "Name lookup time, s: "},
    {CURLINFO_CONNECT_TIME, "Connect time, s: "},
    {CURLINFO_APPCONNECT_TIME, "App. connect time, s: "},
    {CURLINFO_PRETRANSFER_TIME, "Start transfer time, s: "},
    {CURLINFO_STARTTRANSFER_TIME, "Start transfer time, s: "},
    {CURLINFO_TOTAL_TIME, "Total time, s: "},
    {CURLINFO_REDIRECT_TIME, "Redirect time, s: "},
    {CURLINFO_SPEED_DOWNLOAD, "Download speed, bytes/s: "},
    {CURLINFO_SPEED_UPLOAD, "Upload speed, bytes/s: "}
};

CURLcode CurlWrapper::performRequest(CString url)
{
    if (httpHeader_)
        curl_easy_setopt(curl_, CURLOPT_HTTPHEADER, httpHeader_);

    if (!proxy_.empty())
        curl_easy_setopt(curl_, CURLOPT_PROXY, proxy_.c_str());

    responseBody_.clear();
    responseHeaders_.clear();
    curl_easy_setopt(curl_, CURLOPT_URL, url.ptr());
    CURLcode rv = curl_easy_perform(curl_);

    responseCode_ = 0;
    curl_easy_getinfo(curl_, CURLINFO_RESPONSE_CODE, &responseCode_);

#if DUMP_CURL_PERF_DATA
    size_t numEntries = sizeof(curlPerf)/sizeof(curlPerf[0]);
    double value;

    for (size_t i = 0; i < numEntries; ++i)
        if (curl_easy_getinfo(curl_, curlPerf[i].info, &value) == CURLE_OK)
        {
            LOG(DEBUG) << curlPerf[i].description << value;
        }
#endif

    double value = 0;
    if (curl_easy_getinfo(curl_, CURLINFO_SIZE_UPLOAD, &value) == CURLE_OK  &&  value > 0)
        LOG(DEBUG) << "Uploaded, bytes: " << value;

    return rv;
}

}
}
