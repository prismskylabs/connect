/*
 * Copyright (C) 2017 Prism Skylabs
 */
#ifndef CONNECT_SDK_CURLWRAPPER_H
#define CONNECT_SDK_CURLWRAPPER_H

#include "common-types.h"
#include "curl/curl.h"
#include "util.h"

namespace prism
{
namespace connect {

struct CurlCallbacks
{
    // should they all be pure virtual or with empty implementation, or mix?
    virtual size_t writeFunction(void* ptr, size_t size, size_t nmemb) = 0;
    virtual size_t headerFunction(void* ptr, size_t size, size_t nmemb) = 0;

    virtual ~CurlCallbacks()
    {
    }
};

struct CurlFactory
{
    virtual CURL* create() = 0;
    virtual void destroy(CURL*) = 0;

    virtual ~CurlFactory()
    {
    }
};

typedef boost::shared_ptr<CurlFactory> CurlFactoryPtr;

class CurlWrapper;
typedef unique_ptr<CurlWrapper>::t CurlWrapperPtr;

// incapsulates request and response
class CurlWrapper : public CurlCallbacks
{
public:
    CurlWrapper()
        : curl_(0)
        , httpHeader_(0)
        , post_(0)
        , last_(0)
    {
    }

    // must be called before any other function call
    bool init(CurlFactoryPtr curlFactory);

    virtual ~CurlWrapper();

    CURLcode httpGet(const std::string& url)
    {
        curl_easy_setopt(curl_, CURLOPT_HTTPGET, 1);
        return performRequest(url);
    }

    CURLcode httpPost(const std::string& url, CString postField)
    {
        curl_easy_setopt(curl_, CURLOPT_COPYPOSTFIELDS, postField.ptr());
        return performRequest(url);
    }

    // overwrites existing headers, if any
    void  setHeader(CString header)
    {
        if (httpHeader_)
        {
            curl_slist_free_all(httpHeader_);
            httpHeader_ = 0;
        }

        addHeader(header);
    }

    void addHeader(CString header)
    {
        httpHeader_ = curl_slist_append(httpHeader_, header);
    }

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

    void addFormFile(CString key, CString filePath, CString mimeType)
    {
        curl_formadd(&post_, &last_,
                     CURLFORM_COPYNAME, key.ptr(),
                     CURLFORM_FILE, filePath.ptr(),
                     CURLFORM_CONTENTTYPE, mimeType.ptr(),
                     CURLFORM_END);
    }

    void addFormFile(CString key, const void* data, size_t dataSize, CString mimeType)
    {
        curl_formadd(&post_, &last_,
                     CURLFORM_COPYNAME, key.ptr(),
                     CURLFORM_BUFFER, "dummyname",
                     CURLFORM_BUFFERPTR, data,
                     CURLFORM_BUFFERLENGTH, dataSize,
                     CURLFORM_CONTENTTYPE, mimeType.ptr(),
                     CURLFORM_END);
    }

    CURLcode httpPostForm(const std::string& url);

    const std::string& getResponseBodyAsString() const
    {
        return responseBody_;
    }

    long getResponseCode() const
    {
        return responseCode_;
    }

    void setConnectionTimeoutMs(long timeoutMs)
    {
        curl_easy_setopt(curl_, CURLOPT_CONNECTTIMEOUT, timeoutMs);
    }

    void setLowSpeed(long lowSpeedTime, long lowSpeedLimit)
    {
        curl_easy_setopt(curl_, CURLOPT_LOW_SPEED_TIME, lowSpeedTime);
        curl_easy_setopt(curl_, CURLOPT_LOW_SPEED_LIMIT, lowSpeedLimit);
    }

    void setSslVerifyPeer(bool sslVerifyPeer)
    {
        curl_easy_setopt(curl_, CURLOPT_SSL_VERIFYPEER, long(sslVerifyPeer ? 1 : 0));
    }

    void setAuthMethod(long bitmask)
    {
        curl_easy_setopt(curl_, CURLOPT_HTTPAUTH, bitmask);
    }

    void setUserPwd(const std::string& userpwd)
    {
        curl_easy_setopt(curl_, CURLOPT_USERPWD, userpwd.c_str());
    }

    void setUsername(const std::string& username)
    {
        curl_easy_setopt(curl_, CURLOPT_USERNAME, username.c_str());
    }

    void setPassword(const std::string& password)
    {
        curl_easy_setopt(curl_, CURLOPT_PASSWORD, password.c_str());
    }

    operator bool() const
    {
        return curl_;
    }

    operator CURL*() const
    {
        return curl_;
    }

    void setProxy(const std::string& proxy)
    {
        proxy_ = proxy;
    }

    void setCaBundlePath(const std::string& caBundlePath)
    {
        curl_easy_setopt(curl_, CURLOPT_CAINFO, caBundlePath.c_str());
    }

protected:
    virtual CURLcode performRequest(CString url);

private:
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

    virtual size_t writeFunction(void* ptr, size_t size, size_t nmemb)
    {
        responseBody_.append((char*) ptr, size * nmemb);
        return size * nmemb;
    }

    virtual size_t headerFunction(void* ptr, size_t size, size_t nmemb)
    {
        responseHeaders_.append((char*) ptr, size * nmemb);
        return size * nmemb;
    }

    CURL* curl_;
    struct curl_slist* httpHeader_;
    struct curl_httppost* post_;
    struct curl_httppost* last_;
    long responseCode_;
    std::string responseBody_;
    std::string responseHeaders_;
    CurlFactoryPtr curlFactory_;
    std::string proxy_;
};

}
}

#endif // CONNECT_SDK_CURLWRAPPER_H
