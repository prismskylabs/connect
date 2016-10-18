#include "processors/background.h"

#include <opencv2/core.hpp>
#include <opencv2/video.hpp>

namespace prism {
namespace connect {
namespace processors {

Background::Background() : background_model_{cv::createBackgroundSubtractorMOG2()} {}

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
