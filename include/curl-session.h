/*
 * Copyright (C) 2016-2017 Prism Skylabs
 */
#ifndef CONNECT_SDK_CURL_SESSION_H
#define CONNECT_SDK_CURL_SESSION_H

#include "common-types.h"
#include "curl/curl.h"
#include "util.h"

namespace prism
{
namespace connect
{

struct CurlCallbacks
{
    // should they all be pure virtual or with empty implementation, or mix?
    virtual size_t writeFunction(void* ptr, size_t size, size_t nmemb) = 0;
    virtual size_t headerFunction(void* ptr, size_t size, size_t nmemb) = 0;

    virtual ~CurlCallbacks()
    {
    }
};

class CurlSession;
typedef unique_ptr<CurlSession>::t CurlSessionPtr;

// incapsulates request and response
class CurlSession : public CurlCallbacks
{
public:
    static CurlSessionPtr create(const std::string& token);

    ~CurlSession();

    CURLcode httpGet(const std::string& url);

    CURLcode httpPost(const std::string& url, CString postField);

    void addHeader(CString header);

    void addFormField(CString key, CString value)
    {
        curl_formadd(&post_, &last_,
                     CURLFORM_COPYNAME, key.ptr(),
                     CURLFORM_COPYCONTENTS, value.ptr(),
                     CURLFORM_END);
    }

    void addFormField(CString key, CString value, CString mimeType)
    {
        curl_formadd(&post_, &last_,
                     CURLFORM_COPYNAME, key.ptr(),
                     CURLFORM_COPYCONTENTS, value.ptr(),
                     CURLFORM_CONTENTTYPE, mimeType.ptr(),
                     CURLFORM_END);
    }

    void addFormFile(CString key, CString filePath, CString mimeType);
    void addFormFile(CString key, const void* data, size_t dataSize, CString mimeType);

    CURLcode httpPostForm(const std::string& url);

    const std::string& getResponseBodyAsString() const
    {
        return responseBody_;
    }

    long getResponseCode() const
    {
        return responseCode_;
    }

    const std::string& getErrorMessage() const
    {
        return errorMessage_;
    }

private:
    CurlSession();
    bool init(const std::string& token);

    CURLcode performRequest(CString url);

    static size_t writeFunctionThunk(void* ptr, size_t size, size_t nmemb,
                                     CurlCallbacks* callbacks)
    {
        return callbacks->writeFunction(ptr, size, nmemb);
    }

    static size_t headerFunctionThunk(void *ptr, size_t size, size_t nmemb,
                                      CurlCallbacks* callbacks)
    {
        return callbacks->headerFunction(ptr, size, nmemb);
    }

    void parseResponseForMessage();

    // allows for better control over transfer state
    virtual size_t writeFunction(void* ptr, size_t size, size_t nmemb);
    virtual size_t headerFunction(void* ptr, size_t size, size_t nmemb);

    CURL* curl_;
    struct curl_slist* authHeader_;
    struct curl_httppost* post_;
    struct curl_httppost* last_;
    long responseCode_;
    std::string errorMessage_; // if response code is 4??
    std::string responseBody_;
    std::string responseHeaders_;
};

}
}

#endif // CONNECT_SDK_CURL_SESSION_H
