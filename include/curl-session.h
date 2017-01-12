/*
 * Copyright (C) 2016-2017 Prism Skylabs
 */
#ifndef CONNECT_SDK_CURL_SESSION_H
#define CONNECT_SDK_CURL_SESSION_H

#include "common-types.h"
#include "curl/curl.h"

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

    CURLcode httpPost(const std::string& url, const std::string& postField);

    void addHeader(const std::string& header);

    void addFormField(const char* key, const char* value)
    {
        curl_formadd(&post_, &last_,
                     CURLFORM_COPYNAME, key,
                     CURLFORM_COPYCONTENTS, value,
                     CURLFORM_END);
    }

    void addFormField(const char* key, const char* value, const char* mimeType)
    {
        curl_formadd(&post_, &last_,
                     CURLFORM_COPYNAME, key,
                     CURLFORM_COPYCONTENTS, value,
                     CURLFORM_CONTENTTYPE, mimeType,
                     CURLFORM_END);
    }

    void addFormField(const char* key, const std::string& value)
    {
        addFormField(key, value.c_str());
    }

    void addFormField(const char* key, const std::string& value, const char* mimeType)
    {
        addFormField(key, value.c_str(), mimeType);
    }

    void addFormField(const char* key, const char* value, const std::string& mimeType)
    {
        addFormField(key, value, mimeType.c_str());
    }

    void addFormField(const char* key, const std::string& value, const std::string& mimeType)
    {
        addFormField(key, value.c_str(), mimeType.c_str());
    }


    void addFormFile(const char* key, const char* filePath, const char* mimeType);
    void addFormFile(const char* key, const void* data, size_t dataSize, const char* mimeType);

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

    CURLcode performRequest(const std::string& url);

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
