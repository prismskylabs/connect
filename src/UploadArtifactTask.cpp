/*
 * Copyright (C) 2017 Prism Skylabs
 */
#include "private/ArtifactUploader.h"
#include "easylogging++.h"
#include "private/UploadArtifactTask.h"
#include "boost/format.hpp"
#include "public-util.h"

namespace prism
{
namespace connect
{

namespace prc = prism::connect;

bool UploadBackgroundTask::execute(ArtifactUploader* uploader) const
{
    return uploader->uploadBackground(timestamp_, prism::connect::Payload(image_->data(), image_->size(), "image/jpeg"));
}

size_t UploadBackgroundTask::getArtifactSize() const
{
    return sizeof(timestamp_) + image_->size();
}

std::string UploadBackgroundTask::toString() const
{
    return (boost::format("Background: (timestamp: %s)")
        % prc::toString(timestamp_)).str();
}

bool UploadObjectStreamTask::execute(ArtifactUploader* uploader) const
{
    return uploader->uploadObjectStream(stream_, prism::connect::Payload(image_->data(), image_->size(), "image/jpeg"));
}

size_t UploadObjectStreamTask::getArtifactSize() const
{
    return sizeof(stream_) + image_->size();
}

std::string UploadObjectStreamTask::toString() const
{
    return (boost::format("ObjectStream: (collected: %s, object_id: %d)")
        % prc::toString(stream_.collected)
        % stream_.objectId).str();
}

bool UploadFlipbookTask::execute(ArtifactUploader* uploader) const
{
    return uploader->uploadFlipbook(flipbook_, payload_);
}

size_t UploadFlipbookTask::getArtifactSize() const
{
    return sizeof(flipbook_) + payload_.fileName.size() + payload_.dataSize;
}

std::string UploadFlipbookTask::toString() const
{
    return (boost::format("Flipbook: (start_timestamp: %s, stop_timestamp: %s, file_name: %s)")
        % prc::toString(flipbook_.startTimestamp)
        % prc::toString(flipbook_.stopTimestamp)
        % payload_.fileName).str();
}

size_t UploadEventTask::getArtifactSize() const
{
    return sizeof(prism::connect::timestamp_t) * (data_.size() + 1);
}

bool UploadEventTask::execute(ArtifactUploader* uploader) const
{
    return uploader->uploadEvent(timestamp_, data_);
}

void UploadEventTask::addEvent(const prism::connect::Event& event)
{
    data_.push_back(event);
}

std::string UploadEventTask::toString() const
{
    return (boost::format("Event: (timestamp: %s)") % prc::toString(timestamp_)).str();
}

} // namespace connect
} // namespace prism
