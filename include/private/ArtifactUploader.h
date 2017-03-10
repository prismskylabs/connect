/*
 * Copyright (C) 2017 Prism Skylabs
 */
#ifndef PRISM_ARTIFACTUPLOADER_H_
#define PRISM_ARTIFACTUPLOADER_H_

#include "domain-types.h"
#include "PrismConnectService.h"

namespace prism
{
namespace connect
{

class ArtifactUploader
{
public:
    ArtifactUploader(PrismConnectServicePtr connectService)
        : connectService_(connectService)
    {}

    bool uploadBackground(const prism::connect::timestamp_t& timestamp, const prism::connect::Payload& payload);
    bool uploadObjectStream(const prism::connect::ObjectStream& stream, const prism::connect::Payload& payload);
    bool uploadFlipbook(const prism::connect::Flipbook& flipbook, const prism::connect::Payload& payload);
    bool uploadEvent(const prism::connect::timestamp_t& timestamp, const prism::connect::Events& data);

private:
    PrismConnectServicePtr connectService_;
};
typedef boost::shared_ptr<ArtifactUploader> ArtifactUploaderPtr;

} // namespace connect
} // namespace prism

#endif // PRISM_ARTIFACTUPLOADER_H_
