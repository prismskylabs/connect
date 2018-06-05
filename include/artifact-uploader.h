/*
 * Copyright (C) 2017 Prism Skylabs
 */
#ifndef CONNECT_SDK_ARTIFACT_UPLOADER_H
#define CONNECT_SDK_ARTIFACT_UPLOADER_H

#include "domain-types.h"
#include "payload-holder.h"
#include "public-util.h"

namespace prism
{
namespace connect
{

class Client;

// For usage example see
// <ConnectSDK>/platforms/shared/tests/test-client/testArtifactUploader.cpp
class ArtifactUploader
{
public:
    typedef void (ClientConfigCallback)(Client& client);

    struct Configuration
    {
        Configuration(const std::string& apiRoot,
                      const std::string& apiToken,
                      const std::string& cameraName,
                      size_t maxQueueSize,
                      size_t warnQueueSize,
                      const std::string& queueType = "simple",
                      int timeoutToCompleteUploadSec = 0)
            : apiRoot(apiRoot)
            , apiToken(apiToken)
            , cameraName(cameraName)
            , maxQueueSize(maxQueueSize)
            , warnQueueSize(warnQueueSize)
            , queueType(queueType)
            , timeoutToCompleteUploadSec(timeoutToCompleteUploadSec)
        {
        }

        std::string apiRoot;
        std::string apiToken;
        std::string cameraName;
        size_t maxQueueSize;
        size_t warnQueueSize;
        std::string queueType; // "simple"
        int timeoutToCompleteUploadSec; // 0
    };

    ArtifactUploader();

    // synchronous, may block
    // configCallback will be called to let config client to e.g. set connection timeouts,
    // logging flags and similar. This will happen once and may be called from a thread
    // different than the one, where init() is called
    // configCallback may be NULL, in which case no configuration is performed
    Status init(const Configuration& cfg, ClientConfigCallback configCallback);

    // Need this, even if empty, because unique_ptr can't work with auto-generated
    // destructor for incomplete type Impl
    // See http://stackoverflow.com/questions/9954518/stdunique-ptr-with-an-incomplete-type-wont-compile
    ~ArtifactUploader();

    // all upload* methods are asynchronous, non-blocking, make copy of timestamp,
    // stream, flipbook
    Status uploadBackground(const Background& background, PayloadHolderPtr payload);
    Status uploadObjectSnapshot(const ObjectSnapshot& snapshot, PayloadHolderPtr payload);
    Status uploadFlipbook(const Flipbook& flipbook, PayloadHolderPtr payload);
    Status uploadTimeSeries(const TimeSeries& series);

    // Stop uploader thread ASAP, enqueued data won't be uploaded
    // Non-blocking, doesn't wait for thread actaully exiting only signals it to exit.
    void abort();

private:
    class Impl;
    unique_ptr<Impl>::t pImpl_;

    // using this instead of pImpl_-> enables autocomplete and go to definition in QtCreator
    Impl& impl() const
    {
        return *pImpl_;
    }
};

} // namespace connect
} // namespace prism

#endif // CONNECT_SDK_ARTIFACT_UPLOADER_H
