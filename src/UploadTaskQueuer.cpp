/*
 * Copyright (C) 2017 Prism Skylabs
 */
#include "private/UploadTaskQueuer.h"

namespace prism
{
namespace connect
{

void SimpleUploadTaskQueuer::addBackgroundTask(UploadBackgroundTaskPtr task)
{
    uploadQueue_->push_back(task);
}

void SimpleUploadTaskQueuer::addObjectStreamTask(UploadObjectStreamTaskPtr task)
{
    uploadQueue_->push_back(task);
}

void SimpleUploadTaskQueuer::addFlipbookTask(UploadFlipbookTaskPtr task)
{
    uploadQueue_->push_back(task);
}

void SimpleUploadTaskQueuer::addEventTask(UploadEventTaskPtr task)
{
    uploadQueue_->push_back(task);
}

void SimpleUploadTaskQueuer::finalizeUpload()
{
    // adding NULL pointer to indicate the end of items submission
    uploadQueue_->push_back(UploadArtifactTaskPtr());
}

} // namespace connect
} // namespace prism
