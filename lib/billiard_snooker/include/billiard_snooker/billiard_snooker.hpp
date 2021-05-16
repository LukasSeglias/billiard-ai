#pragma once

#include "macro_definition.hpp"
#include <opencv2/opencv.hpp>
#include <opencv2/aruco.hpp>
#include "type.hpp"
#include <billiard_detection/billiard_detection.hpp>

namespace billiard::snooker {

    struct EXPORT_BILLIARD_SNOOKER_LIB SnookerDetectionConfig {
        bool valid = false;

        // Scaling of input image
        double scale = 1.0;

        // Error margin for ball radii
        double radiusErrorLow = 20 /* % (percent) */;
        double radiusErrorHigh = 0 /* % (percent) */;

        // Hough: minimal distance between centers
        double houghMinDistance;
        // Hough: higher threshold of the two passed to the Canny edge detector (lower threshold is twice smaller)
        double houghCannyHigherThreshold = 60;
        // Hough: accumulator threshold for the circle centers (if the value is smaller, the more "circles" are detected)
        double houghAccumulatorThreshold = 5;
        // Hough: min radius in pixels
        int minRadius;
        // Hough: max radius in pixels
        int maxRadius;

        // HSV Filters for different balls
        cv::Point2d saturationFilter {100, 255};
        cv::Point2d blackValueFilter{0, 80};
        cv::Point2d whitePinkValueFilter{200, 255};

        // Rectangular 3x3 structuring element for morphological operations
        cv::Mat morphElementRect3x3;

        // Binary mask (255=true) to determine possible locations of balls (only on the green inner table)
        // Scaled by DetectionConfig::scale
        cv::Mat innerTableMask;
    };

    EXPORT_BILLIARD_SNOOKER_LIB bool configure(const billiard::detection::DetectionConfig& detectionConfig);

    EXPORT_BILLIARD_SNOOKER_LIB billiard::detection::State detect(const billiard::detection::State& previousState, const cv::Mat& image);
}