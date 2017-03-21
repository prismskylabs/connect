/*
 * Copyright (C) 2017 Prism Skylabs
 */
#ifndef PRISM_TEST_UTILS_H
#define PRISM_TEST_UTILS_H

#include "opencv2/core.hpp"
#include "common-types.h"

// Helper classes for internal use i.e. their interface may change in any time
namespace prism
{
namespace test
{

typedef std::vector<uint8_t> buf_t;
typedef std::pair<prism::connect::timestamp_t, prism::connect::timestamp_t> ts_pair_t;

cv::Mat generateBackgroundImage(const cv::Size size, const cv::Scalar& bgColor,
                                const std::string& text, const cv::Scalar& textColor);

int saveAsJpeg(cv::Mat image, buf_t& buffer);

// returns zero on success
int saveAsJpeg(cv::Mat image, const std::string& filePath);

// duration is 1 minute, FPS is 1, saves to mp4 file
// returns 0 on success
int generateFlipbookFile(const cv::Size frameSize, const cv::Scalar& bgColor,
                         const std::string& text, const cv::Scalar& textColor,
                         const std::string& filePath);

prism::connect::timestamp_t generateTimestamp();

ts_pair_t generateFlipbookTimestamps();

} // namespace test
} // namespace prism

#endif // PRISM_TEST_UTILS_H
