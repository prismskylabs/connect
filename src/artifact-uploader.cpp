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
    Status uploadBackground(const timestamp_t& timestamp, PayloadHolderPtr payload);
    Status uploadObjectStream(const ObjectStream& stream, PayloadHolderPtr payload);
    Status uploadFlipbook(const Flipbook& flipbook, PayloadHolderPtr payload);
    Status uploadEvent(const timestamp_t& timestamp, move_ref<Events> data);
    Status uploadCount(move_ref<Counts> counts);

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
    return impl().init(cfg, configCallback);
}

ArtifactUploader::~ArtifactUploader()
{
}

Status ArtifactUploader::uploadBackground(const timestamp_t& timestamp, PayloadHolderPtr payload)
{
    return impl().uploadBackground(timestamp, payload);
}

Status ArtifactUploader::uploadObjectStream(const ObjectStream& stream, PayloadHolderPtr payload)
{
    return impl().uploadObjectStream(stream, payload);
}

Status ArtifactUploader::uploadFlipbook(const Flipbook& flipbook, PayloadHolderPtr payload)
{
    return impl().uploadFlipbook(flipbook, payload);
}

Status ArtifactUploader::uploadEvent(const timestamp_t& timestamp, move_ref<Events> events)
{
    return impl().uploadEvent(timestamp, events);
}

Status ArtifactUploader::uploadCount(move_ref<Counts> counts)
{
    return impl().uploadCount(counts);
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

Status ArtifactUploader::Impl::uploadBackground(const timestamp_t& timestamp, PayloadHolderPtr payload)
{
    return uploadTaskQueuer_->addBackgroundTask(boost::make_shared<UploadBackgroundTask>(timestamp, payload));
}

Status ArtifactUploader::Impl::uploadObjectStream(const ObjectStream& stream, PayloadHolderPtr payload)
{
    return uploadTaskQueuer_->addObjectStreamTask(boost::make_shared<UploadObjectStreamTask>(stream, payload));
}

Status ArtifactUploader::Impl::uploadFlipbook(const Flipbook& flipbook, PayloadHolderPtr payload)
{
    return uploadTaskQueuer_->addFlipbookTask(boost::make_shared<UploadFlipbookTask>(flipbook, payload));
}

Status ArtifactUploader::Impl::uploadEvent(const timestamp_t& timestamp, move_ref<Events> data)
{
    return uploadTaskQueuer_->addEventTask(boost::make_shared<UploadEventTask>(timestamp, data));
}

Status ArtifactUploader::Impl::uploadCount(move_ref<Counts> counts)
{
    return uploadTaskQueuer_->addCountTask(boost::make_shared<UploadCountTask>(counts));
}

} // namespace connect
} // namespace prism
