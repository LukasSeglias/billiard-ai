#pragma once

#include "macro_definition.hpp"
#include <opencv2/opencv.hpp>

namespace billiard::snooker {

    struct Histogram {
        cv::Mat histogram;
        cv::Point maxLocation;
    };

}