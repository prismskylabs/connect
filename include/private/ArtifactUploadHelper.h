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

    bool uploadBackground(const prism::connect::timestamp_t& timestamp, const prism::connect::Payload& payload);
    bool uploadObjectStream(const prism::connect::ObjectStream& stream, const prism::connect::Payload& payload);
    bool uploadFlipbook(const prism::connect::Flipbook& flipbook, const prism::connect::Payload& payload);
    bool uploadEvent(const prism::connect::timestamp_t& timestamp, const prism::connect::Events& data);

private:
    PrismConnectServicePtr connectService_;
};
typedef boost::shared_ptr<ArtifactUploadHelper> ArtifactUploaderPtr;

} // namespace connect
} // namespace prism

#endif // CONNECT_SDK_UPLOAD_HELPER_H
