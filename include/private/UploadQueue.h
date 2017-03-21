/*
 * Copyright (C) 2017 Prism Skylabs
 */
#ifndef PRISM_UPLOAD_QUEUE_H_
#define PRISM_UPLOAD_QUEUE_H_

#include <deque>

#include <boost/thread/condition_variable.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/locks.hpp>

#include "UploadArtifactTask.h"

namespace prism
{
namespace connect
{

class UploadQueue
{
public:
    UploadQueue(size_t maxMemorySize, size_t usageWarningSize)
        : maxMemorySize_(maxMemorySize)
        , usageSizeWarning_(usageWarningSize)
        , size_(0)
    {}

    Status push_back(UploadArtifactTaskPtr task);
    Status push_front(UploadArtifactTaskPtr task);
    bool pop_front(UploadArtifactTaskPtr& task, const boost::posix_time::time_duration waitTime = boost::posix_time::pos_infin);

private:
    const size_t maxMemorySize_;
    const size_t usageSizeWarning_;
    size_t size_;
    std::deque<UploadArtifactTaskPtr> deque_;

    boost::condition_variable cv_;
    boost::mutex mutex_;

    bool arrangeFreeSpaceForTask(const size_t taskSize);
    void addSize(int size);
};
typedef boost::shared_ptr<UploadQueue> UploadQueuePtr;

} // namespace connect
} // namespace prism

#endif // PRISM_UPLOAD_QUEUE_H_
