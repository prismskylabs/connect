/*
 * Copyright (C) 2016-2017 Prism Skylabs
 */
#include "curl-session.h"
#include "rapidjson/document.h"
#include "const-strings.h"
#include "easylogging++.h"

namespace prism
{
namespace connect
{

class CurlHandlersPool
{
public:
    static CurlHandlersPool& get()
    {
        static CurlHandlersPool pool;
        return pool;
    }

    CURL* aqcuireHandle()
    {
        if (availableHandles_.empty())
        {
            ++numExistingHandles_;
            return curl_easy_init();
        }

        CURL* rv = availableHandles_.back();
        availableHandles_.pop_back();

        return rv;
    }

    void returnHandle(CURL* handle)
    {
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
    CurlHandlersPool()
        : numExistingHandles_(0)
    {
    }

    ~CurlHandlersPool()
    {
        // it may be too late to clear here as libCURL may be already uninitialized
        clear();
    }

    std::vector<CURL*> availableHandles_;
    size_t numExistingHandles_;
};

CurlSessionPtr CurlSession::create(const std::string& token)
{
    CurlSession* rawSession = new CurlSession();

    return CurlSessionPtr(rawSession->init(token) ? rawSession : nullptr);
}

CurlSession::~CurlSession()
{
    if (curl_ != nullptr)
    {
        CurlHandlersPool::get().returnHandle(curl_);
        curl_ = nullptr;
    }

    if (authHeader_ != nullptr)
    {
        curl_slist_free_all(authHeader_);
        authHeader_ = nullptr;
    }
}

CURLcode CurlSession::httpGet(const std::string &url)
{
    curl_easy_setopt(curl_, CURLOPT_HTTPGET, 1);
    return performRequest(url);
}

CURLcode CurlSession::httpPost(const std::string& url, const std::string& postField)
{
    curl_easy_setopt(curl_, CURLOPT_COPYPOSTFIELDS, postField.c_str());
    return performRequest(url);
}

void CurlSession::addHeader(const std::string& header)
{
    authHeader_ = curl_slist_append(authHeader_, header.c_str());
}

void CurlSession::addFormFile(const char* key, const char* filePath, const char* mimeType)
{
    curl_formadd(&post_, &last_,
                 CURLFORM_COPYNAME, key,
                 CURLFORM_FILE, filePath,
                 CURLFORM_CONTENTTYPE, mimeType,
                 CURLFORM_END);
}

void CurlSession::addFormFile(const char* key, const void* data, size_t dataSize, const char* mimeType)
{
    curl_formadd(&post_, &last_,
                 CURLFORM_COPYNAME, key,
                 CURLFORM_BUFFER, "dummyname",
                 CURLFORM_BUFFERPTR, data,
                 CURLFORM_BUFFERLENGTH, dataSize,
                 CURLFORM_CONTENTTYPE, mimeType,
                 CURLFORM_END);
}

CURLcode CurlSession::httpPostForm(const std::string& url)
{
    curl_easy_setopt(curl_, CURLOPT_HTTPPOST, post_);
    CURLcode rv = performRequest(url);
    curl_formfree(post_);
    last_ = post_ = nullptr;
    return rv;
}

CurlSession::CurlSession()
    : curl_(nullptr)
    , authHeader_(nullptr)
    , post_(nullptr)
    , last_(nullptr)
{
}

bool CurlSession::init(const std::string& token)
{
    curl_ = CurlHandlersPool::get().aqcuireHandle();

    if (!curl_)
        return false;

    // for debugging
//    curl_easy_setopt(curl_, CURLOPT_STDERR, stdout);
//    curl_easy_setopt(curl_, CURLOPT_VERBOSE, 1);

    curl_easy_setopt(curl_, CURLOPT_FOLLOWLOCATION, 1);
    curl_easy_setopt(curl_, CURLOPT_MAXREDIRS, 10);

    authHeader_ = curl_slist_append(authHeader_, std::string("Authorization: Token ").append(token).c_str());
    curl_easy_setopt(curl_, CURLOPT_HTTPHEADER, authHeader_);

    curl_easy_setopt(curl_, CURLOPT_WRITEFUNCTION, writeFunctionThunk);
    curl_easy_setopt(curl_, CURLOPT_WRITEDATA, (CurlCallbacks*)this);
    curl_easy_setopt(curl_, CURLOPT_HEADERFUNCTION, headerFunctionThunk);
    curl_easy_setopt(curl_, CURLOPT_HEADERDATA, (CurlCallbacks*)this);

    return true;
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

CURLcode CurlSession::performRequest(const std::string& url)
{
    responseBody_.clear();
    responseHeaders_.clear();
    curl_easy_setopt(curl_, CURLOPT_URL, url.c_str());
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

    double value;
    if (curl_easy_getinfo(curl_, CURLINFO_SIZE_UPLOAD, &value) == CURLE_OK)
        LOG(DEBUG) << "Uploaded, bytes: " << value;

    errorMessage_.clear();

    if (responseCode_ >= 400  &&  responseCode_ < 500)
        parseResponseForMessage();

    return rv;
}

void CurlSession::parseResponseForMessage()
{
    rapidjson::Document doc;

    if (doc.Parse(getResponseBodyAsString().c_str()).HasParseError())
    {
        LERROR << "Error parsing response for message";
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

size_t CurlSession::writeFunction(void *ptr, size_t size, size_t nmemb)
{
    responseBody_.append((char*) ptr, size * nmemb);
    return size * nmemb;
}

size_t CurlSession::headerFunction(void *ptr, size_t size, size_t nmemb)
{
    responseHeaders_.append((char*) ptr, size * nmemb);
    return size * nmemb;
}

}
}
