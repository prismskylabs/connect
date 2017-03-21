/*
 * Copyright (C) 2017 Prism Skylabs
 */
#include "private/UploadTaskQueuer.h"

namespace prism
{
namespace connect
{

Status SimpleUploadTaskQueuer::addBackgroundTask(UploadBackgroundTaskPtr task)
{
    return uploadQueue_->push_back(task);
}

Status SimpleUploadTaskQueuer::addObjectStreamTask(UploadObjectStreamTaskPtr task)
{
    return uploadQueue_->push_back(task);
}

Status SimpleUploadTaskQueuer::addFlipbookTask(UploadFlipbookTaskPtr task)
{
    return uploadQueue_->push_back(task);
}

Status SimpleUploadTaskQueuer::addEventTask(UploadEventTaskPtr task)
{
    return uploadQueue_->push_back(task);
}

void SimpleUploadTaskQueuer::finalizeUpload()
{
    // adding NULL pointer to indicate the end of items submission
    uploadQueue_->push_back(UploadArtifactTaskPtr());
}

} // namespace connect
} // namespace prism
