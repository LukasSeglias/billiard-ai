#pragma once

#include "macro_definition.hpp"
#include <opencv2/opencv.hpp>
#include <opencv2/aruco.hpp>
#include "type.hpp"
#include <billiard_detection/billiard_detection.hpp>
#include <billiard_search/type.hpp>

namespace billiard::snooker {

    struct EXPORT_BILLIARD_SNOOKER_LIB SnookerDetectionConfig {
        bool valid = false;

        // Scaling of input image
        double scale = 1.0;

        // Error margin for ball radii
        double radiusErrorLow = 20 /* % (percent) */;
        double radiusErrorHigh = 10 /* % (percent) */;

        // Hough: minimal distance between centers
        double houghMinDistance;
        // Hough: higher threshold of the two passed to the Canny edge detector (lower threshold is twice smaller)
        double houghCannyHigherThreshold = 60;
        // Hough: accumulator threshold for the circle centers (if the value is smaller, the more "circles" are detected)
        double houghAccumulatorThreshold = 7;
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

    struct EXPORT_BILLIARD_SNOOKER_LIB SnookerClassificationConfig {
        bool valid = false;

        // Factor of the ball's radius to be used to define a ROI for classification
        // Example: roiRadius = ballRadius * roiRadiusFactor
        double roiRadiusFactor = 0.5;
        double roiRadius = 0.0;

        cv::Size blurSize {5, 5};
    };

    EXPORT_BILLIARD_SNOOKER_LIB bool configure(const billiard::detection::DetectionConfig& detectionConfig);

    EXPORT_BILLIARD_SNOOKER_LIB billiard::detection::State detect(const billiard::detection::State& previousState, const cv::Mat& image);
    EXPORT_BILLIARD_SNOOKER_LIB void classify(const billiard::detection::State& previousState,
                                              billiard::detection::State& currentState,
                                              const cv::Mat& image);
    EXPORT_BILLIARD_SNOOKER_LIB void classify(billiard::detection::Ball& ball, const cv::Mat& image);

    EXPORT_BILLIARD_SNOOKER_LIB std::string nextSearchType(const std::string& previousType); // TODO: Naming

    EXPORT_BILLIARD_SNOOKER_LIB billiard::search::node::Layer
    stateAfterBreak(const billiard::search::node::Layer& layer); // TODO: Naming

    EXPORT_BILLIARD_SNOOKER_LIB bool
    validEndState(const std::string& expectedType, const billiard::search::node::Layer& layer); // TODO: Naming
}