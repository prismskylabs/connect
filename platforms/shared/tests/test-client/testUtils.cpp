/*
 * Copyright (C) 2017 Prism Skylabs
 */
#include "testUtils.h"
#include "opencv2/imgproc.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/videoio.hpp"
#include "boost/format.hpp"
#include "public-util.h"

namespace prc = prism::connect;

namespace prism
{
namespace test
{

struct DefaultCompressionParams
{
    DefaultCompressionParams()
    {
        data.push_back(cv::IMWRITE_JPEG_QUALITY);
        data.push_back(95);
    }

    std::vector<int> data;
};


cv::Mat generateBackgroundImage(const cv::Size size, const cv::Scalar& bgColor,
                                const std::string& text, const cv::Scalar& textColor)
{
    cv::Mat rv(size, CV_8UC3, bgColor);
    int fontFace = cv::FONT_HERSHEY_SCRIPT_SIMPLEX;
    double fontScale = 2;
    int thickness = 3;
    cv::Size textSize = cv::getTextSize(text.c_str(), fontFace, fontScale, thickness, 0);

    cv::Point textOrg((size.width - textSize.width)/2,
                      (size.height + textSize.height)/2);

    cv::putText(rv, text.c_str(), textOrg, fontFace, fontScale,
                textColor, thickness, 8);

    return rv;
}

int saveAsJpeg(cv::Mat image, buf_t& buffer)
{
    DefaultCompressionParams compressionParams;
    bool rv = cv::imencode(".jpg", image, buffer, compressionParams.data);
    return rv ? 0 : -1;
}

int saveAsJpeg(cv::Mat image, const std::string& filePath)
{
    DefaultCompressionParams compressionParams;
    bool rv = cv::imwrite(filePath.c_str(), image, compressionParams.data);
    return rv ? 0 : -1;
}

int generateFlipbookFile(const cv::Size frameSize, const cv::Scalar& bgColor,
                         const std::string& text, const cv::Scalar& textColor,
                         const std::string& filePath)
{
    cv::VideoWriter vw;
    const float FPS = 1.0;
    const int FRAME_NUM = 60;

    vw.open(filePath.c_str(), cv::VideoWriter::fourcc('H','2','6','4'),
            FPS, frameSize, 1);

    if (!vw.isOpened())
        return -1;

    for (int i = 0; i < FRAME_NUM; ++i)
    {
        std::string curText = (boost::format("%s %02d") % text % i).str();
        cv::Mat frame = generateBackgroundImage(frameSize, bgColor, curText, textColor);
        vw.write(frame);
    }

    return 0;
}

prc::timestamp_t generateTimestamp()
{
    // time in past
    boost::chrono::system_clock::time_point tp = boost::chrono::system_clock::now() - boost::chrono::hours(1);
    return prc::toTimestamp(tp);
}

ts_pair_t generateFlipbookTimestamps()
{
    const int MS_IN_MINUTE = 60000;
    prc::timestamp_t timestampMs = generateTimestamp();
    timestampMs = timestampMs / MS_IN_MINUTE * MS_IN_MINUTE;
    return ts_pair_t(timestampMs, timestampMs + MS_IN_MINUTE);
}

}
}
