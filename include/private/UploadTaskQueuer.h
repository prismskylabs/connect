/*
 * Copyright (C) 2017 Prism Skylabs
 */
#ifndef PRISM_UPLOAD_TASK_QUEUER_H_
#define PRISM_UPLOAD_TASK_QUEUER_H_

#include "UploadArtifactTask.h"
#include "UploadQueue.h"

namespace prism
{
namespace connect
{

class UploadTaskQueuer
{
public:
    virtual ~UploadTaskQueuer() {}

    virtual Status addBackgroundTask(UploadBackgroundTaskPtr task) = 0;
    virtual Status addObjectStreamTask(UploadObjectStreamTaskPtr task) = 0;
    virtual Status addFlipbookTask(UploadFlipbookTaskPtr task) = 0;
    virtual Status addEventTask(UploadEventTaskPtr task) = 0;
    virtual Status addCountTask(UploadCountTaskPtr task) = 0;

    virtual void finalizeUpload() = 0;
};

typedef boost::shared_ptr<UploadTaskQueuer> UploadTaskQueuerPtr;

class SimpleUploadTaskQueuer : public UploadTaskQueuer
{
public:
    SimpleUploadTaskQueuer(UploadQueuePtr uploadDeque)
        : uploadQueue_(uploadDeque)
    {}

    Status addBackgroundTask(UploadBackgroundTaskPtr task);
    Status addObjectStreamTask(UploadObjectStreamTaskPtr task);
    Status addFlipbookTask(UploadFlipbookTaskPtr task);
    Status addEventTask(UploadEventTaskPtr task);
    Status addCountTask(UploadCountTaskPtr task);

    void finalizeUpload();
private:
    UploadQueuePtr uploadQueue_;
};

} // namespace connect
} // namespace prism

#endif // PRISM_UPLOAD_TASK_QUEUER_H_
