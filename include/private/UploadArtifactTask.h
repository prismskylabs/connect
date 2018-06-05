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

struct ClientSession;

class UploadArtifactTask
{
public:
    virtual ~UploadArtifactTask()
    {}

    virtual Status execute(ClientSession& session) const = 0;
    virtual size_t getArtifactSize() const = 0;
    virtual std::string toString() const = 0;
};

typedef boost::shared_ptr<UploadArtifactTask> UploadArtifactTaskPtr;


class UploadBackgroundTask : public UploadArtifactTask
{
public:
    UploadBackgroundTask(const Background& background, PayloadHolderPtr image)
        : background_(background)
        , image_(image)
    {
    }

    Status execute(ClientSession& session) const;
    size_t getArtifactSize() const;
    std::string toString() const;

private:
    Background background_;
    PayloadHolderPtr image_;
};

typedef boost::shared_ptr<UploadBackgroundTask> UploadBackgroundTaskPtr;


class UploadObjectSnapshotTask : public UploadArtifactTask
{
public:
    UploadObjectSnapshotTask(const ObjectSnapshot& snapshot, PayloadHolderPtr image)
        : snapshot_(snapshot)
        , image_(image)
    {
    }

    Status execute(ClientSession& session) const;
    size_t getArtifactSize() const;
    std::string toString() const;

private:
    ObjectSnapshot snapshot_;
    PayloadHolderPtr image_;
};

typedef boost::shared_ptr<UploadObjectSnapshotTask> UploadObjectSnapshotTaskPtr;


class UploadFlipbookTask : public UploadArtifactTask
{
public:
    UploadFlipbookTask(const Flipbook& flipbook, PayloadHolderPtr data)
        : flipbook_(flipbook)
        , data_(data)
    {
    }

    Status execute(ClientSession& session) const;
    size_t getArtifactSize() const;
    std::string toString() const;

private:
    Flipbook flipbook_;
    PayloadHolderPtr data_;
};

typedef boost::shared_ptr<UploadFlipbookTask> UploadFlipbookTaskPtr;


class UploadTimeSeriesTask : public UploadArtifactTask
{
public:
    UploadTimeSeriesTask(const TimeSeries& series)
        : series_(series)
    {
    }

    Status execute(ClientSession& session) const;
    size_t getArtifactSize() const;
    std::string toString() const;

private:
    TimeSeries series_;
};

typedef boost::shared_ptr<UploadTimeSeriesTask> UploadCountTaskPtr;

} // namespace connect
} // namespace prism

#endif // PRISM_UPLOAD_ARTIFACT_TASK_H_
