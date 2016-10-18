#include "processors/background.h"

#include <opencv2/core.hpp>
#include <opencv2/video.hpp>

namespace prism {
namespace connect {
namespace processors {

static const int kHistory = 500;
static const double kVarThreshold = 16.0;
static const bool kDetectShadows = false;

Background::Background()
        : background_model_{
                  cv::createBackgroundSubtractorMOG2(kHistory, kVarThreshold, kDetectShadows)} {}

void Background::AddImage(const cv::Mat& input) {
    background_model_->apply(input, foreground_mask_);
}

cv::Mat Background::GetBackgroundImage() {
    background_model_->getBackgroundImage(background_image_);
    return background_image_;
}

cv::Mat Background::GetForegroundMask() {
    return foreground_mask_;
}

} // namespace processors
} // namespace connect
} // namespace prism
