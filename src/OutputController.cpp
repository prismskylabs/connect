/*
 * Copyright (C) 2017 Prism Skylabs
 */
#include "private/OutputController.h"
#include <boost/thread/lock_guard.hpp>
#include <easylogging++.h>

namespace
{
    static const boost::posix_time::time_duration POP_TASK_TIMEOUT_SEC = boost::posix_time::seconds(3);
}

namespace prism
{
namespace connect
{

void OutputController::start()
{
    if(!cfg_.queue)
    {
        LOG(ERROR) << "OutputController: Unable to upload artifacts. UploadDeque wasn't set";
        return;
    }
    if(!cfg_.uploader)
    {
        LOG(ERROR) << "OutputController: Unable to upload artifacts. Uploader wasn't set";
        return;
    }

    const State state = getState();
    if(state != OCS_IDLE && state != OCS_TERMINATED)
    {
        LOG(WARNING) << "OutputController already started";
        return;
    }

    boost::thread t(&OutputController::doWork, this);
    thread_.swap(t);
    setState(OCS_RUNNING);
}

void OutputController::terminate()
{
    if(getState() == OCS_TERMINATED)
        return;

    LOG(INFO) << "Terminating artifact upload...";
    setState(OCS_TERMINATED);
    joinWorkingThread();
    LOG(INFO) << "Upload has been terminated";
}

void OutputController::stop()
{
    const State state = getState();
    if(state == OCS_STOPPING || state == OCS_TERMINATED)
        return;

    LOG(INFO) << "Waiting for upload to complete...";
    setState(OCS_STOPPING);
    joinWorkingThread();
    LOG(INFO) << "Upload has been completed";
}

void OutputController::doWork()
{
    try
    {
        while(getState() != OCS_TERMINATED)
        {
            UploadArtifactTaskPtr task;
            if(cfg_.queue->pop_front(task, POP_TASK_TIMEOUT_SEC))
            {
                if(!task)
                {
                    LOG(DEBUG) << "Artifact upload complete";
                    break;
                }
                else if(!task->execute(cfg_.uploader.get()))
                    cfg_.queue->push_front(task);
            }
            else if(getState() == OCS_STOPPING) // stop() has been called and nothing left for upload, terminating
                break;
        }
    }
    catch(const std::exception& e)
    {
        LOG(ERROR) << e.what();
    }
    catch(...)
    {
        LOG(ERROR) << "Unknown exception during artifact upload";
    }
    setState(OCS_TERMINATED);
}

OutputController::State OutputController::getState()
{
    boost::lock_guard<boost::mutex> lock(mutex_);
    return state_;
}

void OutputController::setState(State state)
{
    boost::lock_guard<boost::mutex> lock(mutex_);
    state_ = state;
}

OutputController::~OutputController()
{
    stop();
}

void OutputController::joinWorkingThread()
{
    if(cfg_.timeoutToCompleteUpload)
    {
        if(!thread_.try_join_for(boost::chrono::seconds(cfg_.timeoutToCompleteUpload)))
        {
            setState(OCS_TERMINATED);
            LOG(ERROR) << "OutputController thread did not finish for " << cfg_.timeoutToCompleteUpload
                    << " seconds. Need to increase timeout (output_controller.timeout_to_complete_upload_sec)?"
                    "May be there is some other bug? Deadlock?";

            thread_.detach(); // Expect dragons if you reached this point.
            // We shall increase timeout or look for bug if we got here.
        }
    }
    else
        thread_.join();
}

} // namespace camera
} // namespace prism
