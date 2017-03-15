/*
 * Copyright (C) 2017 Prism Skylabs
 */
#ifndef CONNECT_SDK_ARTIFACT_UPLOADER_H
#define CONNECT_SDK_ARTIFACT_UPLOADER_H

#include "domain-types.h"
#include "boost/shared_ptr.hpp"
#include "public-util.h"

namespace prism
{
namespace connect
{

class Client;
// Usage example
// namespace prc = prism::connect;
// void configCallback(prc::Client& client)
// {
//     client.setLogFlags(prc::Client::LOG_INPUT | prc::Client::LOG_INPUT_JSON);
//     client.setConnectionTimeoutMs(5000);
//     client.setLowSpeed(5);
// }
//
// prc::ArtifactUploader::Configuration uploaderConfig("<apiRoot>", "<apiToken>",
//      "camOne", 32);
// prc::ArtifactUploader uploader;
// prc::Status status = uploader.init(uploaderConfig, configCallback);
// if (status.isError())
//     abort();
// uploader.uploadBackground(<timestamp>, PayloadAu::makeByMovingData(<data>, <mimeType>));

//class PayloadAu;
//typedef boost::shared_ptr<PayloadAu> PayloadAuPtr;

// TODO: other options for class name: SmartPayload, PayloadV2, PayloadTwo
// This class is carefully designed to take ownership and clean data
// after use.
// The moment, when data is copied or stolen is well-defined: in a call
// to make* static method.
// The moment, when owned data or file is deleted is uknknown: whenever
// PayloadAu instance is destroyed.
// Copying and assignment are explicitly prohibited to prevent unintended
// losing of ownership or data copying.
// This class is intentionally made distinct from Payload struct to keep
// current interface using Payload unchanged.
class PayloadAu : private boost::noncopyable
{
public:
    static PayloadAuPtr makeByMovingData(move_ref<ByteBuffer> data, const std::string& mimeType);
    static PayloadAuPtr makeByCopyingData(const void* data, size_t dataSize, const std::string& mimeType);

    // This method is intentionally not implemented, as it isn't good for
    // async interface
    // PayloadAuPtr makeByReferencingData();

    // This method is intentionally not implemented, as it isn't good for
    // async interface
    // PayloadAuPtr makeByReferencingFile();

    // file will be deleted on PayloadAu destruction
    static PayloadAuPtr makeByReferencingFileAutodelete(const std::string& filePath);

    bool isFile() const;
    std::string getFilePath() const;
    std::string getMimeType() const;
    const uint8_t* getData() const;
    size_t getDataSize() const;

private:
    PayloadAu();

    class Impl;
    unique_ptr<Impl>::t pImpl_;
};

class ArtifactUploader
{
public:
    typedef void (*ClientConfigCallback)(Client& client);

    struct Configuration
    {
        Configuration(const std::string& apiRoot,
                      const std::string& apiToken,
                      const std::string& cameraName,
                      float maxQueueSizeMB,
                      float warnQueueSizeMB,
                      const std::string& queueType = "simple",
                      int timeoutToCompleteUploadSec = 0)
            : apiRoot(apiRoot)
            , apiToken(apiToken)
            , cameraName(cameraName)
            , maxQueueSizeMB(maxQueueSizeMB)
            , queueType(queueType)
            , timeoutToCompleteUploadSec(timeoutToCompleteUploadSec)
        {
        }

        std::string apiRoot;
        std::string apiToken;
        std::string cameraName;
        float maxQueueSizeMB;
        float warnQueueSizeMB;
        std::string queueType; // "simple"
        int timeoutToCompleteUploadSec; // 0
    };

    ArtifactUploader();

    // synchronous, may block
    // configCallback will be called to let config client to e.g. set connection timeouts,
    // logging flags and similar. This will happen once and may be called from a thread
    // different than the one, where init() is called
    // configCallback may be NULL, in which case no configuration is performed
    Status init(const Configuration& cfg, ClientConfigCallback* configCallback);

//    ~ArtifactUploader();

    // all upload* methods are asynchronous, non-blocking, take ownership
    // of data passed to them and/or copy data e.g. timestamp_t, Flipbook, ObjectStream
    void uploadBackground(const timestamp_t& timestamp, PayloadAuPtr payload);
    void uploadObjectStream(const ObjectStream& stream, PayloadAuPtr payload);
    void uploadFlipbook(const Flipbook& flipbook, PayloadAuPtr payload);
    void uploadEvent(const timestamp_t& timestamp, move_ref<Events> events);

private:
    class Impl;
    unique_ptr<Impl>::t pImpl_;
};

} // namespace connect
} // namespace prism

#endif // CONNECT_SDK_ARTIFACT_UPLOADER_H
