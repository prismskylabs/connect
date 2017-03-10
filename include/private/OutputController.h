/*
 * Copyright (C) 2017 Prism Skylabs
 */
#ifndef PRISM_OUTPUTCONTROLLER_H_
#define PRISM_OUTPUTCONTROLLER_H_

#include "UploadQueue.h"
#include "ArtifactUploader.h"

#include <boost/thread/thread.hpp>

namespace prism
{
namespace connect
{

class OutputController
{
public:
    enum State
    {
        OCS_IDLE,
        OCS_RUNNING,
        OCS_STOPPING,
        OCS_TERMINATED
    };

    struct Configuration
    {
        Configuration(UploadQueuePtr queue, ArtifactUploaderPtr uploader, int timeoutToCompleteUpload)
            : queue(queue)
            , uploader(uploader)
            , timeoutToCompleteUpload(timeoutToCompleteUpload)
        {
        }

        UploadQueuePtr queue;
        ArtifactUploaderPtr uploader;
        int timeoutToCompleteUpload;
    };

    OutputController(const Configuration& cfg)
        : cfg_(cfg)
        , state_(OCS_IDLE)
    {
    }

    ~OutputController();

    void start();
    void terminate();
    // OutputController will continue upload until it'll get NULL task. The NULL means an end of upload.
    void stop();
    State getState();

private:
    Configuration cfg_;
    boost::thread thread_;
    boost::mutex mutex_;
    State state_;

    void setState(State state);
    void doWork();
    void joinWorkingThread();
};

typedef boost::shared_ptr<OutputController> OutputControllerPtr;

} // namespace connect
} // namespace prism

#endif // PRISM_OUTPUTCONTROLLER_H_
