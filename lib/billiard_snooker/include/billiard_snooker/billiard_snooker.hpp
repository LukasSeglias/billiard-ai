#pragma once

#include "macro_definition.hpp"
#include <opencv2/opencv.hpp>
#include <opencv2/aruco.hpp>
#include "type.hpp"
#include <billiard_detection/billiard_detection.hpp>
#include <billiard_search/type.hpp>
#include <glm/glm.hpp>
#include <unordered_map>
#include <string>

namespace billiard::snooker {

    #define SPOT_REPLACE_RADIUS_MULTIPLIER 5

    struct EXPORT_BILLIARD_SNOOKER_LIB SnookerDetectionConfig {
        bool valid = false;

        // Scaling of input image
        double scale = 1.0;

        // Error margin for ball radii
        double radiusErrorLow = 20 /* % (percent) */;
        double radiusErrorHigh = 10 /* % (percent) */;

        cv::HoughModes houghMethod = cv::HOUGH_GRADIENT;
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

    struct Cluster {
        std::string type;
        glm::vec3 point;
        Cluster(std::string type, glm::vec3 point): type(std::move(type)), point(point) {}
    };

    struct EXPORT_BILLIARD_SNOOKER_LIB SnookerClassificationConfig {
        bool valid = false;

        // Factor of the ball's radius to be used to define a ROI for classification
        // Example: roiRadius = ballRadius * roiRadiusFactor
        double roiRadiusFactor = 0.5;
        double roiRadius = 0.0;

        cv::Size blurSize {5, 5};

        cv::Point2d yellowHue {10, 35};
        cv::Point2d yellowSaturation {90, 255};
        cv::Point2d yellowValue {245, 255};

        cv::Point2d whiteHue {0, 40};
        cv::Point2d whiteSaturation {0, 80};
        cv::Point2d whiteValue {245, 255};

        cv::Point2d blackValue {0, 100};

        cv::Point2d blueHue {95, 110};
        cv::Point2d greenHue {80, 94};

        cv::Point2d redHue1 {0, 10};
        cv::Point2d redHue2 {170, 180};
        cv::Point2d redSaturation {240, 255};
        cv::Point2d redValue {160, 255};

        cv::Point2i brownHue {0, 10};
        cv::Point2d brownSaturation {175, 250};
        cv::Point2d brownValue {150, 255};

        cv::Point2d pinkSaturation {0, 240};
        cv::Point2d pinkValue {245, 255};

        std::vector<Cluster> clusters {
                // Based on images under ./resources/test_classification/with_projector_on/with_halo_2/
                // class: BROWN    samples:  32  avg hue1: 3.40625    median hue1: 3 (32)    avg hue2: -nan(ind)  median hue2: 0 (0)     avg saturation: 226.875    median saturation: 228   avg value: 216.5      median value: 233
                // class: PINK     samples:  32  avg hue1: 3.2        median hue1: 3 (5)     avg hue2: 175.407    median hue2: 175 (27)  avg saturation: 96.375     median saturation: 97    avg value: 253.344    median value: 254
                // class: RED      samples: 224  avg hue1: 0          median hue1: 0 (192)   avg hue2: 176        median hue2: 176 (32)  avg saturation: 252.585    median saturation: 253   avg value: 241.683    median value: 250
                // class: BLACK    samples:  32  avg hue1: 20.6667    median hue1: 30 (3)    avg hue2: 165.448    median hue2: 167 (29)  avg saturation: 91.9375    median saturation: 89    avg value: 42.75      median value: 44
                // class: YELLOW   samples:  32  avg hue1: 28.25      median hue1: 30 (32)   avg hue2: -nan(ind)  median hue2: 0 (0)     avg saturation: 249.312    median saturation: 253   avg value: 254.719    median value: 255
                // class: WHITE    samples:  32  avg hue1: 32.9688    median hue1: 35 (32)   avg hue2: -nan(ind)  median hue2: 0 (0)     avg saturation: 8.75       median saturation: 6     avg value: 255        median value: 255
                // class: BLUE     samples:  32  avg hue1: 100.688    median hue1: 101 (32)  avg hue2: -nan(ind)  median hue2: 0 (0)     avg saturation: 254.188    median saturation: 255   avg value: 211.375    median value: 200
                // class: GREEN    samples:  32  avg hue1: 91.5       median hue1: 92 (32)   avg hue2: -nan(ind)  median hue2: 0 (0)     avg saturation: 253.125    median saturation: 255   avg value: 140.562    median value: 142
                // class: UNKNOWN  samples:   0  avg hue1: -nan(ind)  median hue1: 0 (0)     avg hue2: -nan(ind)  median hue2: 0 (0)     avg saturation: -nan(ind)  median saturation: 0     avg value: -nan(ind)  median value: 0
                Cluster { "BROWN",  {   3, 228, 233 } },
                Cluster { "PINK",   {   3,  97, 254 } },
                Cluster { "PINK",   { 175,  97, 254 } },
                Cluster { "RED",    {   0, 253, 250 } },
                Cluster { "RED",    { 176, 253, 250 } },
                Cluster { "BLACK",  { 30,  89,  44 } },
                Cluster { "BLACK",  { 167,  89,  44 } },
                Cluster { "YELLOW", {  30, 253, 255 } },
                Cluster { "WHITE",  {  35,   6, 255 } },
                Cluster { "BLUE",   { 101, 255, 200 } },
                Cluster { "GREEN",  {  92, 255, 142 } }
        };

    };

    struct EXPORT_BILLIARD_SNOOKER_LIB Spot {
        glm::vec2 _position;
    };

    struct EXPORT_BILLIARD_SNOOKER_LIB SnookerSearchConfig {
        std::unordered_map<std::string, Spot> spots;
        glm::vec2 headRailDirection;
        float diameterSquared;
        float radius;
        glm::vec2 _minimum;
        glm::vec2 _maximum;
    };

    EXPORT_BILLIARD_SNOOKER_LIB bool configure(const billiard::detection::DetectionConfig& detectionConfig);
    EXPORT_BILLIARD_SNOOKER_LIB void searchConfig(const SnookerSearchConfig& searchConfig);

    EXPORT_BILLIARD_SNOOKER_LIB billiard::detection::State detect(const billiard::detection::State& previousState, const cv::Mat& image);
    EXPORT_BILLIARD_SNOOKER_LIB void classify(const billiard::detection::State& previousState,
                                              billiard::detection::State& currentState,
                                              const cv::Mat& image);
    EXPORT_BILLIARD_SNOOKER_LIB void classify(billiard::detection::Ball& ball, const cv::Mat& image);

    EXPORT_BILLIARD_SNOOKER_LIB double scoreForPottedBall(const std::string& ballType);

    EXPORT_BILLIARD_SNOOKER_LIB billiard::search::Search nextSearch(const billiard::search::State& state,
                                                                    const std::vector<std::string>& previousTypes);

    EXPORT_BILLIARD_SNOOKER_LIB billiard::search::State stateAfterBreak(const billiard::search::State& state,
                                                                        const std::unordered_map<std::string, std::string>& ids);

    EXPORT_BILLIARD_SNOOKER_LIB bool
    validEndState(const std::vector<std::string>& expectedTypes, const billiard::search::node::Layer& layer);
}