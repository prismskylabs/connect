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

private:
    PrismConnectServicePtr connectService_;
};
typedef boost::shared_ptr<ArtifactUploadHelper> ArtifactUploaderPtr;

} // namespace connect
} // namespace prism

#endif // CONNECT_SDK_UPLOAD_HELPER_H
