/*
 * Copyright (C) 2017 Prism Skylabs
 */
#include "private/ArtifactUploadHelper.h"
#include "public-util.h"

namespace prism
{
namespace connect
{

Status ArtifactUploadHelper::uploadBackground(const timestamp_t& timestamp,
        const Payload& payload)
{
    return connectService_->client->uploadBackground(connectService_->accountId, connectService_->instrumentId, timestamp, payload);
}

Status ArtifactUploadHelper::uploadObjectStream(const ObjectStream& stream,
        const Payload& payload)
{
    return connectService_->client->uploadObjectStream(connectService_->accountId, connectService_->instrumentId, stream, payload);
}

Status ArtifactUploadHelper::uploadFlipbook(const Flipbook& flipbook,
        const Payload& payload)
{
    const Status status = connectService_->client->uploadFlipbook(
            connectService_->accountId, connectService_->instrumentId, flipbook, payload);

    if (!isNetworkError(status))
    {
        removeFile(payload.fileName);
    }

    return status;
}

Status ArtifactUploadHelper::uploadEvent(const timestamp_t& timestamp,
        const Events& data)
{
    return connectService_->client->uploadEvent(connectService_->accountId, connectService_->instrumentId, timestamp, data);
}

} // namespace connect
} // namespace prism
