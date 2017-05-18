/*
 * Copyright (C) 2017 Prism Skylabs
 */
#include "easylogging++.h"
#include "private/UploadArtifactTask.h"
#include "boost/format.hpp"
#include "public-util.h"
#include "artifact-uploader.h"
#include "client.h"

namespace prism
{
namespace connect
{

namespace prc = prism::connect;

Status UploadBackgroundTask::execute(ClientSession& session) const
{
    return session.client.uploadBackground(
                session.accountId, session.cameraId, timestamp_,
                image_->isFile() ? Payload(image_->getFilePath())
                                 : Payload(image_->getData(), image_->getDataSize(),
                                           image_->getMimeType()));
}

size_t UploadBackgroundTask::getArtifactSize() const
{
    return sizeof(timestamp_) +  sizeof(image_) + image_->getDataSize();
}

std::string UploadBackgroundTask::toString() const
{
    return (boost::format("Background: (timestamp: %s)")
        % prc::toString(timestamp_)).str();
}

Status UploadObjectStreamTask::execute(ClientSession& session) const
{
    return session.client.uploadObjectStream(
                session.accountId, session.cameraId, stream_,
                image_->isFile() ? Payload(image_->getFilePath())
                                 : Payload(image_->getData(), image_->getDataSize(),
                                           image_->getMimeType()));
}

size_t UploadObjectStreamTask::getArtifactSize() const
{
    return sizeof(stream_) + sizeof(image_) + image_->getDataSize();
}

std::string UploadObjectStreamTask::toString() const
{
    return (boost::format("ObjectStream: (collected: %s, object_id: %d)")
        % prc::toString(stream_.collected)
        % stream_.objectId).str();
}

Status UploadFlipbookTask::execute(ClientSession& session) const
{
    return session.client.uploadFlipbook(
                session.accountId, session.cameraId, flipbook_,
                data_->isFile() ? Payload(data_->getFilePath())
                                : Payload(data_->getData(), data_->getDataSize(),
                                          data_->getMimeType()));
}

size_t UploadFlipbookTask::getArtifactSize() const
{
    return sizeof(flipbook_) + sizeof(data_) + data_->getDataSize();
}

std::string UploadFlipbookTask::toString() const
{
    return (boost::format("Flipbook: (start_timestamp: %s, stop_timestamp: %s, file_name: %s)")
        % prc::toString(flipbook_.startTimestamp)
        % prc::toString(flipbook_.stopTimestamp)
        % data_->getFilePath()).str();
}

size_t UploadEventTask::getArtifactSize() const
{
    return sizeof(timestamp_t) * (data_.size() + 1);
}

Status UploadEventTask::execute(ClientSession& session) const
{
    return session.client.uploadEvent(session.accountId, session.cameraId, timestamp_, data_);
}

std::string UploadEventTask::toString() const
{
    return (boost::format("Event: (timestamp: %s)") % prc::toString(timestamp_)).str();
}

Status UploadCountTask::execute(ClientSession& session) const
{
    return session.client.uploadCount(session.accountId, session.cameraId, data_);
}

size_t UploadCountTask::getArtifactSize() const
{
    return sizeof(data_) + sizeof(Count) * data_.capacity();
}

std::string UploadCountTask::toString() const
{
    return "Counts";
}

} // namespace connect
} // namespace prism
