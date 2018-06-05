/*
 * Copyright (C) 2017 Prism Skylabs
 */
#include "artifact-uploader.h"
#include "client.h"

#include "private/UploadQueue.h"
#include "private/util.h"
#include "private/const-strings.h"

#include "easylogging++.h"

#include "boost/filesystem.hpp"
#include "boost/make_shared.hpp"
#include "boost/thread/thread.hpp"

namespace
{
    static const boost::posix_time::time_duration NETWORK_ERROR_WAIT_PERIOD_SEC = boost::posix_time::seconds(3);
}

namespace prism
{
namespace connect
{

class ArtifactUploader::Impl
{
public:
    Impl()
        : done_(false)
        , timeoutToCompleteUploadSec_(0)
    {
    }

    ~Impl();

    Status init(const ArtifactUploader::Configuration& cfg,
                ArtifactUploader::ClientConfigCallback* configCallback);

    Status enqueueTask(UploadArtifactTaskPtr task)
    {
        return queue_->push_back(task);
    }

    void abort()
    {
        done_ = true;
    }

private:
    void threadFunc();

    ClientSession session_;
    UploadQueuePtr queue_;
    boost::thread thread_;

    // We don't care about race condition or atomicity as we need to signal
    // value changed from false to true.
    // volatile is to prevent optimizing while(!done) into while(true)
    volatile bool done_;

    int timeoutToCompleteUploadSec_;
};

ArtifactUploader::ArtifactUploader()
    : pImpl_(new Impl())
{
}

Status ArtifactUploader::init(const ArtifactUploader::Configuration& cfg,
                              ClientConfigCallback configCallback)
{
    return impl().init(cfg, configCallback);
}

ArtifactUploader::~ArtifactUploader()
{
}

Status ArtifactUploader::uploadBackground(const Background& background, PayloadHolderPtr payload)
{
    return impl().enqueueTask(boost::make_shared<UploadBackgroundTask>(background, payload));
}

Status ArtifactUploader::uploadFlipbook(const Flipbook& flipbook, PayloadHolderPtr payload)
{
    return impl().enqueueTask(boost::make_shared<UploadFlipbookTask>(flipbook, payload));
}

Status ArtifactUploader::uploadObjectSnapshot(const ObjectSnapshot& snapshot, PayloadHolderPtr payload)
{
    return impl().enqueueTask(boost::make_shared<UploadObjectSnapshotTask>(snapshot, payload));
}

Status ArtifactUploader::uploadTimeSeries(const TimeSeries& series)
{
    return impl().enqueueTask(boost::make_shared<UploadTimeSeriesTask>(series));
}

void ArtifactUploader::abort()
{
    impl().abort();
}

ArtifactUploader::Impl::~Impl()
{
    const char* FNAME = "ArtifactUploader::Impl::~Impl()";

    LOG(DEBUG) << "Entered " << FNAME
               << ", timeout to complete upload, sec: " << timeoutToCompleteUploadSec_;

    // This will interrupt wait on queue_'s conditional variable.
    queue_->push_back(UploadArtifactTaskPtr());

    if (timeoutToCompleteUploadSec_)
    {
        if (!thread_.try_join_for(boost::chrono::seconds(timeoutToCompleteUploadSec_)))
        {
            LOG(ERROR) << "Thread didn't finish for timeout period. Need to increase "
                          "timeout (output_controller.timeout_to_complete_upload_sec)?"
                          "May be there is some other bug? Deadlock?";

            abort();
            thread_.join();
        }
    }
    else
        thread_.join();

    if (!queue_->empty())
        LOG(WARNING) << "Tasks still in queue: " << queue_->size();

    LOG(DEBUG) << "Exiting " << FNAME;
}

Status ArtifactUploader::Impl::init(const ArtifactUploader::Configuration& cfg,
                                    ArtifactUploader::ClientConfigCallback* configCallback)
{
    if (cfg.maxQueueSize == 0)
    {
        LOG(ERROR) << "Invalid maxQueueSize value " << cfg.maxQueueSize;
        return makeError();
    }

    // cfg.warnQueueSize is always >= 0, as type is size_t
    if (cfg.queueType.compare(kStrSimple))
    {
        LOG(ERROR) << "Unsupported queue type: " << cfg.queueType;
        return makeError();
    }

    Client client(cfg.apiRoot, cfg.apiToken);

    if (configCallback)
        configCallback(client);

    Status status = client.init();

    if (status.isError())
    {
        LOG(ERROR) << "Client::init() failed: " << status;
        return status;
    }

    Accounts accounts;
    status = client.queryAccountsList(accounts);

    if (status.isError())
    {
        LOG(ERROR) << "Failed to get accounts list: " << status;
        return status;
    }

    if (accounts.empty())
    {
        LOG(ERROR) << "No accounts associated with given token";
        return makeError();
    }

    id_t accountId = accounts[0].id;

    LOG(INFO) << "Account ID: " << accountId;

    Feed camera;
    status = findCameraByName(client, accountId, cfg.cameraName, camera);

    if (status.isError())
    {
        if (status.getCode() != Status::NOT_FOUND)
            return status;

        status = registerNewCamera(client, accountId, cfg.cameraName, camera);

        if (status.isError())
            return status;
    }

    LOG(INFO) << "Camera (feed) ID: " << camera.id;

    session_.client.swap(client);
    session_.accountId = accountId;
    session_.cameraId = camera.id;

    queue_ = boost::make_shared<UploadQueue>(cfg.maxQueueSize, cfg.warnQueueSize);

    boost::thread t(&Impl::threadFunc, this);
    thread_.swap(t);

    timeoutToCompleteUploadSec_ = cfg.timeoutToCompleteUploadSec;

    return makeSuccess();
}

static bool shouldRetryUpload(Status status)
{
    return isNetworkError(status)
            || (status.getFacility() == Status::FACILITY_HTTP && status.getCode() == 503)
            || (status.getFacility() == Status::FACILITY_HTTP && status.getCode() == 500);
}

void ArtifactUploader::Impl::threadFunc()
{
    // defining const as __FUNCTIONS__ gives too little, __func__ gives too much
    const char* FNAME = "ArtifactUploader::Impl::threadFunc()";
    LOG(DEBUG) << "Entered " << FNAME;

    try
    {
        while (!done_)
        {
            UploadArtifactTaskPtr task;

            if (!queue_->pop_front(task))
                continue;

            if (!task) // upload complete
                break;

            const Status status = task->execute(session_);

            if (status.isSuccess())
            {
                LOG(INFO) << "Artifact " << task->toString() << " uploaded successfully";
                continue;
            }

            LOG(ERROR) << "Unable to upload artifact " << task->toString() << ". Error: " << status;

            if (shouldRetryUpload(status))
            {
                LOG(DEBUG) << "Returning artifact back to upload queue";
                queue_->push_front(task);

                boost::system_time waitUntil = boost::get_system_time() + NETWORK_ERROR_WAIT_PERIOD_SEC;

                // Don't try to upload the task right away, wait awhile.
                // while is to handle spurious wake-ups and wake-ups due to adding
                // new task to the queue.
                while (!done_ && boost::get_system_time() < waitUntil)
                    queue_->timed_wait(waitUntil);
            }
        } // while
    } // try
    catch (const std::exception& e)
    {
        LOG(ERROR) << FNAME << ": " << e.what();
    }
    catch (...)
    {
        LOG(ERROR) << FNAME << ": Unknown exception";
    }

    LOG(DEBUG) << "Exiting " << FNAME;
}

} // namespace connect
} // namespace prism
