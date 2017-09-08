/*
 * Copyright (C) 2017 Prism Skylabs
 */
#include "private/UploadQueue.h"
#include "boost/thread/locks.hpp"
#include "boost/format.hpp"
#include "easylogging++.h"
#include "private/util.h"

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
        addSize(-(int)t->getArtifactSize());
        LOG(WARNING) << "Upload queue is full. Preemptively removed " << t->toString();
    }
    return true;
}

Status UploadQueue::push_back(UploadArtifactTaskPtr task)
{
    const size_t artifactSize = task ? task->getArtifactSize() : 0;
    bool canPush = false;

    {
        boost::lock_guard<boost::mutex> lock(mutex_);
        canPush = arrangeFreeSpaceForTask(artifactSize);

        if (canPush)
        {
            deque_.push_back(task);
            addSize(artifactSize);
        }
    }

    cv_.notify_one();

    if (!canPush)
    {
        const std::string message = (boost::format("Artifact %s is too large (%d) to put into upload queue. "
                "Queue max size is: %d bytes") % task->toString() % artifactSize % maxMemorySize_).str();
        LOG(ERROR) << message;
        return makeError();
    }

    return makeSuccess();
}

Status UploadQueue::push_front(UploadArtifactTaskPtr task)
{
    const size_t artifactSize = task ? task->getArtifactSize() : 0;
    bool queueIsFull = false;

    {
        boost::lock_guard<boost::mutex> lock(mutex_);
        queueIsFull = (artifactSize + size_ > maxMemorySize_);

        if (!queueIsFull)
        {
            deque_.push_front(task);
            addSize(artifactSize);
        }
    }

    cv_.notify_one();

    if (queueIsFull)
    {
        LOG(WARNING) << "Unable to push artifact into the upload queue: queue is full";
        return makeError();
    }

    return makeSuccess();
}

bool UploadQueue::pop_front(UploadArtifactTaskPtr& task, const boost::posix_time::time_duration waitTime)
{
    boost::unique_lock<boost::mutex> lock(mutex_);
    const NotEmpty nePred(deque_);

    if(cv_.timed_wait(lock, waitTime, nePred))
    {
        task = deque_.front();
        addSize(task ? -(int)task->getArtifactSize() : 0);
        deque_.pop_front();
        return true;
    }
    return false;
}

bool UploadQueue::timed_wait(boost::system_time waitUntil)
{
    boost::unique_lock<boost::mutex> lock(mutex_);
    return cv_.timed_wait(lock, waitUntil);
}

// Caller must lock mutex_ before calling.
void UploadQueue::addSize(size_t size)
{
    size_ += size;

    if(size >= usageSizeWarning_)
        LOG_EVERY_N(5, WARNING) << boost::format("Upload queue is using %.2f MB out of %.2f MB")
                                   % ((float)size_ / 10e6) % ((float)maxMemorySize_/10e6);
}

} // namespace connect
} // namespace prism

