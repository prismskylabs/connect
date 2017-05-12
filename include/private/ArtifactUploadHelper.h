/*
 * Copyright (C) 2017 Prism Skylabs
 */
#ifndef CONNECT_SDK_UPLOAD_HELPER_H
#define CONNECT_SDK_UPLOAD_HELPER_H

#include "domain-types.h"
#include "PrismConnectService.h"

namespace prism
{
namespace connect
{

// TODO: remove helper, so caller just calls methods of class Client
class ArtifactUploadHelper
{
public:
    ArtifactUploadHelper(PrismConnectServicePtr connectService)
        : connectService_(connectService)
    {}

    Status uploadBackground(const timestamp_t& timestamp, const Payload& payload);
    Status uploadObjectStream(const ObjectStream& stream, const Payload& payload);
    Status uploadFlipbook(const Flipbook& flipbook, const Payload& payload);
    Status uploadEvent(const timestamp_t& timestamp, const Events& data);
    Status uploadCount(const Counts& data);

private:
    PrismConnectServicePtr connectService_;
};
typedef boost::shared_ptr<ArtifactUploadHelper> ArtifactUploaderPtr;

} // namespace connect
} // namespace prism

#endif // CONNECT_SDK_UPLOAD_HELPER_H
