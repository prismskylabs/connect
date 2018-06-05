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
static const std::string JPEG_MIME = "image/jpeg";
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
static void testObjectSnapshotUploading(prc::ArtifactUploader& uploader);

void testArtifactUploader
(
    const std::string& apiRoot,
    const std::string& apiToken,
    const std::string& cameraName
)
{
    const int ONE_MB = 1000000;
    prc::ArtifactUploader::Configuration
            uploaderConfig(apiRoot, apiToken, cameraName, 32 * ONE_MB, 24 * ONE_MB,
                           "simple", 5);
    prc::ArtifactUploader uploader;
    prc::Status status = uploader.init(uploaderConfig, configCallback);

    if (status.isError())
    {
        LOG(ERROR) << "Failed to init artifact uploader: " << status;
        return;
    }

    testBackgroundUploading(uploader);
    testFlipbookUploading(uploader);
    testObjectSnapshotUploading(uploader);
}

static void testBackgroundUploading(prc::ArtifactUploader& uploader)
{
    prc::removeFile(BACKGROUND_FILE);
    prc::removeFile(FLIPBOOK_FILE);

    cv::Mat backgroundImage
            = generateBackgroundImage(BACKGROUND_SIZE, BACKGROUND_COLOR,
                                      BACKGROUND_TEXT, TEXT_COLOR);

    buf_t buffer;

    if (saveAsJpeg(backgroundImage, buffer) != 0)
    {
        LOG(ERROR) << "Failed to encode background to memory buffer";
        return;
    }

    // use variable and explicit increments to have different timestamps
    // for different calls
    prc::timestamp_t ts = generateTimestamp();

    std::string uuid4[] = {
        "C05D60C636FE4572BB6526075D2458DF",
        "BEC0D83BDA7A4FBB80F6BC82AC48E84F",
        "CCFEA1DAC5F0456683F32BFC8749371E"
    };

    prc::timestamp_t timestamp[] = {
        ts,
        ts + 10,
        ts + 20
    };

    prc::Background background[] = {
        prc::Background(uuid4[0], timestamp[0], timestamp[0], BACKGROUND_SIZE.width, BACKGROUND_SIZE.height),
        prc::Background(uuid4[1], timestamp[1], timestamp[1], BACKGROUND_SIZE.width, BACKGROUND_SIZE.height),
        prc::Background(uuid4[2], timestamp[2], timestamp[2], BACKGROUND_SIZE.width, BACKGROUND_SIZE.height)
    };

    uploader.uploadBackground(background[0], prc::makePayloadHolderByCopyingData(buffer.data(), buffer.size(), JPEG_MIME));

    // moving case should be the last one, as buffer will be lost in the process
    uploader.uploadBackground(background[1], prc::makePayloadHolderByMovingData(prc::move(buffer), JPEG_MIME));

    if (saveAsJpeg(backgroundImage, BACKGROUND_FILE) != 0)
    {
        LOG(ERROR) << "Failed to encode background to file";
        return;
    }

    uploader.uploadBackground(background[2], prc::makePayloadHolderByReferencingFileAutodelete(BACKGROUND_FILE));
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

    fb.extId = "07AB428E962A401D9DCF5A92E0DA3098";

    ts_pair_t timestamps = generateFlipbookTimestamps();
    fb.begin = timestamps.first;
    fb.end = timestamps.second;

    fb.frameHeight = FLIPBOOK_SIZE.height;
    fb.frameWidth = FLIPBOOK_SIZE.width;
    fb.numberOfFrames = 60;

    uploader.uploadFlipbook(fb, prc::makePayloadHolderByReferencingFileAutodelete(FLIPBOOK_FILE));
}

static void testObjectSnapshotUploading(prc::ArtifactUploader& uploader)
{
    cv::Mat roi = generateBackgroundImage(ROI_RECT.size(), BACKGROUND_COLOR, ROI_TEXT, TEXT_COLOR);
    buf_t buffer;

    if (saveAsJpeg(roi, buffer) != 0)
    {
        LOG(ERROR) << "Failed to save ROI image to memory";
        return;
    }

    prc::ObjectSnapshot os;
    os.extId = "97F31AFA11B84B63AEFADB7CEAA8649B";

    prc::timestamp_t ts = generateTimestamp();
    os.begin = ts;
    os.end = ts + 10;

    os.frameHeight = ROI_RECT.height;
    os.frameWidth = ROI_RECT.width;
    os.locationX = ROI_RECT.x;
    os.locationY = ROI_RECT.y;
    os.imageHeight = BACKGROUND_SIZE.height;
    os.imageWidth = BACKGROUND_SIZE.width;

    std::vector<int64_t> objectIds;
    objectIds.push_back(std::numeric_limits<int64_t>::max() - 2);

    os.objectIds = objectIds;

    uploader.uploadObjectSnapshot(os, prc::makePayloadHolderByMovingData(prc::move(buffer), JPEG_MIME));
}

} // namespace test
} // namespace prism
