/*
 * Copyright (C) 2017 Prism Skylabs
 */
#include "private/ArtifactUploadHelper.h"
#include "boost/filesystem.hpp"
#include "easylogging++.h"
#include "public-util.h"

namespace prc = prism::connect;

namespace prism
{
namespace connect
{

Status ArtifactUploadHelper::uploadBackground(const prism::connect::timestamp_t& timestamp,
        const prism::connect::Payload& payload)
{
    return connectService_->client->uploadBackground(connectService_->accountId, connectService_->instrumentId, timestamp, payload);
}

Status ArtifactUploadHelper::uploadObjectStream(const prism::connect::ObjectStream& stream,
        const prism::connect::Payload& payload)
{
    return connectService_->client->uploadObjectStream(connectService_->accountId, connectService_->instrumentId, stream, payload);
}

Status ArtifactUploadHelper::uploadFlipbook(const prism::connect::Flipbook& flipbook,
        const prism::connect::Payload& payload)
{
    const prc::Status status = connectService_->client->uploadFlipbook(
            connectService_->accountId, connectService_->instrumentId, flipbook, payload);

    if (!isNetworkError(status))
    {
        prc::removeFile(payload.fileName);
    }

    return status;
}

Status ArtifactUploadHelper::uploadEvent(const prism::connect::timestamp_t& timestamp,
        const prism::connect::Events& data)
{
    return connectService_->client->uploadEvent(connectService_->accountId, connectService_->instrumentId, timestamp, data);
}

} // namespace connect
} // namespace prism
