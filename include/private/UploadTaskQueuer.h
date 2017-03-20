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

    virtual void addBackgroundTask(UploadBackgroundTaskPtr task) = 0;
    virtual void addObjectStreamTask(UploadObjectStreamTaskPtr task) = 0;
    virtual void addFlipbookTask(UploadFlipbookTaskPtr task) = 0;
    virtual void addEventTask(UploadEventTaskPtr task) = 0;

    virtual void finalizeUpload() = 0;
};

typedef boost::shared_ptr<UploadTaskQueuer> UploadTaskQueuerPtr;

class SimpleUploadTaskQueuer : public UploadTaskQueuer
{
public:
    SimpleUploadTaskQueuer(UploadQueuePtr uploadDeque)
        : uploadQueue_(uploadDeque)
    {}

    void addBackgroundTask(UploadBackgroundTaskPtr task);
    void addObjectStreamTask(UploadObjectStreamTaskPtr task);
    void addFlipbookTask(UploadFlipbookTaskPtr task);
    void addEventTask(UploadEventTaskPtr task);

    void finalizeUpload();
private:
    UploadQueuePtr uploadQueue_;
};

} // namespace connect
} // namespace prism

#endif // PRISM_UPLOAD_TASK_QUEUER_H_
