/*
 * Copyright (C) 2017 Prism Skylabs
 */
#ifndef PRISM_UPLOAD_ARTIFACT_TASK_H_
#define PRISM_UPLOAD_ARTIFACT_TASK_H_

#include "domain-types.h"
#include "boost/shared_ptr.hpp"
#include "public-util.h"
#include "payload-holder.h"

namespace prism
{
namespace connect
{

class Client;

class UploadArtifactTask
{
public:
    virtual ~UploadArtifactTask()
    {}

    virtual Status execute(Client& client, id_t accountId, id_t cameraId) const = 0;
    virtual size_t getArtifactSize() const = 0;
    virtual std::string toString() const = 0;
};

typedef boost::shared_ptr<UploadArtifactTask> UploadArtifactTaskPtr;


class UploadBackgroundTask : public UploadArtifactTask
{
public:
    UploadBackgroundTask(const timestamp_t& timestamp, PayloadHolderPtr image)
        : timestamp_(timestamp)
        , image_(image)
    {
    }

    Status execute(Client& client, id_t accountId, id_t cameraId) const;
    size_t getArtifactSize() const;
    std::string toString() const;

private:
    prism::connect::timestamp_t timestamp_;
    PayloadHolderPtr image_;
};

typedef boost::shared_ptr<UploadBackgroundTask> UploadBackgroundTaskPtr;


class UploadObjectStreamTask : public UploadArtifactTask
{
public:
    UploadObjectStreamTask(const ObjectStream& stream, PayloadHolderPtr image)
        : stream_(stream)
        , image_(image)
    {
    }

    Status execute(Client& client, id_t accountId, id_t cameraId) const;
    size_t getArtifactSize() const;
    std::string toString() const;

private:
    ObjectStream stream_;
    PayloadHolderPtr image_;
};

typedef boost::shared_ptr<UploadObjectStreamTask> UploadObjectStreamTaskPtr;


class UploadFlipbookTask : public UploadArtifactTask
{
public:
    UploadFlipbookTask(const Flipbook& flipbook, PayloadHolderPtr data)
        : flipbook_(flipbook)
        , data_(data)
    {
    }

    Status execute(Client& client, id_t accountId, id_t cameraId) const;
    size_t getArtifactSize() const;
    std::string toString() const;

private:
    Flipbook flipbook_;
    PayloadHolderPtr data_;
};

typedef boost::shared_ptr<UploadFlipbookTask> UploadFlipbookTaskPtr;


class UploadEventTask : public UploadArtifactTask
{
public:
    UploadEventTask(const timestamp_t& timestamp, move_ref<Events> events)
        : timestamp_(timestamp)
    {
        std::swap(events.ref, data_);
    }

    Status execute(Client& client, id_t accountId, id_t cameraId) const;
    size_t getArtifactSize() const;
    std::string toString() const;

private:
    timestamp_t timestamp_;
    Events data_;
};

typedef boost::shared_ptr<UploadEventTask> UploadEventTaskPtr;


class UploadCountTask : public UploadArtifactTask
{
public:
    UploadCountTask(move_ref<Counts> counts)
    {
        std::swap(counts.ref, data_);
    }

    Status execute(Client& client, id_t accountId, id_t cameraId) const;
    size_t getArtifactSize() const;
    std::string toString() const;

private:
    Counts data_;
};

typedef boost::shared_ptr<UploadCountTask> UploadCountTaskPtr;

} // namespace connect
} // namespace prism

#endif // PRISM_UPLOAD_ARTIFACT_TASK_H_
