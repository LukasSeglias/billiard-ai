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
#ifdef BILLIARD_SNOOKER_CLASSIFICATION_CLUSTERS
        cv::Point2d yellowSaturation {90, 255};
#else
        cv::Point2d yellowSaturation {170, 255};
#endif
        cv::Point2d yellowValue {245, 255};

#ifdef BILLIARD_SNOOKER_CLASSIFICATION_CLUSTERS
        cv::Point2d whiteHue {0, 40};
#else
        cv::Point2d whiteHue {10, 40};
#endif
        cv::Point2d whiteSaturation {0, 80};
        cv::Point2d whiteValue {245, 255};

        cv::Point2d blackValue {0, 100};
        cv::Point2d blueHue {95, 110};
        cv::Point2d greenHue {80, 94};

        cv::Point2d redHue1 {0, 10};
        cv::Point2d redHue2 {170, 180};

        cv::Point2i brownHue {0, 10};
        cv::Point2d brownValue {150, 255};
#ifdef BILLIARD_SNOOKER_CLASSIFICATION_CLUSTERS
        cv::Point2d brownSaturation {175, 250};
#else
        cv::Point2d brownSaturation {180, 240};
#endif

        cv::Point2d pinkSaturation {0, 180};
        cv::Point2d pinkValue {245, 255};

        std::vector<Cluster> clusters {
                // Based on images under ./resources/test_classification/with_projector_on/with_halo/
//                Cluster { "BROWN",  {   2, 210, 225 } },
//                Cluster { "PINK",   { 171, 106, 250 } },
//                Cluster { "RED",    {   0, 252, 240 } },
//                Cluster { "RED",    { 179, 252, 240 } },
//                Cluster { "BLACK",  { 163,  91,  37 } },
//                Cluster { "YELLOW", {  30, 252, 250 } },
//                Cluster { "WHITE",  {  30,   6, 254 } },
//                Cluster { "BLUE",   { 102, 253, 241 } },
//                Cluster { "GREEN",  {  93, 253, 150 } }
                // Based on images under ./resources/test_classification/with_projector_on/without_text/
                //class: BROWN samples: 24 avg hue1: 4.20833 (24) avg hue2: -nan(ind) (0) avg saturation: 191.083 avg value: 195.667
                //class: PINK samples: 24 avg hue1: 4.5 (14) avg hue2: 175.7 (10) avg saturation: 92.2083 avg value: 254.375
                //class: RED samples: 358 avg hue1: -nan(ind) (0) avg hue2: 176.542 (358) avg saturation: 244.788 avg value: 223.12
                //class: BLACK samples: 24 avg hue1: 34.6429 (14) avg hue2: 165.9 (10) avg saturation: 88 avg value: 42.25
                //class: YELLOW samples: 24 avg hue1: 25.875 (24) avg hue2: -nan(ind) (0) avg saturation: 239.333 avg value: 254.75
                //class: WHITE samples: 24 avg hue1: 33.5 (24) avg hue2: -nan(ind) (0) avg saturation: 16 avg value: 255
                //class: BLUE samples: 24 avg hue1: 99.4583 (24) avg hue2: -nan(ind) (0) avg saturation: 254.042 avg value: 196.75
                //class: GREEN samples: 24 avg hue1: 89.625 (24) avg hue2: -nan(ind) (0) avg saturation: 248.583 avg value: 136.792
                //class: UNKNOWN samples: 0 avg hue1: -nan(ind) (0) avg hue2: -nan(ind) (0) avg saturation: -nan(ind) avg value: -nan(ind)
                Cluster { "BROWN",  {   4, 191, 195 } },
                Cluster { "PINK",   {   4,  92, 254 } },
                Cluster { "PINK",   { 176,  92, 254 } },
                Cluster { "RED",    {   0, 244, 223 } },
                Cluster { "RED",    { 179, 244, 223 } },
                Cluster { "BLACK",  { 34,  88,  42 } },
                Cluster { "BLACK",  { 166,  88,  42 } },
                Cluster { "YELLOW", {  26, 239, 254 } },
                Cluster { "WHITE",  {  33,   16, 255 } },
                Cluster { "BLUE",   { 99, 254, 196 } },
                Cluster { "GREEN",  {  89, 248, 136 } }
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