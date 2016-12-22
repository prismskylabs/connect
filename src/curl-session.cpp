#include "curl-session.h"

namespace prism {
namespace connect {

CurlSessionPtr CurlSession::create(const string& token) {
    CurlSession* rawSession = new CurlSession();

    return CurlSessionPtr(rawSession->init(token) ? rawSession : nullptr);
}

CurlSession::~CurlSession()
{
    if (curl_ != nullptr) {
        curl_easy_cleanup(curl_);
        curl_ = nullptr;
    }

    if (authHeader_ != nullptr) {
        curl_slist_free_all(authHeader_);
        authHeader_ = nullptr;
    }
}

CURLcode CurlSession::httpGet(const string &url) {
    curl_easy_setopt(curl_, CURLOPT_HTTPGET, 1);
    return performRequest(url);
}

CurlSession::CurlSession()
    : curl_(nullptr)
    , authHeader_(nullptr)
{
}

bool CurlSession::init(const string& token) {
    // TODO abstract curl_easy_init()/cleanup(), so connection can be reused
    // by reusing CURL handle. There shall be abstraction for single- and multi-
    // thread, and possibly for multi CURL mode
    curl_ = curl_easy_init();

    if (!curl_)
        return false;

    // for debugging
    curl_easy_setopt(curl_, CURLOPT_VERBOSE, 1);

    curl_easy_setopt(curl_, CURLOPT_FOLLOWLOCATION, 1);
    curl_easy_setopt(curl_, CURLOPT_MAXREDIRS, 10);

    authHeader_ = curl_slist_append(authHeader_, string("Authorization: Token ").append(token).c_str());
    curl_easy_setopt(curl_, CURLOPT_HTTPHEADER, authHeader_);

    curl_easy_setopt(curl_, CURLOPT_WRITEFUNCTION, writeFunctionThunk);
    curl_easy_setopt(curl_, CURLOPT_WRITEDATA, (CurlCallbacks*)this);
    curl_easy_setopt(curl_, CURLOPT_HEADERFUNCTION, headerFunctionThunk);
    curl_easy_setopt(curl_, CURLOPT_HEADERDATA, (CurlCallbacks*)this);

    return true;
}

CURLcode CurlSession::performRequest(const string& url) {
    responseBody_.clear();
    responseHeaders_.clear();
    curl_easy_setopt(curl_, CURLOPT_URL, url.c_str());
    return curl_easy_perform(curl_);
}

size_t CurlSession::writeFunction(void *ptr, size_t size, size_t nmemb) {
    responseBody_.append((char*) ptr, size * nmemb);
    return size * nmemb;
}

size_t CurlSession::headerFunction(void *ptr, size_t size, size_t nmemb) {
    responseHeaders_.append((char*) ptr, size * nmemb);
    return size * nmemb;
}

}
}
