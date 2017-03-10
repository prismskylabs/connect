/*
 * Copyright (C) 2017 Prism Skylabs
 */
#include "private/UploadQueue.h"
#include "boost/thread/locks.hpp"
#include "boost/format.hpp"
#include "easylogging++.h"

namespace
{
struct NotEmpty
{
    NotEmpty(const std::deque<prism::connect::UploadArtifactTaskPtr>& d)
        : deque(d)
    {}

    bool operator()() const { return !deque.empty(); }

    const std::deque<prism::connect::UploadArtifactTaskPtr>& deque;
};
}

namespace prism
{
namespace connect
{

// Used internally by push_back. Caller must lock mutex_ before calling.
bool UploadQueue::arrangeFreeSpaceForTask(const size_t taskSize)
{
    if(taskSize > maxMemorySize_)
        return false;

    while(taskSize + size_ > maxMemorySize_)
    {
        const UploadArtifactTaskPtr t = deque_.front();
        deque_.pop_front();
        size_ -= t->getArtifactSize();
        LOG(WARNING) << "Upload queue is full. Preemptively removed " << t->toString();
    }
    return true;
}

void UploadQueue::push_back(UploadArtifactTaskPtr task)
{
    const size_t artifactSize = task ? task->getArtifactSize() : 0;
    bool canPush = false;
    {
        boost::lock_guard<boost::mutex> lock(mutex_);
        canPush = arrangeFreeSpaceForTask(artifactSize);
        if(canPush)
        {
            deque_.push_back(task);
            size_ += artifactSize;
        }
    }
    cv_.notify_one();
    if(!canPush)
    {
        const std::string message = (boost::format("Artifact %s is too large (%d) to put into upload queue. "
                "Queue max size is: %d bytes") % task->toString() % artifactSize % maxMemorySize_).str();
//        ThrowPrismException(message);
    }
}

void UploadQueue::push_front(UploadArtifactTaskPtr task)
{
    const size_t artifactSize = task ? task->getArtifactSize() : 0;
    bool queueIsFull = false;
    {
        boost::lock_guard<boost::mutex> lock(mutex_);
        queueIsFull = (artifactSize + size_ > maxMemorySize_);
        if(!queueIsFull)
        {
            deque_.push_front(task);
            size_ += artifactSize;
        }
    }
    cv_.notify_one();
    if(queueIsFull)
        LOG(WARNING) << "Unable to push artifact into the upload queue: queue is full";
}

bool UploadQueue::pop_front(UploadArtifactTaskPtr& task, const boost::posix_time::time_duration waitTime)
{
    boost::unique_lock<boost::mutex> lock(mutex_);
    const NotEmpty nePred(deque_);

    if(cv_.timed_wait(lock, waitTime, nePred))
    {
        task = deque_.front();
        size_ -= (task ? task->getArtifactSize() : 0);
        deque_.pop_front();
        return true;
    }
    return false;
}

} // namespace camera
} // namespace prism
