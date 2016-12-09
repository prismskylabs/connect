#ifndef PRISM_CONNECT_PROCESSORS_Background_H_
#define PRISM_CONNECT_PROCESSORS_Background_H_

#if 0

#include <opencv2/core.hpp>
#include <opencv2/video.hpp>

namespace prism {
namespace connect {
namespace processors {

class Background {
  public:
    Background();

    void AddImage(const cv::Mat& input);
    cv::Mat GetBackgroundImage();
    cv::Mat GetForegroundMask();

  private:
    cv::Ptr<cv::BackgroundSubtractor> background_model_;
    cv::Mat background_image_;
    cv::Mat foreground_mask_;
};

} // namespace processors
} // namespace connect
} // namespace prism

#endif

#endif // PRISM_CONNECT_PROCESSORS_Background_H_
