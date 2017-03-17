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
    ~Impl();

    Status init(const ArtifactUploader::Configuration& cfg,
                ArtifactUploader::ClientConfigCallback* configCallback);

    // all upload* methods are asynchronous, non-blocking, take ownership
    // of data passed to them
    void uploadBackground(const timestamp_t& timestamp, PayloadHolderPtr payload);
    void uploadObjectStream(const ObjectStream& stream, PayloadHolderPtr payload);
    void uploadFlipbook(const Flipbook& flipbook, PayloadHolderPtr payload);
    void uploadEvent(const timestamp_t& timestamp, move_ref<Events> data);

private:
    OutputControllerPtr outputController_;
    UploadTaskQueuerPtr uploadTaskQueuer_;
};

ArtifactUploader::ArtifactUploader()
    : pImpl_(new Impl())
{

}

Status ArtifactUploader::init(const ArtifactUploader::Configuration& cfg,
                              ClientConfigCallback configCallback)
{
    return pImpl_->init(cfg, configCallback);
}

ArtifactUploader::~ArtifactUploader()
{
}

void ArtifactUploader::uploadBackground(const timestamp_t& timestamp, PayloadHolderPtr payload)
{
    pImpl_->uploadBackground(timestamp, payload);
}

void ArtifactUploader::uploadObjectStream(const ObjectStream& stream, PayloadHolderPtr payload)
{
    pImpl_->uploadObjectStream(stream, payload);
}

void ArtifactUploader::uploadFlipbook(const Flipbook& flipbook, PayloadHolderPtr payload)
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
    if (cfg.maxQueueSize == 0)
    {
        LOG(ERROR) << "Invalid maxQueueSize value " << cfg.maxQueueSize;
        return makeError();
    }

    // cfg.warnQueueSize is always >= 0, as type is size_t
    if (cfg.queueType.compare(kStrSimple))
    {
        LOG(ERROR) << "Unsupported queue type: " << cfg.queueType;
        return makeError();
    }

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

    UploadQueuePtr uploadQueue = boost::make_shared<UploadQueue>(cfg.maxQueueSize, cfg.warnQueueSize);

    outputController_ = boost::make_shared<OutputController>(
                OutputController::Configuration(
                    uploadQueue,
                    boost::make_shared<ArtifactUploadHelper>(connect),
                    cfg.timeoutToCompleteUploadSec));

    uploadTaskQueuer_ = boost::make_shared<SimpleUploadTaskQueuer>(uploadQueue);

    outputController_->start();

    return makeSuccess();
}

void ArtifactUploader::Impl::uploadBackground(const timestamp_t& timestamp, PayloadHolderPtr payload)
{
    uploadTaskQueuer_->addBackgroundTask(boost::make_shared<UploadBackgroundTask>(timestamp, payload));
}

void ArtifactUploader::Impl::uploadObjectStream(const ObjectStream& stream, PayloadHolderPtr payload)
{
    uploadTaskQueuer_->addObjectStreamTask(boost::make_shared<UploadObjectStreamTask>(stream, payload));
}

void ArtifactUploader::Impl::uploadFlipbook(const Flipbook& flipbook, PayloadHolderPtr payload)
{
    uploadTaskQueuer_->addFlipbookTask(boost::make_shared<UploadFlipbookTask>(flipbook, payload));
}

void ArtifactUploader::Impl::uploadEvent(const timestamp_t& timestamp, move_ref<Events> data)
{
    uploadTaskQueuer_->addEventTask(boost::make_shared<UploadEventTask>(timestamp, data));
}

} // namespace connect
} // namespace prism
