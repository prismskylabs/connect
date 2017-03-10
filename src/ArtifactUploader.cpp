/*
 * Copyright (C) 2017 Prism Skylabs
 */
#include "private/ArtifactUploader.h"

#include <boost/filesystem.hpp>
#include <easylogging++.h>

namespace prc = prism::connect;

namespace prism
{
namespace connect
{

bool ArtifactUploader::uploadBackground(const prism::connect::timestamp_t& timestamp,
        const prism::connect::Payload& payload)
{
    const prc::Status status = connectService_->client->uploadBackground(
            connectService_->accountId, connectService_->instrumentId, timestamp, payload);
    if(status.isError())
        LOG(ERROR) << "Unable to upload background (" << timestamp << ", " << payload.fileName << ")";

    return status.isSuccess() || (status.isError() && status.getFacility() != prc::Status::FACILITY_NETWORK);
}

bool ArtifactUploader::uploadObjectStream(const prism::connect::ObjectStream& stream,
        const prism::connect::Payload& payload)
{
    const prc::Status status = connectService_->client->uploadObjectStream(
            connectService_->accountId, connectService_->instrumentId, stream, payload);
    if(status.isError())
        LOG(ERROR) << "Unable to upload object stream (" << stream.collected << ")";

    return status.isSuccess() || (status.isError() && status.getFacility() != prc::Status::FACILITY_NETWORK);
}

bool ArtifactUploader::uploadFlipbook(const prism::connect::Flipbook& flipbook,
        const prism::connect::Payload& payload)
{
    const prc::Status status = connectService_->client->uploadFlipbook(
            connectService_->accountId, connectService_->instrumentId, flipbook, payload);
    if(status.isSuccess())
    {
        try
        {
            boost::filesystem::remove(payload.fileName);
        }
        catch (const std::exception& e)
        {
            LOG(ERROR)<< "Error removing flipbook file " << payload.fileName << ": " << e.what();
        }
        catch(...)
        {   LOG(ERROR) << "Error removing flipbook file " << payload.fileName;}
    }
    else
        LOG(ERROR) << "Unable to upload flipbook (" << flipbook.startTimestamp << ", " << flipbook.stopTimestamp << ", " << payload.fileName << ")";

    return status.isSuccess() || (status.isError() && status.getFacility() != prc::Status::FACILITY_NETWORK);
}

bool ArtifactUploader::uploadEvent(const prism::connect::timestamp_t& timestamp,
        const prism::connect::Events& data)
{
    const prc::Status status = connectService_->client->uploadEvent(
            connectService_->accountId, connectService_->instrumentId, timestamp, data);
    if(status.isError())
        LOG(ERROR) << "Unable to upload event (" << timestamp << ")";

    return status.isSuccess() || (status.isError() && status.getFacility() != prc::Status::FACILITY_NETWORK);
}

} // namespace connect
} // namespace prism
