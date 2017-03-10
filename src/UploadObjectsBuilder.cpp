/*
 * Copyright (C) 2017 Prism Skylabs
 */
#include "private/UploadObjectsBuilder.h"
#include "boost/make_shared.hpp"
#include "boost/shared_ptr.hpp"
#include "easylogging++.h"

namespace prism
{
namespace connect
{
UploadObjectsBuilder::UploadObjectsBuilder()
{
}

//UploadObjectsBuilder::UploadObjectsBuilder(const JsonConfigPtr cfg)
//    : cfg_(cfg)
//{
//    makeArtifactUploadQueue();
//}

UploadTaskQueuerPtr UploadObjectsBuilder::makeUploadTaskQueuer() const
{
//    if(!artifactUploadQueue_)
//        ThrowPrismException("Artifact upload queue wasn't created");

//    JsonConfigPtr uploadTaskQueuerCfg = cfg_->getConfig("upload_task_queuer");
//    const std::string type = uploadTaskQueuerCfg->getValueAsString("type");

//    if(!type.compare("simple"))
//        return boost::make_shared<SimpleUploadTaskQueuer>(artifactUploadQueue_);
//    else
//        ThrowPrismException("Unknown artifact upload task manager type: " + type);
}

void UploadObjectsBuilder::makeArtifactUploadQueue()
{
//    JsonConfigPtr uploadQueueCfg = cfg_->getConfig("artifact_upload_queue");

//    const int maxQuerySizeMB = uploadQueueCfg->getValueAsInt("max_memory_size_mb");
//    if(maxQuerySizeMB <= 0)
//    {
//        const std::string message = (boost::format("Invalid value artifact_upload_queue.max_memory_size_mb: %d") % maxQuerySizeMB).str();
//        ThrowPrismException(message.c_str());
//    }

//    artifactUploadQueue_ = boost::make_shared<UploadQueue>(maxQuerySizeMB * 1024 * 1024);
}

OutputControllerPtr UploadObjectsBuilder::makeOutputController() const
{
//    if(!artifactUploadQueue_)
//        ThrowPrismException("Artifact upload queue wasn't created");

//    JsonConfigPtr cfgEnvironment = cfg_->getConfig("environment");
//    JsonConfigPtr cfgInput = cfg_->getConfig("input");
//    PrismConnectService::Configuration connectConfig;

//    connectConfig.apiRoot = cfgEnvironment->getValueAsString("api_root");
//    connectConfig.apiToken = cfgEnvironment->getValueAsString("api_token");
//    connectConfig.cameraName = cfgInput->getValueAsString("camera_name");

//    PrismConnectServicePtr connect(new PrismConnectService());
//    int result = connect->init(connectConfig);
//    LOG(INFO) << "Prism Connect init() return code " << result;

//    JsonConfigPtr cfgOutputController = cfg_->getConfig("output_controller");

//    return boost::make_shared<OutputController>(OutputController::Configuration(
//            artifactUploadQueue_,
//            boost::make_shared<ArtifactUploader>(connect),
//            cfgOutputController->getValueAsInt("timeout_to_complete_upload_sec", 0)));
}

} // namespace camera
} // namespace prism
