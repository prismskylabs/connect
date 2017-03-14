/*
 * Copyright (C) 2017 Prism Skylabs
 */
#include "private/ArtifactUploadHelper.h"
#include "easylogging++.h"
#include "private/UploadArtifactTask.h"
#include "boost/format.hpp"
#include "public-util.h"
#include "artifact-uploader.h"

namespace prism
{
namespace connect
{

namespace prc = prism::connect;

bool UploadBackgroundTask::execute(ArtifactUploadHelper* uploader) const
{
    return uploader->uploadBackground(timestamp_,
                                      image_->isFile() ? Payload(image_->getFilePath())
                                        : Payload(image_->getData(), image_->getDataSize(), image_->getMimeType()));
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

bool UploadObjectStreamTask::execute(ArtifactUploadHelper* uploader) const
{
    return uploader->uploadObjectStream(stream_,
                                        image_->isFile() ? Payload(image_->getFilePath())
                                          : Payload(image_->getData(), image_->getDataSize(), image_->getMimeType()));
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

bool UploadFlipbookTask::execute(ArtifactUploadHelper* uploader) const
{
    return uploader->uploadFlipbook(flipbook_,
                                    data_->isFile() ? Payload(data_->getFilePath())
                                      : Payload(data_->getData(), data_->getDataSize(), data_->getMimeType()));
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

bool UploadEventTask::execute(ArtifactUploadHelper* uploader) const
{
    return uploader->uploadEvent(timestamp_, data_);
}

std::string UploadEventTask::toString() const
{
    return (boost::format("Event: (timestamp: %s)") % prc::toString(timestamp_)).str();
}

} // namespace connect
} // namespace prism
