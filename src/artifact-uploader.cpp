/*
 * Copyright (C) 2017 Prism Skylabs
 */
#include "artifact-uploader.h"
#include "private/UploadQueue.h"
#include "private/OutputController.h"
#include "private/UploadTaskQueuer.h"
#include "private/util.h"
#include "boost/make_shared.hpp"
#include "private/const-strings.h"
#include "easylogging++.h"
#include "boost/filesystem.hpp"

namespace prism
{
namespace connect
{

class ArtifactUploader::Impl
{
public:
    Impl();
    ~Impl();

    Status init(const ArtifactUploader::Configuration& cfg,
                ArtifactUploader::ClientConfigCallback* configCallback);

//    ~ArtifactUploader();

    // all upload* methods are asynchronous, non-blocking, take ownership
    // of data passed to them
    void uploadBackground(const timestamp_t& timestamp, PayloadAuPtr payload);
    void uploadObjectStream(const ObjectStream& stream, PayloadAuPtr payload);
    void uploadFlipbook(const Flipbook& flipbook, PayloadAuPtr payload);
    void uploadEvent(const timestamp_t& timestamp, move_ref<Events> data);

private:
    UploadQueuePtr uploadQueue_;
    OutputControllerPtr outputController_;
    UploadTaskQueuerPtr uploadTaskQueuer_;
};

ArtifactUploader::ArtifactUploader()
    : pImpl_(new Impl())
{

}

Status ArtifactUploader::init(const ArtifactUploader::Configuration& cfg,
                              ArtifactUploader::ClientConfigCallback* configCallback)
{
    return pImpl_->init(cfg, configCallback);
}

void ArtifactUploader::uploadBackground(const timestamp_t& timestamp, PayloadAuPtr payload)
{
    pImpl_->uploadBackground(timestamp, payload);
}

void ArtifactUploader::uploadObjectStream(const ObjectStream& stream, PayloadAuPtr payload)
{
    pImpl_->uploadObjectStream(stream, payload);
}

void ArtifactUploader::uploadFlipbook(const Flipbook& flipbook, PayloadAuPtr payload)
{
    pImpl_->uploadFlipbook(flipbook, payload);
}

void ArtifactUploader::uploadEvent(const timestamp_t& timestamp, move_ref<Events> events)
{
    pImpl_->uploadEvent(timestamp, events);
}

ArtifactUploader::Impl::~Impl()
{
    if (uploadTaskQueuer_)
        uploadTaskQueuer_->finalizeUpload();

    if (outputController_)
        outputController_->stop();
}

Status ArtifactUploader::Impl::init(const ArtifactUploader::Configuration& cfg,
                                    ArtifactUploader::ClientConfigCallback* configCallback)
{
    if (cfg.maxQueueSizeMB <= 1e-5)
    {
        LOG(ERROR) << "Invalid maxQueueSizeMB value " << cfg.maxQueueSizeMB;
        return makeError();
    }

    if (cfg.warnQueueSizeMB <= 1e-5)
    {
        LOG(ERROR) << "Invalid warnQueueSizeMB value " << cfg.warnQueueSizeMB;
    }

    uploadQueue_ = boost::make_shared<UploadQueue>(cfg.maxQueueSizeMB * 10e6, cfg.warnQueueSizeMB * 10e6);

    PrismConnectServicePtr connect(new PrismConnectService());
    PrismConnectService::Configuration serviceConfig;

    serviceConfig.apiRoot = cfg.apiRoot;
    serviceConfig.apiToken = cfg.apiToken;
    serviceConfig.cameraName = cfg.cameraName;

    int result = connect->init(serviceConfig);

    if (result)
    {
        LOG(ERROR) << "Failed to init Prism Connect service, result = " << result;
        return makeError();
    }

    outputController_ = boost::make_shared<OutputController>(
                OutputController::Configuration(
                    uploadQueue_,
                    boost::make_shared<ArtifactUploadHelper>(connect),
                    cfg.timeoutToCompleteUploadSec));

    if (!cfg.queueType.compare(kStrSimple))
    {
        LOG(ERROR) << "Unsupported queue type: " << cfg.queueType;
        return makeError();
    }

    uploadTaskQueuer_ = boost::make_shared<SimpleUploadTaskQueuer>(uploadQueue_);

    outputController_->start();

    return makeSuccess();
}

void ArtifactUploader::Impl::uploadBackground(const timestamp_t& timestamp, PayloadAuPtr payload)
{
    uploadTaskQueuer_->addBackgroundTask(boost::make_shared<UploadBackgroundTask>(timestamp, payload));
}

void ArtifactUploader::Impl::uploadObjectStream(const ObjectStream& stream, PayloadAuPtr payload)
{
    uploadTaskQueuer_->addObjectStreamTask(boost::make_shared<UploadObjectStreamTask>(stream, payload));
}

void ArtifactUploader::Impl::uploadFlipbook(const Flipbook& flipbook, PayloadAuPtr payload)
{
    uploadTaskQueuer_->addFlipbookTask(boost::make_shared<UploadFlipbookTask>(flipbook, payload));
}

void ArtifactUploader::Impl::uploadEvent(const timestamp_t& timestamp, move_ref<Events> data)
{
    uploadTaskQueuer_->addEventTask(boost::make_shared<UploadEventTask>(timestamp, data));
}

class PayloadAu::Impl
{
public:
    ~Impl();

    bool isFile() const
    {
        return !filePath_.empty();
    }

    std::string getFilePath() const
    {
        return filePath_;
    }

    std::string getMimeType() const
    {
        return mimeType_;
    }

    const uint8_t* getData() const
    {
        return buf_.data();
    }

    size_t getDataSize() const
    {
        return buf_.size();
    }

private:
    friend class PayloadAu;

    ByteBuffer buf_;
    std::string filePath_;
    std::string mimeType_;
};

PayloadAu::PayloadAu()
    : pImpl_(new Impl())
{
}

PayloadAuPtr PayloadAu::makeByMovingData(move_ref<ByteBuffer> data, const std::string& mimeType)
{
    PayloadAuPtr rv(new PayloadAu());
    std::swap(rv->pImpl_->buf_, data.ref);
    rv->pImpl_->mimeType_ = mimeType;

    return rv;
}

PayloadAuPtr PayloadAu::makeByCopyingData(const void* data, size_t dataSize, const std::string& mimeType)
{
    PayloadAuPtr rv(new PayloadAu());
    ByteBuffer& buf = rv->pImpl_->buf_;
    buf.reserve(dataSize);
    const uint8_t* dataStart = static_cast<const uint8_t*>(data);
    buf.insert(buf.end(), dataStart, dataStart + dataSize);
    rv->pImpl_->mimeType_ = mimeType;

    return rv;
}

PayloadAuPtr PayloadAu::makeByReferencingFileAutodelete(const std::string& filePath)
{
    PayloadAuPtr rv(new PayloadAu());
    rv->pImpl_->filePath_ = filePath;

    return rv;
}

bool PayloadAu::isFile() const
{
    return pImpl_->isFile();
}

std::string PayloadAu::getFilePath() const
{
    return pImpl_->getFilePath();
}

std::string PayloadAu::getMimeType() const
{
    return pImpl_->getMimeType();
}

const uint8_t* PayloadAu::getData() const
{
    return pImpl_->getData();
}

size_t PayloadAu::getDataSize() const
{
    return pImpl_->getDataSize();
}

PayloadAu::Impl::~Impl()
{
    if (isFile())
        boost::filesystem::remove(filePath_);
}

} // namespace connect
} // namespace prism
