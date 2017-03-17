/*
 * Copyright (C) 2017 Prism Skylabs
 */
#include <opencv2/videoio.hpp>
#include "easylogging++.h"
#include "artifact-uploader.h"
#include "client.h"
#include "testUtils.h"
#include "boost/filesystem.hpp"
#include "public-util.h"

namespace prc = prism::connect;

static const cv::Size BACKGROUND_SIZE(1280, 720);
static const cv::Size FLIPBOOK_SIZE(480, 360);
static const cv::Scalar BACKGROUND_COLOR(0, 0, 128);
static const cv::Scalar TEXT_COLOR(255, 255, 255);
static const std::string BACKGROUND_TEXT = "background";
static const std::string FLIPBOOK_TEXT = "flipbook";
static const std::string JPEG_MIME = "image/jpg";
static const std::string BACKGROUND_FILE = "background.jpg";
static const std::string FLIPBOOK_FILE = "flipbook.mp4";
static const cv::Rect ROI_RECT(0, 0, 240, 180);
static const std::string ROI_TEXT = "ROI";
static const std::string STREAM_TYPE = "foreground";

namespace prism
{
namespace test
{

void configCallback(prc::Client& client)
{
    client.setLogFlags(prc::Client::LOG_INPUT | prc::Client::LOG_INPUT_JSON);
    client.setConnectionTimeoutMs(5000);
    client.setLowSpeed(5);
}

static void testBackgroundUploading(prc::ArtifactUploader& uploader);
static void testFlipbookUploading(prc::ArtifactUploader& uploader);
static void testEventsUploading(prc::ArtifactUploader& uploader);
static void testObjectStreamUploading(prc::ArtifactUploader& uploader);

void testArtifactUploader
(
    const std::string& apiRoot,
    const std::string& apiToken,
    const std::string& cameraName
)
{
    const int ONE_MB = 1000000;
    prc::ArtifactUploader::Configuration
            uploaderConfig(apiRoot, apiToken, cameraName, 32 * ONE_MB, 24 * ONE_MB);
    prc::ArtifactUploader uploader;
    prc::Status status = uploader.init(uploaderConfig, configCallback);

    if (status.isError())
    {
        LOG(ERROR) << "Failed to init artifact uploader: " << status;
        return;
    }

    testBackgroundUploading(uploader);
    testFlipbookUploading(uploader);
    testEventsUploading(uploader);
    testObjectStreamUploading(uploader);
}

static void testBackgroundUploading(prc::ArtifactUploader& uploader)
{
    prc::removeFile(BACKGROUND_FILE);
    prc::removeFile(FLIPBOOK_FILE);

    cv::Mat background
            = generateBackgroundImage(BACKGROUND_SIZE, BACKGROUND_COLOR,
                                      BACKGROUND_TEXT, TEXT_COLOR);

    buf_t buffer;

    if (saveAsJpeg(background, buffer) != 0)
    {
        LOG(ERROR) << "Failed to encode background to memory buffer";
        return;
    }

    // use variable and explicit increments to have different timestamps
    // for different calls
    prc::timestamp_t ts = generateTimestamp();

    uploader.uploadBackground(ts, prc::makePayloadHolderByCopyingData(
                                  buffer.data(), buffer.size(), JPEG_MIME));

    // moving case should be the last one, as buffer will be lost in the process
    uploader.uploadBackground(ts + 10,
                              prc::makePayloadHolderByMovingData(prc::move(buffer), JPEG_MIME));

    if (saveAsJpeg(background, BACKGROUND_FILE) != 0)
    {
        LOG(ERROR) << "Failed to encode background to file";
        return;
    }

    uploader.uploadBackground(ts + 20,
                              prc::makePayloadHolderByReferencingFileAutodelete(BACKGROUND_FILE));
}

static void testFlipbookUploading(prc::ArtifactUploader& uploader)
{
    if (generateFlipbookFile(FLIPBOOK_SIZE, BACKGROUND_COLOR, FLIPBOOK_TEXT, TEXT_COLOR,
                              FLIPBOOK_FILE) != 0)
    {
        LOG(ERROR) << "Failed to create flipbook file";
        return;
    }

    prc::Flipbook fb;
    fb.height = FLIPBOOK_SIZE.height;
    fb.width = FLIPBOOK_SIZE.width;
    fb.numberOfFrames = 60;
    ts_pair_t timestamps = generateFlipbookTimestamps();
    fb.startTimestamp = timestamps.first;
    fb.stopTimestamp = timestamps.second;

    uploader.uploadFlipbook(fb, prc::makePayloadHolderByReferencingFileAutodelete(FLIPBOOK_FILE));
}

static void testEventsUploading(prc::ArtifactUploader& uploader)
{
    prc::Events events;
    events.push_back(prc::Event(generateTimestamp()));

    uploader.uploadEvent(generateTimestamp(), prc::move(events));
}

static void testObjectStreamUploading(prc::ArtifactUploader& uploader)
{
    cv::Mat roi = generateBackgroundImage(ROI_RECT.size(), BACKGROUND_COLOR, ROI_TEXT, TEXT_COLOR);
    buf_t buffer;

    if (saveAsJpeg(roi, buffer) != 0)
    {
        LOG(ERROR) << "Failed to save ROI image to memory";
        return;
    }

    prc::ObjectStream os;
    os.collected = generateTimestamp();
    os.height = ROI_RECT.height;
    os.width = ROI_RECT.width;
    os.locationX = ROI_RECT.x;
    os.locationY = ROI_RECT.y;
    os.origImageHeight = BACKGROUND_SIZE.height;
    os.origImageWidth = BACKGROUND_SIZE.width;
    os.objectId = 42;
    os.streamType = STREAM_TYPE;

    uploader.uploadObjectStream(os, prc::makePayloadHolderByMovingData(prc::move(buffer), JPEG_MIME));
}

} // namespace test
} // namespace prism
