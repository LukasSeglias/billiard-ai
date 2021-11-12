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

        cv::HoughModes houghMethod = cv::HOUGH_GRADIENT; // TODO: try HOUGH_GRADIENT_ALT for better accuracy?
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
        cv::Point2d saturationFilter {150, 255};
        cv::Point2d saturatedBallsValueFilter {60, 255};
        cv::Point2d blackValueFilter{0, 80};
        cv::Point2d whitePinkValueFilter{200, 255};

        // Rectangular 3x3 structuring element for morphological operations
        cv::Mat morphElementRect3x3;

        // Binary mask (255=true) to determine possible locations of balls (only on the green inner table)
        // Scaled by DetectionConfig::scale
        cv::Mat innerTableMask;

        // Binary mask (255=true) masking everything in the image that is inside the rectangle defined by the rails of the table.
        // Scaled by DetectionConfig::scale
        cv::Mat railMask;
    };

    struct EXPORT_BILLIARD_SNOOKER_LIB SnookerClassificationConfig {
        bool valid = false;

        // Factor of the ball's radius to be used to define a ROI for classification
        // Example: roiRadius = ballRadius * roiRadiusFactor
        double roiRadiusFactor = 0.5;
        double roiRadius = 0.0;

        cv::Size blurSize {5, 5};

        cv::Point2d yellowHue {10, 35};
        cv::Point2d yellowSaturation {170, 255};
        cv::Point2d yellowValue {245, 255};

        cv::Point2d whiteHue {10, 40};
        cv::Point2d whiteSaturation {0, 80};
        cv::Point2d whiteValue {245, 255};

        cv::Point2d blackValue {0, 100};
        cv::Point2d blueHue {95, 110};
        cv::Point2d greenHue {80, 94};

        cv::Point2d redHue1 {0, 10};
        cv::Point2d redHue2 {170, 180};

        cv::Point2d brownValue {150, 255};
        cv::Point2d brownSaturation {180, 240};

        cv::Point2d pinkSaturation {0, 180};
        cv::Point2d pinkValue {245, 255};
    };

    EXPORT_BILLIARD_SNOOKER_LIB bool configure(const billiard::detection::DetectionConfig& detectionConfig);

    EXPORT_BILLIARD_SNOOKER_LIB billiard::detection::State detect(const billiard::detection::State& previousState, const cv::Mat& image);
    EXPORT_BILLIARD_SNOOKER_LIB void classify(const billiard::detection::State& previousState,
                                              billiard::detection::State& currentState,
                                              const cv::Mat& image);
    EXPORT_BILLIARD_SNOOKER_LIB void classify(billiard::detection::Ball& ball, const cv::Mat& image);

    EXPORT_BILLIARD_SNOOKER_LIB double scoreForPottedBall(const std::string& ballType);

    EXPORT_BILLIARD_SNOOKER_LIB billiard::search::Search nextSearch(const billiard::search::Search& previousSearch,
                                                                    const std::vector<std::string>& previousTypes); // TODO: Naming

    EXPORT_BILLIARD_SNOOKER_LIB billiard::search::node::Layer
    stateAfterBreak(const billiard::search::node::Layer& layer); // TODO: Naming

    EXPORT_BILLIARD_SNOOKER_LIB bool
    validEndState(const std::vector<std::string>& expectedTypes, const billiard::search::node::Layer& layer); // TODO: Naming
}