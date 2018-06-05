/*
 * Copyright (C) 2017 Prism Skylabs
 */
#include "easylogging++.h"
#include "private/UploadArtifactTask.h"
#include "private/util.h"
#include "boost/format.hpp"
#include "public-util.h"
#include "artifact-uploader.h"
#include "client.h"

namespace prism
{
namespace connect
{

namespace prc = prism::connect;

static inline Payload makePayload(const PayloadHolder& holder)
{
    return holder.isFile()
            ? Payload(holder.getFilePath())
            : Payload(holder.getData(), holder.getDataSize(), holder.getMimeType());
}

Status UploadBackgroundTask::execute(ClientSession& session) const
{
    return session.client.uploadBackground(
                session.accountId, session.cameraId, background_, makePayload(*image_));
}

size_t UploadBackgroundTask::getArtifactSize() const
{
    return sizeof(background_) + sizeof(image_) + image_->getDataSize();
}

std::string UploadBackgroundTask::toString() const
{
    return prc::toString(background_);
}

Status UploadObjectSnapshotTask::execute(ClientSession& session) const
{
    return session.client.uploadObjectSnapshot(
                session.accountId, session.cameraId, snapshot_, makePayload(*image_));
}

size_t UploadObjectSnapshotTask::getArtifactSize() const
{
    return sizeof(snapshot_) + sizeof(image_) + image_->getDataSize();
}

std::string UploadObjectSnapshotTask::toString() const
{
    return prc::toString(snapshot_);
}

Status UploadFlipbookTask::execute(ClientSession& session) const
{
    return session.client.uploadFlipbook(
                session.accountId, session.cameraId, flipbook_, makePayload(*data_));
}

size_t UploadFlipbookTask::getArtifactSize() const
{
    return sizeof(flipbook_) + sizeof(data_) + data_->getDataSize();
}

std::string UploadFlipbookTask::toString() const
{
    return prc::toString(flipbook_);
}

Status UploadTimeSeriesTask::execute(ClientSession& session) const
{
    return session.client.uploadTimeSeries(session.accountId, session.cameraId, series_);
}

size_t UploadTimeSeriesTask::getArtifactSize() const
{
    return sizeof(series_);
}

std::string UploadTimeSeriesTask::toString() const
{
    return prc::toString(series_);
}

} // namespace connect
} // namespace prism
