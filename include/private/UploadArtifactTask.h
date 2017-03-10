/*
 * Copyright (C) 2017 Prism Skylabs
 */
#ifndef PRISM_UPLOAD_ARTIFACT_TASK_H_
#define PRISM_UPLOAD_ARTIFACT_TASK_H_

#include "domain-types.h"
#include "boost/shared_ptr.hpp"

namespace prism
{
namespace connect
{

class ArtifactUploader;

typedef std::vector<unsigned char> UCharBuffer;
typedef boost::shared_ptr<UCharBuffer> UCharBufferPtr;

class UploadArtifactTask
{
public:
    virtual ~UploadArtifactTask()
    {}

    virtual bool execute(ArtifactUploader* uploader) const = 0;
    virtual size_t getArtifactSize() const = 0;
    virtual std::string toString() const = 0;
};
typedef boost::shared_ptr<UploadArtifactTask> UploadArtifactTaskPtr;

class UploadBackgroundTask : public UploadArtifactTask
{
public:
    UploadBackgroundTask(prism::connect::timestamp_t timestamp, UCharBufferPtr image)
        : timestamp_(timestamp)
        , image_(image)
    {}

    bool execute(ArtifactUploader* uploader) const;
    size_t getArtifactSize() const;
    std::string toString() const;

private:
    prism::connect::timestamp_t timestamp_;
    UCharBufferPtr image_;
};
typedef boost::shared_ptr<UploadBackgroundTask> UploadBackgroundTaskPtr;

class UploadObjectStreamTask : public UploadArtifactTask
{
public:
    UploadObjectStreamTask(const prism::connect::ObjectStream& stream, UCharBufferPtr image)
        : stream_(stream)
        , image_(image)
    {}

    bool execute(ArtifactUploader* uploader) const;
    size_t getArtifactSize() const;
    std::string toString() const;

private:
    prism::connect::ObjectStream stream_;
    UCharBufferPtr image_;
};
typedef boost::shared_ptr<UploadObjectStreamTask> UploadObjectStreamTaskPtr;

class UploadFlipbookTask : public UploadArtifactTask
{
public:
    UploadFlipbookTask(prism::connect::Flipbook flipbook, std::string imageFile)
        : flipbook_(flipbook)
        , payload_(imageFile)
    {}

    bool execute(ArtifactUploader* uploader) const;
    size_t getArtifactSize() const;
    std::string toString() const;

private:
    prism::connect::Flipbook flipbook_;
    prism::connect::Payload payload_;
};
typedef boost::shared_ptr<UploadFlipbookTask> UploadFlipbookTaskPtr;

class UploadEventTask : public UploadArtifactTask
{
public:
    UploadEventTask(prism::connect::timestamp_t timestamp)
        : timestamp_(timestamp)
    {}

    bool execute(ArtifactUploader* uploader) const;
    size_t getArtifactSize() const;
    std::string toString() const;

    void addEvent(const prism::connect::Event& event);
private:
    prism::connect::timestamp_t timestamp_;
    prism::connect::Events data_;
};
typedef boost::shared_ptr<UploadEventTask> UploadEventTaskPtr;

} // namespace connect
} // namespace prism

#endif // PRISM_UPLOAD_ARTIFACT_TASK_H_
