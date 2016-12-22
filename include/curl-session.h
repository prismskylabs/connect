#ifndef CONNECT_SDK_CURL_SESSION_H
#define CONNECT_SDK_CURL_SESSION_H

#include "common-types.h"
#include "curl/curl.h"

namespace prism {
namespace connect {

struct CurlCallbacks {
    // should they all be pure virtual or with empty implementation, or mix?
    virtual size_t writeFunction(void* ptr, size_t size, size_t nmemb) = 0;
    virtual size_t headerFunction(void* ptr, size_t size, size_t nmemb) = 0;
};

class CurlSession;
typedef unique_ptr<CurlSession> CurlSessionPtr;

// incapsulates request and response
class CurlSession : public CurlCallbacks {
public:
    static CurlSessionPtr create(const string& token);

    ~CurlSession();

    CURLcode httpGet(const string& url);

    CURLcode httpPost(const string& url);

    const string& getResponseBodyAsString() const {
        return responseBody_;
    }

private:
    CurlSession();
    bool init(const string& token);

    CURLcode performRequest(const string& url);

    static size_t writeFunctionThunk(void* ptr, size_t size, size_t nmemb,
                                     CurlCallbacks* callbacks) {
        return callbacks->writeFunction(ptr, size, nmemb);
    }

    static size_t headerFunctionThunk(void *ptr, size_t size, size_t nmemb,
                                      CurlCallbacks* callbacks) {
        return callbacks->headerFunction(ptr, size, nmemb);
    }

    // allows for better control over transfer state
    virtual size_t writeFunction(void* ptr, size_t size, size_t nmemb);
    virtual size_t headerFunction(void* ptr, size_t size, size_t nmemb);

    CURL* curl_;
    struct curl_slist* authHeader_;
    string responseBody_;
    string responseHeaders_;
};


}
}

#endif // CONNECT_SDK_CURL_SESSION_H
