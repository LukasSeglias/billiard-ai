#include <gtest/gtest.h>
#include <opencv2/opencv.hpp>
#include <iostream>
#include <billiard_detection/billiard_detection.hpp>
#include <billiard_snooker/billiard_snooker.hpp>
#include <billiard_capture/billiard_capture.hpp>
#include <librealsense2/rs.hpp>
#include "config.hpp"

void drawHoughResult(cv::Mat& image, std::vector<cv::Vec3f>& circles) {
    for(auto c : circles) {
        cv::Point center = cv::Point(c[0], c[1]);
        uint8_t radius = c[2];
        cv::Rect roi(center.x - radius, center.y - radius, radius * 2, radius * 2);
        if (roi.x >= 0 && roi.y >= 0 && roi.width <= image.cols && roi.height <= image.rows) {

            // circle center
            cv::circle(image, center, 1, cv::Scalar(0, 100, 100), 3, cv::LINE_AA);
            // circle outline
            cv::circle(image, center, radius, cv::Scalar(255, 0, 255), 1, cv::LINE_AA);
        }
    }
}

void hsvFromBgr(cv::Mat& bgr, cv::Mat& hue, cv::Mat& saturation, cv::Mat& value) {
    cv::Mat hsv;
    cv::cvtColor(bgr, hsv, cv::COLOR_BGR2HSV);
    std::vector<cv::Mat> channels;
    cv::split(hsv, channels);
    hue = channels[0];
    saturation = channels[1];
    value = channels[2];
}

inline std::vector<cv::Vec3f> filterCircles(const std::vector<cv::Vec3f>& circles, const cv::Rect& rect) {
    std::vector<cv::Vec3f> filteredCircles;
    for(auto& circle : circles) {
        auto center = cv::Point2i{(int)circle[0], (int)circle[1]};
        if (rect.contains(center)) {
            filteredCircles.push_back(circle);
        }
    }
    return filteredCircles;
}

inline std::vector<cv::Vec3f> filterCircles(const std::vector<cv::Vec3f>& circles, const cv::Mat& mask, bool expectedValue) {
    std::vector<cv::Vec3f> filteredCircles;
    for(auto& circle : circles) {
        auto center = cv::Point2i{(int)circle[0], (int)circle[1]};
        if ((bool)mask.at<char>(center.y, center.x) == expectedValue) {
            filteredCircles.push_back(circle);
        }
    }
    return filteredCircles;
}

inline std::vector<cv::Point2d> circleCenters(const std::vector<cv::Vec3f>& circles) {
    std::vector<cv::Point2d> imagePoints;
    for (auto& circle : circles) {
        imagePoints.emplace_back(circle[0], circle[1]);
    }
    return imagePoints;
}

cv::Point2i findClosestPoint(cv::Point2i& point, std::vector<cv::Point2i>& points) {

    cv::Point2i closest {0,0};
    double min_dist = 999999;
    for(auto& p : points) {
        auto test = point - p;
        double dist = cv::abs(cv::norm(test));
        if (dist < min_dist) {
            min_dist = dist;
            closest = p;
        }
    }
    return closest;
}

void drawCircles(cv::Mat& output, std::vector<cv::Vec3f>& circles, cv::Scalar color) {
    for(auto c : circles) {
        cv::Point center = cv::Point(c[0], c[1]);
        uint8_t radius = c[2];
        cv::Rect roi(center.x - radius, center.y - radius, radius * 2, radius * 2);
        if (roi.x >= 0 && roi.y >= 0 && roi.width <= output.cols && roi.height <= output.rows) {
            cv::circle(output, center, 13, color, cv::FILLED);
        }
    }
}

void printCoordinates(const std::string& context,
                      const std::vector<cv::Point2d>& imagePoints,
                      const std::vector<cv::Point3d>& worldPoints,
                      const std::vector<cv::Point2d>& modelPoints) {

    for(int i = 0; i < imagePoints.size(); i++) {
        auto& imagePoint = imagePoints[i];
        auto& worldPoint = worldPoints[i];
        auto& modelPoint = modelPoints[i];
        std::cout << "[" << context << "]"
                  << " image point: " << imagePoint
                  << " world point: " << worldPoint
                  << " model point: " << modelPoint
                  << std::endl;
    }
}

cv::Ptr<cv::aruco::Board> createArucoBoard3(float markerLengthMilimeters) {
    // Based on the tutorial on https://docs.opencv.org/4.5.1/d5/dae/tutorial_aruco_detection.html
    const int nMarkers = 4;
    const int markerSize = 3;
    cv::Ptr<cv::aruco::Dictionary> dictionary = cv::aruco::generateCustomDictionary(nMarkers, markerSize);
    std::vector<int> ids = {0, 1, 2, 3};

    const float side = markerLengthMilimeters;

    auto cornerPositions = [&side](const cv::Point3f& bottomLeft) -> std::vector<cv::Point3f> {
        return {
                bottomLeft + cv::Point3f{ 0, side, 0 },
                bottomLeft + cv::Point3f{ side, side, 0 },
                bottomLeft + cv::Point3f{ side, 0, 0 },
                bottomLeft + cv::Point3f{ 0, 0, 0 },
        };
    };

    const float separatorX = 1722; // Horizontal distance between bottom-left points of each marker in millimeters
    const float separatorYLeft = 1085; // Vertical distance between bottom-left points of each marker in millimeters
    const float separatorYRight = 1084; // Vertical distance between bottom-left points of each marker in millimeters
    cv::Point3f centerOffset{markerLengthMilimeters/2, markerLengthMilimeters/2, 0};
    std::vector<std::vector<cv::Point3f>> objPoints = {
            cornerPositions(cv::Point3f{0, 0, 0} - centerOffset),            // Marker 0
            cornerPositions(cv::Point3f{separatorX, 0, 0} - centerOffset),       // Marker 1
            cornerPositions(cv::Point3f{separatorX, separatorYRight, 0} - centerOffset), // Marker 2
            cornerPositions(cv::Point3f{0, separatorYLeft, 0} - centerOffset),       // Marker 3
    };

    cv::Ptr<cv::aruco::Board> board = cv::aruco::Board::create(objPoints, dictionary, ids);
    return board;
}

/**
 * Get rail rectangle in model coordinates
 */
std::vector<cv::Point2d> getRailRect(double innerTableLength, double innerTableWidth, const cv::Point2d& innerTableCenter) {
    double middlePocketRadius = 50;
    double cornerPocketRadius = 55;
    double railTop = innerTableWidth;
    double railBottom = 0.0;
    double railLeft = 0.0;
    double railRight = innerTableLength;
    return { // In model coordinates
            // Bottom-left
            cv::Point2d { railLeft, railBottom } - innerTableCenter,
            // Top-left
            cv::Point2d { railLeft, railTop } - innerTableCenter,
            // Top-right
            cv::Point2d { railRight, railTop } - innerTableCenter,
            // Bottom-right
            cv::Point2d { railRight, railBottom } - innerTableCenter,
    };
}

struct Pocket {
    cv::Point2d center;
    double radius;
    Pocket(const cv::Point& center, double radius): center(center), radius(radius) {};
};

/**
 * Get table pockets in model coordinates
 */
std::vector<Pocket> getPockets(double innerTableLength,
                               double innerTableWidth,
                               const cv::Point2d& innerTableCenter) {
    double middlePocketRadius = 70;
    double cornerPocketRadius = 70;
    double pocketTop = innerTableWidth;
    double pocketBottom = 0.0;
    double pocketLeft = 0.0;
    double pocketRight = innerTableLength;
    double pocketMiddle = innerTableLength/2;
    return { // In model coordinates
            Pocket { cv::Point2d { pocketLeft + 25,   pocketBottom + 15 } - innerTableCenter, cornerPocketRadius }, // Bottom-left
            Pocket { cv::Point2d { pocketLeft + 25,   pocketTop - 15 }    - innerTableCenter, cornerPocketRadius }, // Top-left
            Pocket { cv::Point2d { pocketMiddle, pocketTop + 15 }    - innerTableCenter, middlePocketRadius }, // Top-middle
            Pocket { cv::Point2d { pocketRight - 25,  pocketTop - 15 }    - innerTableCenter, cornerPocketRadius }, // Top-right
            Pocket { cv::Point2d { pocketRight - 25,  pocketBottom + 15 } - innerTableCenter, cornerPocketRadius }, // Bottom-right
            Pocket { cv::Point2d { pocketMiddle, pocketBottom - 15 } - innerTableCenter, middlePocketRadius }, // Bottom-middle
    };
}

inline void drawRailRectLines(cv::Mat& output, const std::vector<cv::Point2d>& points) {

    cv::Scalar color(0, 0, 255);
    int thickness = 1;
    cv::line(output, points[0], points[1], color, thickness);
    cv::line(output, points[1], points[2], color, thickness);
    cv::line(output, points[2], points[3], color, thickness);
    cv::line(output, points[3], points[0], color, thickness);
}

inline std::vector<cv::Point> toIntPoints(const std::vector<cv::Point2d>& points) {
    std::vector<cv::Point> intPoints;
    for (auto& imagePoint : points) {
        intPoints.emplace_back((int) imagePoint.x,  (int) imagePoint.y);
    }
    return intPoints;
}

// TODO: prüfen, ob es Kreise gibt, welche (fast) an derselben Position stehen
// TODO: Es werden fälschlicherweise Kugeln in den löchern detektiert.
// TODO: Es werden fälschlicherweise Kugeln detektiert, wo keine sind.
// TODO: Es werden fälschlicherweise Schatten als Kugeln detektiert.

// TODO: inner table mask filtert kugeln raus, welche nahe neben dem loch sind
TEST(BallDetectionTests, combined) {

    bool live = false;
    std::vector<std::string> imagePaths = {
            "./resources/test_detection/1.png",
            "./resources/test_detection/2.png",
            "./resources/test_detection/3.png",
            "./resources/test_detection/4.png",
            "./resources/test_detection/5.png",
            "./resources/test_detection/6.png",
            "./resources/test_detection/7.png",
            "./resources/test_detection/8.png",
            "./resources/test_detection/9.png",
            "./resources/test_detection/10.png",
            "./resources/test_detection/11.png",
            "./resources/test_detection/12.png",
            "./resources/test_detection/13.png",
            "./resources/test_detection/14.png",
            "./resources/test_detection/15.png",
            "./resources/test_detection/16.png",
            "./resources/test_detection/17.png",
            "./resources/test_detection/18.png",
            "./resources/test_detection/19.png",
            "./resources/test_detection/20.png",
    };

    std::vector<int> expectedBallCounts = {
            20,
            20,
            20,
            20,
            20,
            20,
            20,
            20,
            20,
            20,
            20,
            20,
            20,
            22,
            22,
            22,
            21,
            20,
            19,
            17
    };

    double innerTableLength = 1881.0; // millimeters
    double innerTableWidth  =  943;   // millimeters
    double ballDiameter     = 52.3;   // millimeters
    double ballRadius       = ballDiameter/2; // millimeters
    cv::Point2d innerTableCenter { innerTableLength/2, innerTableWidth/2 };

    const float markerLength = 50; // length of marker in millimeters in the real world
    cv::Ptr<cv::aruco::Board> board = createArucoBoard3(markerLength);
    billiard::detection::CameraIntrinsics intrinsics = getIntrinsics_realsense_full_hd();
    billiard::detection::Plane plane {{0, 0, 14.2 - ballRadius}, {0, 0, 1}};

    cv::Vec3d worldToRail { 79, -71.5, 0.0 };
    cv::Vec3d railToModel { innerTableCenter.x, innerTableCenter.y, 0.0 };

    billiard::detection::WorldToModelCoordinates worldToModel;
    worldToModel.translation = worldToRail - railToModel;

    double railWorldPointZComponent = 0; // TODO: insert real value here
    std::vector<cv::Point2d> railRect = getRailRect(innerTableLength, innerTableWidth, innerTableCenter);
    std::vector<Pocket> pockets = getPockets(innerTableLength, innerTableWidth, innerTableCenter);

    double scale = 0.5;
    double resolutionX = 2048 * scale;
    double tableLength = 2130;
    double errorLow = 20;
    double errorHigh = 0;

    long double pixelsPerMillimeter = static_cast<long double>(resolutionX) / tableLength;
    uint16_t radiusInPixel = ceil((ballDiameter / 2.0) * pixelsPerMillimeter);
    uint16_t errorRadiusLow = ceil(radiusInPixel * (errorLow / 100.0));
    uint16_t errorRadiusHigh = ceil(radiusInPixel * (errorHigh / 100.0));
    uint16_t minRadius = radiusInPixel - errorRadiusLow;
    uint16_t maxRadius = radiusInPixel + errorRadiusHigh;

    // minimal distance between centers
    double houghMinDistance = minRadius * 2;
    // higher threshold of the two passed to the Canny edge detector (lower threshold is twice smaller)
    double houghCannyHigherThreshold = 60;
    // accumulator threshold for the circle centers (if the value is smaller, the more "circles" are detected)
    double houghAccumulatorThreshold = 5;

    cv::Point2d saturationFilter {100, 255};
    cv::Point2d blackValueFilter{0, 80};
    cv::Point2d blackSaturationFilter{0, 225};
    cv::Point2d whitePinkValueFilter{200, 255};

    std::cout << "radius in pixels: " << std::to_string(radiusInPixel) << " min radius: " << std::to_string(minRadius) << " max radius: " << std::to_string(maxRadius) << " error radius range: " << std::to_string(errorRadiusLow) << ", " << std::to_string(errorRadiusHigh) << std::endl;

    unsigned long long imageIndex = 0;
    bool imageChanged = true;
    bool showDebuggingOutput = true;
    bool showBlack = true;
    bool showSaturated = true;
    bool showWhitePink = true;
    bool showRails = true;

    cv::Mat elementRect3x3 = getStructuringElement(cv::MORPH_RECT, cv::Size(3,3), cv::Point(1, 1));

    billiard::capture::CameraCapture capture {};

    if (live) {
        if (!capture.open()) {
            std::cerr << "Unable to open image stream" << std::endl;
            return;
        }
    }

    billiard::detection::CameraToWorldCoordinateSystemConfig config;

    while(true) {
        cv::Mat original;

        if (live) {
            if (imageChanged) {
                billiard::capture::CameraFrames frames = capture.read();
                frames.color.copyTo(original);
            }
        } else {
            std::string imagePath = imagePaths[imageIndex];
            original = imread(imagePath, cv::IMREAD_COLOR);
        }

        if (imageChanged) {
            imageChanged = false;

            config = billiard::detection::configure(original, board, intrinsics);

            cv::Mat input;
            cv::resize(original, input, cv::Size(), scale, scale);

            // Build inner table mask
            cv::Mat innerTableMask(cv::Size(original.cols, original.rows), CV_8UC1, cv::Scalar(0));
            if (config.valid) {

                // Rails
                {
                    std::vector<cv::Point2d> modelPoints = railRect;
                    std::vector<cv::Point3d> worldPoints = modelPointsToWorldPoints(worldToModel, modelPoints,
                                                                                    railWorldPointZComponent);
                    std::vector<cv::Point2d> imagePoints = worldPointsToImagePoints(config, worldPoints);
                    std::cout << "------------------ RAIL-RECT ------------------" << std::endl;
                    printCoordinates("Rail-Rect", imagePoints, worldPoints, modelPoints);

                    cv::fillConvexPoly(innerTableMask, toIntPoints(imagePoints), cv::Scalar(255));

                    cv::Mat railOutput = original.clone();
                    drawRailRectLines(railOutput, imagePoints);
                    cv::resize(railOutput, railOutput, cv::Size(), scale, scale);
                    cv::imshow("rail rectangle", railOutput);
                }

                // Pockets
                {
                    std::vector<cv::Point2d> centers;
                    for (auto& pocket : pockets) {
                        centers.push_back(pocket.center);
                    }

                    std::vector<cv::Point2d> modelPoints = centers;
                    std::vector<cv::Point3d> worldPoints = modelPointsToWorldPoints(worldToModel, modelPoints,
                                                                                    railWorldPointZComponent);
                    std::vector<cv::Point2d> imagePoints = worldPointsToImagePoints(config, worldPoints);

                    for (int i = 0; i < pockets.size(); i++) {
                        auto& pocket = pockets[i];
                        // TODO: correct conversion between millimeter-radius to pixel-radius
                        int pocketPixelRadius = pocket.radius * 0.6;
                        cv::circle(innerTableMask, imagePoints[i], pocketPixelRadius, cv::Scalar{0},
                                   cv::LineTypes::FILLED);
                    }

                    if (showDebuggingOutput && showRails) {

                        cv::Mat pocketsOutput = original.clone();

                        for (int i = 0; i < pockets.size(); i++) {
                            auto& pocket = pockets[i];
                            // TODO: correct conversion between millimeter-radius to pixel-radius
                            int pocketPixelRadius = pocket.radius * 0.6;
                            cv::circle(pocketsOutput, imagePoints[i], pocketPixelRadius, cv::Scalar{0, 0, 255}, 1);
                        }

                        cv::resize(pocketsOutput, pocketsOutput, cv::Size(), scale, scale);
                        cv::imshow("pockets", pocketsOutput);
                    }
                }

                cv::resize(innerTableMask, innerTableMask, cv::Size(), scale, scale);

                if (showDebuggingOutput && showRails) {
                    cv::Mat inputMaskedByRails;
                    cv::bitwise_and(input, input, inputMaskedByRails, innerTableMask);

                    cv::imshow("Inner table mask", innerTableMask);
                    cv::imshow("Input masked by rail rect", inputMaskedByRails);
                }
            }

            // Apply blur
            cv::Mat blurred;
            cv::GaussianBlur(input, blurred, cv::Size(5, 5), 0, 0);

            cv::Mat grayscaleInput;
            cv::cvtColor(blurred, grayscaleInput, cv::COLOR_BGR2GRAY);

            // Convert image into HSV and retrieve separate channels
            cv::Mat hue, saturation, value;
            hsvFromBgr(blurred, hue, saturation, value);

            if (showDebuggingOutput) {
                cv::imshow("input", input);
                cv::imshow("blurred", blurred);
                cv::imshow("hue", hue);
                cv::imshow("saturation", saturation);
                cv::imshow("value", value);
            }

            // Filter on saturation to retrieve a mask

            cv::Mat saturationMask;
            cv::inRange(saturation, saturationFilter.x, saturationFilter.y, saturationMask);

            cv::Mat openedSaturationMask;
            cv::morphologyEx(saturationMask, openedSaturationMask, cv::MORPH_OPEN, elementRect3x3, cv::Point(-1, -1),
                             1);

            cv::Mat closedSaturationMask;
            cv::morphologyEx(openedSaturationMask, closedSaturationMask, cv::MORPH_CLOSE, elementRect3x3,
                             cv::Point(-1, -1), 2);

            cv::Mat saturatedBallMask = closedSaturationMask;

            if (showDebuggingOutput && showSaturated) {
                cv::imshow("saturationMask", saturationMask);
                cv::imshow("openedSaturationMask", openedSaturationMask);
                cv::imshow("closedSaturationMask", closedSaturationMask);
                cv::imshow("saturatedBallMask", saturatedBallMask);
            }

            cv::Mat saturationMaskedGrayscale;
            cv::bitwise_and(grayscaleInput, grayscaleInput, saturationMaskedGrayscale, saturatedBallMask);

            if (showDebuggingOutput && showSaturated) {
                cv::Mat saturationMaskedInput;
                cv::bitwise_and(input, input, saturationMaskedInput, saturatedBallMask);

                cv::Mat invMask;
                cv::bitwise_not(saturatedBallMask, invMask);
                cv::Mat maskedDelta;
                cv::bitwise_and(input, input, maskedDelta, invMask);

                cv::imshow("saturationMask - masked color", saturationMaskedInput);
                cv::imshow("saturationMask - masked grayscale", saturationMaskedGrayscale);
                cv::imshow("saturationMask - masked delta", maskedDelta);
            }

            // Hough on saturation masked input
            std::vector<cv::Vec3f> saturationCircles;
            HoughCircles(saturationMaskedGrayscale, saturationCircles, cv::HOUGH_GRADIENT, 1,
                         houghMinDistance, // minimal distance between centers
                         houghCannyHigherThreshold, // higher threshold of the two passed to the Canny edge detector (lower threshold is twice smaller)
                         houghAccumulatorThreshold, // accumulator threshold for the circle centers (if the value is smaller, the more "circles" are detected)
                         minRadius, maxRadius
            );

            if (showDebuggingOutput && showSaturated) {
                cv::Mat edges;
                cv::Canny(saturationMaskedGrayscale, edges, houghCannyHigherThreshold / 2, houghCannyHigherThreshold);

                cv::imshow("saturationMask - maskedInput grayscale edges", edges);
            }

            // Black ball mask
            cv::Mat blackMask;
            {
                cv::Mat valueMask;
                cv::inRange(value, blackValueFilter.x, blackValueFilter.y, valueMask);

                cv::Mat valueMaskAndedWithTableMask;
                cv::bitwise_and(valueMask, innerTableMask, valueMaskAndedWithTableMask);

                cv::Mat closedValueMask;
                cv::morphologyEx(valueMaskAndedWithTableMask, closedValueMask, cv::MORPH_CLOSE, elementRect3x3,
                                 cv::Point(-1, -1), 4);

                cv::Mat openedValueMask;
                cv::morphologyEx(closedValueMask, openedValueMask, cv::MORPH_OPEN, elementRect3x3, cv::Point(-1, -1),
                                 3);

                blackMask = openedValueMask;

                if (showDebuggingOutput && showBlack) {
                    cv::imshow("black - valueMask", valueMask);
                    cv::imshow("black - valueMask anded with innerTableMask", valueMaskAndedWithTableMask);
                    cv::imshow("black - closedValueMask", closedValueMask);
                    cv::imshow("black - openedValueMask", openedValueMask);
                    cv::imshow("black - mask", blackMask);
                }

            }

            cv::Mat blackMaskedGrayscale;
            cv::bitwise_and(grayscaleInput, grayscaleInput, blackMaskedGrayscale, blackMask);

            if (showDebuggingOutput && showBlack) {

                cv::Mat blackMaskedInput;
                cv::bitwise_and(input, input, blackMaskedInput, blackMask);
                cv::Mat invMask;
                cv::bitwise_not(blackMask, invMask);
                cv::Mat maskedDelta;
                cv::bitwise_and(input, input, maskedDelta, invMask);

                cv::imshow("black - masked color", blackMaskedInput);
                cv::imshow("black - masked grayscale", blackMaskedGrayscale);
                cv::imshow("black - masked delta", maskedDelta);
            }

            // Hough on black masked input
            std::vector<cv::Vec3f> blackCircles;
            HoughCircles(blackMaskedGrayscale, blackCircles, cv::HOUGH_GRADIENT, 1,
                         houghMinDistance, // minimal distance between centers
                         houghCannyHigherThreshold, // higher threshold of the two passed to the Canny edge detector (lower threshold is twice smaller)
                         houghAccumulatorThreshold, // accumulator threshold for the circle centers (if the value is smaller, the more "circles" are detected)
                         minRadius, maxRadius
            );

            if (showDebuggingOutput && showBlack) {
                cv::Mat edges;
                cv::Canny(blackMaskedGrayscale, edges, houghCannyHigherThreshold / 2, houghCannyHigherThreshold);

                cv::imshow("black - maskedInput grayscale edges", edges);
            }

            // White & Pink Mask
            cv::Mat whitePinkMask;
            {
                cv::Mat valueMask;
                cv::inRange(value, whitePinkValueFilter.x, whitePinkValueFilter.y, valueMask);

                cv::Mat openedMask;
                cv::morphologyEx(valueMask, openedMask, cv::MORPH_OPEN, elementRect3x3, cv::Point(-1, -1), 1);

                cv::Mat closedMask;
                cv::morphologyEx(openedMask, closedMask, cv::MORPH_CLOSE, elementRect3x3, cv::Point(-1, -1), 1);

                whitePinkMask = closedMask;

                if (showDebuggingOutput && showWhitePink) {
                    cv::imshow("white&pink - valueMask", valueMask);
                    cv::imshow("white&pink - openedMask", openedMask);
                    cv::imshow("white&pink - closedMask", closedMask);
                    cv::imshow("white&pink - mask", whitePinkMask);
                }
            }

            cv::Mat whitePinkMaskedGrayscale;
            cv::bitwise_and(grayscaleInput, grayscaleInput, whitePinkMaskedGrayscale, whitePinkMask);

            cv::Mat closedWhitePinkMaskedGrayscale;
            cv::morphologyEx(whitePinkMaskedGrayscale, closedWhitePinkMaskedGrayscale, cv::MORPH_CLOSE, elementRect3x3, cv::Point(-1, -1), 2);

            if (showDebuggingOutput && showWhitePink) {
                cv::Mat whitePinkMaskedInput;
                cv::bitwise_and(input, input, whitePinkMaskedInput, whitePinkMask);

                cv::Mat invMask;
                cv::bitwise_not(whitePinkMask, invMask);
                cv::Mat maskedDelta;
                cv::bitwise_and(input, input, maskedDelta, invMask);

                cv::imshow("white&pink - masked color", whitePinkMaskedInput);
                cv::imshow("white&pink - masked grayscale", whitePinkMaskedGrayscale);
                cv::imshow("white&pink - masked and closed grayscale", closedWhitePinkMaskedGrayscale);
                cv::imshow("white&pink - masked delta", maskedDelta);
            }

            // Hough on white&pink masked input
            std::vector<cv::Vec3f> whitePinkCircles;
            HoughCircles(closedWhitePinkMaskedGrayscale, whitePinkCircles, cv::HOUGH_GRADIENT, 1,
                         houghMinDistance, // minimal distance between centers
                         houghCannyHigherThreshold, // higher threshold of the two passed to the Canny edge detector (lower threshold is twice smaller)
                         houghAccumulatorThreshold, // accumulator threshold for the circle centers (if the value is smaller, the more "circles" are detected)
                         minRadius, maxRadius
            );

            if (showDebuggingOutput && showWhitePink) {
                cv::Mat edges;
                cv::Canny(whitePinkMaskedGrayscale, edges, houghCannyHigherThreshold / 2, houghCannyHigherThreshold);

                cv::imshow("white&pink - maskedInput grayscale edges", edges);
            }

            std::vector<cv::Vec3f> allCircles;

            {
                std::vector<cv::Vec3f> circles = saturationCircles;
                std::vector<cv::Vec3f> filteredCircles1 = filterCircles(circles, innerTableMask, true);
                std::vector<cv::Vec3f> filteredCircles2 = filterCircles(filteredCircles1, saturatedBallMask, true);
                std::vector<cv::Vec3f> filteredCircles3 = filterCircles(filteredCircles2, blackMask, false);
                for (auto& circle : filteredCircles3) allCircles.push_back(circle);

                if (showDebuggingOutput && showSaturated) {
                    cv::Mat houghOnMaskedInput = input.clone();
                    cv::Mat houghOnMaskedInputAfterCircleFilter1 = input.clone();
                    cv::Mat houghOnMaskedInputAfterCircleFilter2 = input.clone();
                    cv::Mat houghOnMaskedInputAfterCircleFilter3 = input.clone();

                    drawHoughResult(houghOnMaskedInput, circles);
                    drawHoughResult(houghOnMaskedInputAfterCircleFilter1, filteredCircles1);
                    drawHoughResult(houghOnMaskedInputAfterCircleFilter2, filteredCircles2);
                    drawHoughResult(houghOnMaskedInputAfterCircleFilter3, filteredCircles3);

                    cv::imshow("saturated - hough on masked input", houghOnMaskedInput);
                    cv::imshow("saturated - hough on masked input after circle filter 1",
                               houghOnMaskedInputAfterCircleFilter1);
                    cv::imshow("saturated - hough on masked input after circle filter 2",
                               houghOnMaskedInputAfterCircleFilter2);
                    cv::imshow("saturated - hough on masked input after circle filter 3",
                               houghOnMaskedInputAfterCircleFilter3);
                }
            }

            {
                std::vector<cv::Vec3f> circles = blackCircles;
                std::vector<cv::Vec3f> filteredCircles1 = filterCircles(circles, innerTableMask, true);
                std::vector<cv::Vec3f> filteredCircles2 = filterCircles(filteredCircles1, blackMask, true);
                for (auto& circle : filteredCircles2) allCircles.push_back(circle);

                if (showDebuggingOutput && showBlack) {
                    cv::Mat houghOnMaskedInput = input.clone();
                    cv::Mat houghOnMaskedInputAfterCircleFilter1 = input.clone();
                    cv::Mat houghOnMaskedInputAfterCircleFilter2 = input.clone();

                    drawHoughResult(houghOnMaskedInput, circles);
                    drawHoughResult(houghOnMaskedInputAfterCircleFilter1, filteredCircles1);
                    drawHoughResult(houghOnMaskedInputAfterCircleFilter2, filteredCircles2);

                    cv::imshow("black - hough on masked input", houghOnMaskedInput);
                    cv::imshow("black - hough on masked input after circle filter 1",
                               houghOnMaskedInputAfterCircleFilter1);
                    cv::imshow("black - hough on masked input after circle filter 2",
                               houghOnMaskedInputAfterCircleFilter2);

                    for (auto circle : filteredCircles2) {
                        std::cout << "BLACK CIRCLE " << circle << "" << std::endl;
                    }
                }
            }

            {
                std::vector<cv::Vec3f> circles = whitePinkCircles;
                std::vector<cv::Vec3f> filteredCircles1 = filterCircles(circles, innerTableMask, true);
                std::vector<cv::Vec3f> filteredCircles2 = filterCircles(filteredCircles1, whitePinkMask, true);

                // Filter circles by saturated ball mask in order to remove 'duplicate' balls
                std::vector<cv::Vec3f> filteredCircles3 = filterCircles(filteredCircles2, saturatedBallMask, false);

                for (auto& circle : filteredCircles3) allCircles.push_back(circle);

                if (showDebuggingOutput && showWhitePink) {
                    cv::Mat houghOnMaskedInput = input.clone();
                    cv::Mat houghOnMaskedInputAfterCircleFilter1 = input.clone();
                    cv::Mat houghOnMaskedInputAfterCircleFilter2 = input.clone();
                    cv::Mat houghOnMaskedInputAfterCircleFilter3 = input.clone();

                    drawHoughResult(houghOnMaskedInput, whitePinkCircles);
                    drawHoughResult(houghOnMaskedInputAfterCircleFilter1, filteredCircles1);
                    drawHoughResult(houghOnMaskedInputAfterCircleFilter2, filteredCircles2);
                    drawHoughResult(houghOnMaskedInputAfterCircleFilter3, filteredCircles3);

                    cv::imshow("white&pink - hough on masked input", houghOnMaskedInput);
                    cv::imshow("white&pink - hough on masked input after circle filter 1",
                               houghOnMaskedInputAfterCircleFilter1);
                    cv::imshow("white&pink - hough on masked input after circle filter 2",
                               houghOnMaskedInputAfterCircleFilter2);
                    cv::imshow("white&pink - hough on masked input after circle filter 3",
                               houghOnMaskedInputAfterCircleFilter3);
                }
            }

            if (showDebuggingOutput) {
                cv::Mat hough = input.clone();
                drawHoughResult(hough, allCircles);

                if (imageIndex < expectedBallCounts.size()) {
                    int expectedBallCount = expectedBallCounts[imageIndex];
                    std::cout << "Detected " << allCircles.size() << " circles, expected " << expectedBallCount
                              << std::endl;
                    std::string text = std::to_string(allCircles.size()) + "/" + std::to_string(expectedBallCount);
                    cv::putText(hough, text, cv::Point(hough.cols / 2, 50), cv::FONT_HERSHEY_COMPLEX, 0.5,
                                cv::Scalar(0, 255, 0));
                }

                cv::imshow("hough", hough);
            }

            if (config.valid) {
                std::vector<cv::Point2d> imagePoints = circleCenters(allCircles);
                std::vector<cv::Point3d> worldPoints = billiard::detection::imagePointsToWorldPoints(config, plane,
                                                                                                     imagePoints);
                std::vector<cv::Point2d> modelPoints = billiard::detection::worldPointsToModelPoints(worldToModel,
                                                                                                     worldPoints);
                std::cout << "------------------ CIRCLES ------------------" << std::endl;
                printCoordinates("Circles", imagePoints, worldPoints, modelPoints);
            }
        }

        char key = (char) cv::waitKey(30);
        if (key == 'q' || key == 27) {
            break;
        } else if (key == 'b') {
            showBlack = !showBlack;
            cv::destroyAllWindows();
        } else if (key == 's') {
            showSaturated = !showSaturated;
            cv::destroyAllWindows();
        } else if (key == 'w') {
            showWhitePink = !showWhitePink;
            cv::destroyAllWindows();
        } else if (key == 'r') {
            showRails = !showRails;
            cv::destroyAllWindows();
        } else if (key == 'o') {
            showDebuggingOutput = !showDebuggingOutput;
            cv::destroyAllWindows();
        } else if (key == 'l') {
            imageChanged = true;
        } else if (key == 97 /* A */) {
            imageIndex = imageIndex == 0 ? imagePaths.size() - 1 : imageIndex - 1;
            imageChanged = true;
        } else if (key == 100 /* D */) {
            imageIndex = (imageIndex + 1) % (imagePaths.size());
            imageChanged = true;
        }
    }

}

void imageGallery(const std::string& name, const std::vector<cv::Mat>& images) {
    int imageIndex = 0;
    while(true) {

        char key = (char) cv::waitKey(30);
        if (key == 'q' || key == 27) break;
        if (key == 97 /* A */) imageIndex = imageIndex == 0 ? images.size() - 1 : imageIndex - 1;
        if (key == 100 /* D */) imageIndex = (imageIndex + 1) % (images.size());

        cv::imshow(name, images[imageIndex]);
    }
}

TEST(BallDetectionTests, ballDetectionByRemovingTableGreen) {
    const auto& imagePath = "./resources/test_detection/8.png";

    cv::Point2d tableHueFilter {0, 100};
    cv::Point2d tableSaturationFilter {0, 150};
    cv::Point2d tableValueFilter {0, 240};

    // Filter for saturated balls
    cv::Point2d ballsSaturationFilter {128, 255};

    // Filter for black ball
    cv::Point2d blackBallValueFilter {0, 60};

    // Filter for white ball
    cv::Point2d whiteBallHueFilter {0, 60};
    cv::Point2d whiteBallSaturationFilter {0, 200};
    cv::Point2d whiteBallValueFilter {200, 255};

    double scale = 0.5;
    double resolutionX = 2048 * scale;
    double tableLength = 2130;
    double ballDiameter = 53;
    double errorLow = 20;
    double errorHigh = 0;

    cv::Rect table_rect_original {126, 126, 1813 - 126, 972 - 126};
    cv::Rect table_rect {(int)(table_rect_original.x * scale),
                         (int)(table_rect_original.y * scale),
                         (int)(table_rect_original.width * scale),
                         (int)(table_rect_original.height * scale)};

    long double pixelsPerMillimeter = static_cast<long double>(resolutionX) / tableLength;
    uint16_t radiusInPixel = ceil((ballDiameter / 2.0) * pixelsPerMillimeter);
    uint16_t errorRadiusLow = ceil(radiusInPixel * (errorLow / 100.0));
    uint16_t errorRadiusHigh = ceil(radiusInPixel * (errorHigh / 100.0));
    uint16_t minRadius = radiusInPixel - errorRadiusLow;
    uint16_t maxRadius = radiusInPixel + errorRadiusHigh;

    // minimal distance between centers
    double houghMinDistance = minRadius * 2;
    // higher threshold of the two passed to the Canny edge detector (lower threshold is twice smaller)
    double houghCannyHigherThreshold = 60;
    // accumulator threshold for the circle centers (if the value is smaller, the more "circles" are detected)
    double houghAccumulatorThreshold = 5;

    std::cout << "radius in pixels: " << std::to_string(radiusInPixel) << " min radius: " << std::to_string(minRadius) << " max radius: " << std::to_string(maxRadius) << " error radius range: " << std::to_string(errorRadiusLow) << ", " << std::to_string(errorRadiusHigh) << std::endl;

    auto original = imread(imagePath, cv::IMREAD_COLOR);
    cv::Mat input;
    cv::resize(original, input, cv::Size(), scale, scale);
    cv::imshow("input", input);

    // TODO: try out different levels of blur
    cv::Mat blurred;
    cv::GaussianBlur(input, blurred, cv::Size(5, 5), 0, 0);
    cv::imshow("blurred", blurred);

    cv::Mat hue, saturation, value;
    hsvFromBgr(blurred, hue, saturation, value);
    cv::imshow("hue", hue);
    cv::imshow("saturation", saturation);
    cv::imshow("value", value);

    // Table mask
    cv::Mat tableMask;
    {
        cv::Mat hueTableMask;
        cv::inRange(hue, tableHueFilter.x, tableHueFilter.y, hueTableMask);

        cv::Mat saturationTableMask;
        cv::inRange(saturation, tableSaturationFilter.x, tableSaturationFilter.y, saturationTableMask);

        cv::Mat valueTableMask;
        cv::inRange(value, tableValueFilter.x, tableValueFilter.y, valueTableMask);

        cv::Mat totalTableMask;
        cv::bitwise_and(hueTableMask, saturationTableMask, totalTableMask);
        cv::bitwise_and(totalTableMask, valueTableMask, totalTableMask);

        tableMask = totalTableMask;
    }

    // Apply opening in order to close holes in black areas
    cv::Mat tableMaskOpened;
    {
        int opening_size = 1;
        cv::Mat element = getStructuringElement(cv::MORPH_RECT,
                                                cv::Size(2*opening_size + 1, 2*opening_size+1),
                                                cv::Point(opening_size, opening_size));

        cv::morphologyEx(tableMask, tableMaskOpened, cv::MORPH_OPEN, element, cv::Point(-1, -1), 1);
    }

    // Apply closing in order to remove excess black pixels
    cv::Mat tableMaskClosed;
    {
        int closing_size = 1;
        cv::Mat element = getStructuringElement(cv::MORPH_RECT,
                                                cv::Size(2*closing_size + 1, 2*closing_size+1),
                                                cv::Point(closing_size, closing_size));

        cv::morphologyEx(tableMaskOpened, tableMaskClosed, cv::MORPH_CLOSE, element, cv::Point(-1, -1), 1);
    }

    cv::imshow("tableMask", tableMask);
    cv::imshow("tableMaskOpened", tableMaskOpened);
    cv::imshow("tableMaskClosed", tableMaskClosed);

    cv::Mat tableMaskInverted;
    cv::bitwise_not(tableMask, tableMaskInverted);

    cv::Mat backgroundRemoved;
    cv::bitwise_and(blurred, blurred, backgroundRemoved, tableMaskInverted);
    cv::imshow("backgroundRemoved", backgroundRemoved);

    cv::Mat table;
    cv::bitwise_and(blurred, blurred, table, tableMask);

    cv::Mat nonTable;
    cv::bitwise_and(blurred, blurred, nonTable, tableMaskInverted);

    cv::imshow("table", table);
    cv::imshow("nonTable", nonTable);

    cv::Mat houghOnBackgroundRemovedInput;
    {
        cv::Mat grayscale;
        cv::cvtColor(backgroundRemoved, grayscale, cv::COLOR_BGR2GRAY);

        std::vector<cv::Vec3f> circles;
        HoughCircles(grayscale, circles, cv::HOUGH_GRADIENT, 1,
                     houghMinDistance, // minimal distance between centers
                     houghCannyHigherThreshold, // higher threshold of the two passed to the Canny edge detector (lower threshold is twice smaller)
                     houghAccumulatorThreshold, // accumulator threshold for the circle centers (if the value is smaller, the more "circles" are detected)
                     minRadius, maxRadius
        );
        std::vector<cv::Vec3f> filteredCircles = filterCircles(circles, table_rect);

        input.copyTo(houghOnBackgroundRemovedInput);
        drawHoughResult(houghOnBackgroundRemovedInput, filteredCircles);
    }
    cv::imshow("houghOnBackgroundRemovedInput", houghOnBackgroundRemovedInput);

    cv::waitKey();
}

TEST(BallDetectionTests, redMask) {

    std::vector<std::string> imagePaths = {
            "./resources/test_detection/1.png",
            "./resources/test_detection/2.png",
            "./resources/test_detection/3.png",
            "./resources/test_detection/4.png",
            "./resources/test_detection/5.png",
            "./resources/test_detection/6.png",
            "./resources/test_detection/7.png",
            "./resources/test_detection/8.png",
    };

    double scale = 0.5;
    double resolutionX = 2048 * scale;
    double tableLength = 2130;
    double ballDiameter = 53;
    double errorLow = 20;
    double errorHigh = 0;

    cv::Rect table_rect_original {126, 126, 1813 - 126, 972 - 126};
    cv::Rect table_rect {(int)(table_rect_original.x * scale),
                         (int)(table_rect_original.y * scale),
                         (int)(table_rect_original.width * scale),
                         (int)(table_rect_original.height * scale)};

    long double pixelsPerMillimeter = static_cast<long double>(resolutionX) / tableLength;
    uint16_t radiusInPixel = ceil((ballDiameter / 2.0) * pixelsPerMillimeter);
    uint16_t errorRadiusLow = ceil(radiusInPixel * (errorLow / 100.0));
    uint16_t errorRadiusHigh = ceil(radiusInPixel * (errorHigh / 100.0));
    uint16_t minRadius = radiusInPixel - errorRadiusLow;
    uint16_t maxRadius = radiusInPixel + errorRadiusHigh;

    // minimal distance between centers
    double houghMinDistance = minRadius * 2;
    // higher threshold of the two passed to the Canny edge detector (lower threshold is twice smaller)
    double houghCannyHigherThreshold = 60;
    // accumulator threshold for the circle centers (if the value is smaller, the more "circles" are detected)
    double houghAccumulatorThreshold = 5;

    std::cout << "radius in pixels: " << std::to_string(radiusInPixel) << " min radius: " << std::to_string(minRadius) << " max radius: " << std::to_string(maxRadius) << " error radius range: " << std::to_string(errorRadiusLow) << ", " << std::to_string(errorRadiusHigh) << std::endl;

    unsigned long long imageIndex = 0;
    bool imageChanged = true;

    cv::Mat elementRect3x3 = getStructuringElement(cv::MORPH_RECT, cv::Size(3,3), cv::Point(1, 1));

    while(true) {
        std::string imagePath = imagePaths[imageIndex];

        auto original = imread(imagePath, cv::IMREAD_COLOR);
        cv::Mat input;
        cv::resize(original, input, cv::Size(), scale, scale);

        cv::imshow("input", input);

        // TODO: try out different levels of blur
        cv::Mat blurred;
        cv::GaussianBlur(input, blurred, cv::Size(5, 5), 0, 0);
        cv::imshow("blurred", blurred);

        cv::Mat hue, saturation, value;
        hsvFromBgr(blurred, hue, saturation, value);
        cv::imshow("hue", hue);
        cv::imshow("saturation", saturation);
        cv::imshow("value", value);

        cv::Mat redMask;
        {
            cv::Point2d redHueFilter1 {0, 10};
            cv::Point2d redHueFilter2 {170, 180};

            cv::Mat redMask1;
            cv::inRange(hue, redHueFilter1.x, redHueFilter1.y, redMask1);
            cv::imshow("redMask1", redMask1);

            cv::Mat redMask2;
            cv::inRange(hue, redHueFilter2.x, redHueFilter2.y, redMask2);
            cv::imshow("redMask2", redMask2);

            cv::Mat totalMask;
            cv::bitwise_or(redMask1, redMask2, totalMask);

            cv::Mat openedMask;
            cv::morphologyEx(totalMask, openedMask, cv::MORPH_OPEN, elementRect3x3, cv::Point(-1, -1), 3);
            cv::imshow("openedMask", openedMask);

            cv::Mat closedMask;
            cv::morphologyEx(openedMask, closedMask, cv::MORPH_CLOSE, elementRect3x3, cv::Point(-1, -1), 3);
            cv::imshow("closedMask", closedMask);

            cv::Mat redSaturation;
            cv::Point2d saturationFilter {100, 255};
            cv::inRange(saturation, saturationFilter.x, saturationFilter.y, redSaturation);
            cv::imshow("redSaturation", redSaturation);

            cv::Mat redMaskAndedWithRedSaturation;
            cv::bitwise_and(closedMask, redSaturation, redMaskAndedWithRedSaturation);
            cv::imshow("redMaskAndedWithRedSaturation", redMaskAndedWithRedSaturation);

            cv::Mat redValue;
            cv::Point2d valueFilter {215, 255};
            cv::inRange(value, valueFilter.x, valueFilter.y, redValue);
            cv::imshow("redValue", redValue);

            cv::Mat redMaskAndedWithRedValue;
            cv::bitwise_and(closedMask, redValue, redMaskAndedWithRedValue);
            cv::imshow("redMaskAndedWithRedValue", redMaskAndedWithRedValue);

            redMask = redMaskAndedWithRedValue;
        }
        cv::imshow("redMask", redMask);

        cv::Mat redMaskedInput;
        cv::bitwise_and(input, input, redMaskedInput, redMask);
        cv::imshow("redMaskedInput", redMaskedInput);

        cv::Mat invRedMask;
        cv::bitwise_not(redMask, invRedMask);
        cv::Mat redMaskedDelta;
        cv::bitwise_and(input, input, redMaskedDelta, invRedMask);
        cv::imshow("redMaskedDelta", redMaskedDelta);

        cv::Mat houghOnRedMaskedInput;
        cv::Mat houghOnMaskedInputAfterCircleFilter1;
        cv::Mat houghOnMaskedInputAfterCircleFilter2;
        {
            cv::Mat grayscale;
            cv::cvtColor(redMaskedInput, grayscale, cv::COLOR_BGR2GRAY);

            cv::imshow("redMaskedInput grayscale", grayscale);

            cv::Mat opened;
            cv::morphologyEx(grayscale, opened, cv::MORPH_OPEN, elementRect3x3, cv::Point(-1, -1), 1);
            cv::imshow("redMaskedInput grayscale opened", opened);

            cv::Mat edges;
            cv::Canny(opened, edges, houghCannyHigherThreshold / 2, houghCannyHigherThreshold);

            cv::imshow("redMaskedInput grayscale edges", edges);

            std::vector<cv::Vec3f> circles;
            HoughCircles(grayscale, circles, cv::HOUGH_GRADIENT, 1,
                         houghMinDistance, // minimal distance between centers
                         houghCannyHigherThreshold, // higher threshold of the two passed to the Canny edge detector (lower threshold is twice smaller)
                         houghAccumulatorThreshold, // accumulator threshold for the circle centers (if the value is smaller, the more "circles" are detected)
                         minRadius, maxRadius
            );
            std::vector<cv::Vec3f> filteredCircles1 = filterCircles(circles, table_rect);
            std::vector<cv::Vec3f> filteredCircles2 = filterCircles(filteredCircles1, redMask, true);

            input.copyTo(houghOnRedMaskedInput);
            drawHoughResult(houghOnRedMaskedInput, circles);

            input.copyTo(houghOnMaskedInputAfterCircleFilter1);
            drawHoughResult(houghOnMaskedInputAfterCircleFilter1, filteredCircles1);

            input.copyTo(houghOnMaskedInputAfterCircleFilter2);
            drawHoughResult(houghOnMaskedInputAfterCircleFilter2, filteredCircles2);
        }
        cv::imshow("hough on masked input", houghOnRedMaskedInput);
        cv::imshow("hough on masked input after circle filter 1", houghOnMaskedInputAfterCircleFilter1);
        cv::imshow("hough on masked input after circle filter 2", houghOnMaskedInputAfterCircleFilter2);

        char key = (char) cv::waitKey(30);
        if (key == 'q' || key == 27) {
            break;
        }
        if (key == 97 /* A */) {
            imageIndex = imageIndex == 0 ? imagePaths.size() - 1 : imageIndex - 1;
            imageChanged = true;
        }
        if (key == 100 /* D */) {
            imageIndex = (imageIndex + 1) % (imagePaths.size());
            imageChanged = true;
        }
    }
}

TEST(BallDetectionTests, blueGreenYellowMask) {

    std::vector<std::string> imagePaths = {
            "./resources/test_detection/1.png",
            "./resources/test_detection/2.png",
            "./resources/test_detection/3.png",
            "./resources/test_detection/4.png",
            "./resources/test_detection/5.png",
            "./resources/test_detection/6.png",
            "./resources/test_detection/7.png",
            "./resources/test_detection/8.png",
    };

    double scale = 0.5;
    double resolutionX = 2048 * scale;
    double tableLength = 2130;
    double ballDiameter = 53;
    double errorLow = 20;
    double errorHigh = 0;

    cv::Rect table_rect_original {126, 126, 1813 - 126, 972 - 126};
    cv::Rect table_rect {(int)(table_rect_original.x * scale),
                         (int)(table_rect_original.y * scale),
                         (int)(table_rect_original.width * scale),
                         (int)(table_rect_original.height * scale)};

    long double pixelsPerMillimeter = static_cast<long double>(resolutionX) / tableLength;
    uint16_t radiusInPixel = ceil((ballDiameter / 2.0) * pixelsPerMillimeter);
    uint16_t errorRadiusLow = ceil(radiusInPixel * (errorLow / 100.0));
    uint16_t errorRadiusHigh = ceil(radiusInPixel * (errorHigh / 100.0));
    uint16_t minRadius = radiusInPixel - errorRadiusLow;
    uint16_t maxRadius = radiusInPixel + errorRadiusHigh;

    // minimal distance between centers
    double houghMinDistance = minRadius * 2;
    // higher threshold of the two passed to the Canny edge detector (lower threshold is twice smaller)
    double houghCannyHigherThreshold = 60;
    // accumulator threshold for the circle centers (if the value is smaller, the more "circles" are detected)
    double houghAccumulatorThreshold = 5;

    std::cout << "radius in pixels: " << std::to_string(radiusInPixel) << " min radius: " << std::to_string(minRadius) << " max radius: " << std::to_string(maxRadius) << " error radius range: " << std::to_string(errorRadiusLow) << ", " << std::to_string(errorRadiusHigh) << std::endl;

    unsigned long long imageIndex = 0;
    bool imageChanged = true;

    cv::Mat elementRect3x3 = getStructuringElement(cv::MORPH_RECT, cv::Size(3,3), cv::Point(1, 1));

    while(true) {
        std::string imagePath = imagePaths[imageIndex];

        auto original = imread(imagePath, cv::IMREAD_COLOR);
        cv::Mat input;
        cv::resize(original, input, cv::Size(), scale, scale);

        // TODO: try out different levels of blur
        cv::Mat blurred;
        cv::GaussianBlur(input, blurred, cv::Size(5, 5), 0, 0);

        cv::Mat hue, saturation, value;
        hsvFromBgr(blurred, hue, saturation, value);

        cv::imshow("input", input);
        cv::imshow("blurred", blurred);
        cv::imshow("hue", hue);
        cv::imshow("saturation", saturation);
        cv::imshow("value", value);

        cv::Mat ballMask;
        {
            cv::Point2d saturationFilter {100, 255};

            cv::Mat saturationMask;
            cv::inRange(saturation, saturationFilter.x, saturationFilter.y, saturationMask);

            cv::Mat openedMask;
            cv::morphologyEx(saturationMask, openedMask, cv::MORPH_OPEN, elementRect3x3, cv::Point(-1, -1), 1);

            cv::Mat closedMask;
            cv::morphologyEx(openedMask, closedMask, cv::MORPH_CLOSE, elementRect3x3, cv::Point(-1, -1), 2);

            ballMask = closedMask;

            cv::imshow("saturationMask", saturationMask);
            cv::imshow("openedMask", openedMask);
            cv::imshow("closedMask", closedMask);
        }
        cv::imshow("ballMask", ballMask);

        cv::Mat maskedInput;
        cv::bitwise_and(input, input, maskedInput, ballMask);
        cv::imshow("maskedInput", maskedInput);

        cv::Mat invMask;
        cv::bitwise_not(ballMask, invMask);
        cv::Mat maskedDelta;
        cv::bitwise_and(input, input, maskedDelta, invMask);
        cv::imshow("maskedDelta", maskedDelta);

        cv::Mat houghOnMaskedInput;
        cv::Mat houghOnMaskedInputAfterCircleFilter1;
        cv::Mat houghOnMaskedInputAfterCircleFilter2;
        {
            cv::Mat grayscale;
            cv::cvtColor(maskedInput, grayscale, cv::COLOR_BGR2GRAY);

            cv::imshow("maskedInput grayscale", grayscale);

            cv::Mat opened;
            cv::morphologyEx(grayscale, opened, cv::MORPH_OPEN, elementRect3x3, cv::Point(-1, -1), 1);
            cv::imshow("maskedInput grayscale opened", opened);

            cv::Mat edges;
            cv::Canny(opened, edges, houghCannyHigherThreshold / 2, houghCannyHigherThreshold);

            cv::imshow("maskedInput grayscale edges", edges);

            std::vector<cv::Vec3f> circles;
            HoughCircles(grayscale, circles, cv::HOUGH_GRADIENT, 1,
                         houghMinDistance, // minimal distance between centers
                         houghCannyHigherThreshold, // higher threshold of the two passed to the Canny edge detector (lower threshold is twice smaller)
                         houghAccumulatorThreshold, // accumulator threshold for the circle centers (if the value is smaller, the more "circles" are detected)
                         minRadius, maxRadius
            );
            std::vector<cv::Vec3f> filteredCircles1 = filterCircles(circles, table_rect);
            std::vector<cv::Vec3f> filteredCircles2 = filterCircles(filteredCircles1, ballMask, true);

            input.copyTo(houghOnMaskedInput);
            drawHoughResult(houghOnMaskedInput, circles);

            input.copyTo(houghOnMaskedInputAfterCircleFilter1);
            drawHoughResult(houghOnMaskedInputAfterCircleFilter1, filteredCircles1);

            input.copyTo(houghOnMaskedInputAfterCircleFilter2);
            drawHoughResult(houghOnMaskedInputAfterCircleFilter2, filteredCircles2);
        }
        cv::imshow("hough on masked input", houghOnMaskedInput);
        cv::imshow("hough on masked input after circle filter 1", houghOnMaskedInputAfterCircleFilter1);
        cv::imshow("hough on masked input after circle filter 2", houghOnMaskedInputAfterCircleFilter2);

        char key = (char) cv::waitKey(30);
        if (key == 'q' || key == 27) {
            break;
        }
        if (key == 97 /* A */) {
            imageIndex = imageIndex == 0 ? imagePaths.size() - 1 : imageIndex - 1;
            imageChanged = true;
        }
        if (key == 100 /* D */) {
            imageIndex = (imageIndex + 1) % (imagePaths.size());
            imageChanged = true;
        }
    }

}

TEST(BallDetectionTests, blackMask) {

    std::vector<std::string> imagePaths = {
            "./resources/test_detection/1.png",
            "./resources/test_detection/2.png",
            "./resources/test_detection/3.png",
            "./resources/test_detection/4.png",
            "./resources/test_detection/5.png",
            "./resources/test_detection/6.png",
            "./resources/test_detection/7.png",
            "./resources/test_detection/8.png",
    };

    double scale = 0.5;
    double resolutionX = 2048 * scale;
    double tableLength = 2130;
    double ballDiameter = 53;
    double errorLow = 20;
    double errorHigh = 0;

    cv::Rect table_rect_original {126, 126, 1813 - 126, 972 - 126};
    cv::Rect table_rect {(int)(table_rect_original.x * scale),
                         (int)(table_rect_original.y * scale),
                         (int)(table_rect_original.width * scale),
                         (int)(table_rect_original.height * scale)};

    long double pixelsPerMillimeter = static_cast<long double>(resolutionX) / tableLength;
    uint16_t radiusInPixel = ceil((ballDiameter / 2.0) * pixelsPerMillimeter);
    uint16_t errorRadiusLow = ceil(radiusInPixel * (errorLow / 100.0));
    uint16_t errorRadiusHigh = ceil(radiusInPixel * (errorHigh / 100.0));
    uint16_t minRadius = radiusInPixel - errorRadiusLow;
    uint16_t maxRadius = radiusInPixel + errorRadiusHigh;

    // minimal distance between centers
    double houghMinDistance = minRadius * 2;
    // higher threshold of the two passed to the Canny edge detector (lower threshold is twice smaller)
    double houghCannyHigherThreshold = 60;
    // accumulator threshold for the circle centers (if the value is smaller, the more "circles" are detected)
    double houghAccumulatorThreshold = 5;

    std::cout << "radius in pixels: " << std::to_string(radiusInPixel) << " min radius: " << std::to_string(minRadius) << " max radius: " << std::to_string(maxRadius) << " error radius range: " << std::to_string(errorRadiusLow) << ", " << std::to_string(errorRadiusHigh) << std::endl;

    unsigned long long imageIndex = 0;
    bool imageChanged = true;

    cv::Mat elementRect3x3 = getStructuringElement(cv::MORPH_RECT, cv::Size(3,3), cv::Point(1, 1));

    while(true) {
        std::string imagePath = imagePaths[imageIndex];

        auto original = imread(imagePath, cv::IMREAD_COLOR);
        cv::Mat input;
        cv::resize(original, input, cv::Size(), scale, scale);

        // TODO: try out different levels of blur
        cv::Mat blurred;
        cv::GaussianBlur(input, blurred, cv::Size(5, 5), 0, 0);

        cv::Mat hue, saturation, value;
        hsvFromBgr(blurred, hue, saturation, value);

        cv::imshow("input", input);
        cv::imshow("blurred", blurred);
        cv::imshow("hue", hue);
        cv::imshow("saturation", saturation);
        cv::imshow("value", value);

        cv::Mat blackMask;
        {
            cv::Point2d valueFilter {0, 100};
            cv::Point2d saturationFilter {0, 160};

            cv::Point2d greenHueFilter {30, 90};
            cv::Mat greenMask;
            cv::inRange(hue, greenHueFilter.x, greenHueFilter.y, greenMask);

            cv::Mat nonGreenMask;
            cv::bitwise_not(greenMask, nonGreenMask);

            cv::Mat openedNonGreenMask;
            cv::morphologyEx(nonGreenMask, openedNonGreenMask, cv::MORPH_OPEN, elementRect3x3, cv::Point(-1, -1), 1);

            cv::Mat closedNonGreenMask;
            cv::morphologyEx(openedNonGreenMask, closedNonGreenMask, cv::MORPH_CLOSE, elementRect3x3, cv::Point(-1, -1), 2);

            cv::Mat valueMask;
            cv::inRange(value, valueFilter.x, valueFilter.y, valueMask);

            cv::Mat openedValueMask;
            cv::morphologyEx(valueMask, openedValueMask, cv::MORPH_OPEN, elementRect3x3, cv::Point(-1, -1), 2);

            cv::Mat closedValueMask;
            cv::morphologyEx(openedValueMask, closedValueMask, cv::MORPH_CLOSE, elementRect3x3, cv::Point(-1, -1), 2);

            cv::Mat saturationMask;
            cv::inRange(saturation, saturationFilter.x, saturationFilter.y, saturationMask);

            cv::Mat openedSaturationMask;
            cv::morphologyEx(saturationMask, openedSaturationMask, cv::MORPH_OPEN, elementRect3x3, cv::Point(-1, -1), 2);

            cv::Mat nonGreenLowSaturationMask;
            cv::bitwise_and(closedNonGreenMask, saturationMask, nonGreenLowSaturationMask);

            cv::Mat nonGreenAndedWithValueMask;
            cv::bitwise_and(closedNonGreenMask, closedValueMask, nonGreenAndedWithValueMask);

            cv::Mat totalMask;
            cv::bitwise_and(closedValueMask, openedSaturationMask, totalMask);

            cv::Mat openedMask;
            cv::morphologyEx(totalMask, openedMask, cv::MORPH_OPEN, elementRect3x3, cv::Point(-1, -1), 2);

            cv::Mat closedMask;
            cv::morphologyEx(openedMask, closedMask, cv::MORPH_CLOSE, elementRect3x3, cv::Point(-1, -1), 2);

            blackMask = closedMask;

            cv::imshow("greenMask", greenMask);
            cv::imshow("nonGreenMask", nonGreenMask);
            cv::imshow("openedNonGreenMask", openedNonGreenMask);
            cv::imshow("closedNonGreenMask", closedNonGreenMask);
            cv::imshow("nonGreenLowSaturationMask", nonGreenLowSaturationMask);
            cv::imshow("nonGreenAndedWithValueMask", nonGreenAndedWithValueMask);
            cv::imshow("valueMask", valueMask);
            cv::imshow("openedValueMask", openedValueMask);
            cv::imshow("closedValueMask", closedValueMask);
            cv::imshow("saturationMask", saturationMask);
            cv::imshow("openedSaturationMask", openedSaturationMask);
            cv::imshow("totalMask", totalMask);
            cv::imshow("openedMask", openedMask);
            cv::imshow("closedMask", closedMask);
        }
        cv::imshow("blackMask", blackMask);

        cv::Mat maskedInput;
        cv::bitwise_and(input, input, maskedInput, blackMask);
        cv::imshow("maskedInput", maskedInput);

        cv::Mat invMask;
        cv::bitwise_not(blackMask, invMask);
        cv::Mat maskedDelta;
        cv::bitwise_and(input, input, maskedDelta, invMask);
        cv::imshow("maskedDelta", maskedDelta);

        cv::Mat houghOnMaskedInput;
        cv::Mat houghOnMaskedInputAfterCircleFilter1;
        cv::Mat houghOnMaskedInputAfterCircleFilter2;
        {
            cv::Mat grayscale;
            cv::cvtColor(maskedInput, grayscale, cv::COLOR_BGR2GRAY);

            cv::imshow("maskedInput grayscale", grayscale);

            cv::Mat opened;
            cv::morphologyEx(grayscale, opened, cv::MORPH_OPEN, elementRect3x3, cv::Point(-1, -1), 1);
            cv::imshow("maskedInput grayscale opened", opened);

            cv::Mat edges;
            cv::Canny(opened, edges, houghCannyHigherThreshold / 2, houghCannyHigherThreshold);

            cv::imshow("maskedInput grayscale edges", edges);

            std::vector<cv::Vec3f> circles;
            HoughCircles(grayscale, circles, cv::HOUGH_GRADIENT, 1,
                         houghMinDistance, // minimal distance between centers
                         houghCannyHigherThreshold, // higher threshold of the two passed to the Canny edge detector (lower threshold is twice smaller)
                         houghAccumulatorThreshold, // accumulator threshold for the circle centers (if the value is smaller, the more "circles" are detected)
                         minRadius, maxRadius
            );
            std::vector<cv::Vec3f> filteredCircles1 = filterCircles(circles, table_rect);
            std::vector<cv::Vec3f> filteredCircles2 = filterCircles(filteredCircles1, blackMask, true);

            input.copyTo(houghOnMaskedInput);
            drawHoughResult(houghOnMaskedInput, circles);

            input.copyTo(houghOnMaskedInputAfterCircleFilter1);
            drawHoughResult(houghOnMaskedInputAfterCircleFilter1, filteredCircles1);

            input.copyTo(houghOnMaskedInputAfterCircleFilter2);
            drawHoughResult(houghOnMaskedInputAfterCircleFilter2, filteredCircles2);
        }
        cv::imshow("hough on masked input", houghOnMaskedInput);
        cv::imshow("hough on masked input after circle filter 1", houghOnMaskedInputAfterCircleFilter1);
        cv::imshow("hough on masked input after circle filter 2", houghOnMaskedInputAfterCircleFilter2);

        char key = (char) cv::waitKey(30);
        if (key == 'q' || key == 27) {
            break;
        }
        if (key == 97 /* A */) {
            imageIndex = imageIndex == 0 ? imagePaths.size() - 1 : imageIndex - 1;
            imageChanged = true;
        }
        if (key == 100 /* D */) {
            imageIndex = (imageIndex + 1) % (imagePaths.size());
            imageChanged = true;
        }
    }
}

TEST(BallDetectionTests, whitePinkMask) {

    std::vector<std::string> imagePaths = {
            "./resources/test_detection/1.png",
            "./resources/test_detection/2.png",
            "./resources/test_detection/3.png",
            "./resources/test_detection/4.png",
            "./resources/test_detection/5.png",
            "./resources/test_detection/6.png",
            "./resources/test_detection/7.png",
            "./resources/test_detection/8.png",
    };

    double scale = 0.5;
    double resolutionX = 2048 * scale;
    double tableLength = 2130;
    double ballDiameter = 53;
    double errorLow = 20;
    double errorHigh = 0;

    cv::Rect table_rect_original {126, 126, 1813 - 126, 972 - 126};
    cv::Rect table_rect {(int)(table_rect_original.x * scale),
                         (int)(table_rect_original.y * scale),
                         (int)(table_rect_original.width * scale),
                         (int)(table_rect_original.height * scale)};

    long double pixelsPerMillimeter = static_cast<long double>(resolutionX) / tableLength;
    uint16_t radiusInPixel = ceil((ballDiameter / 2.0) * pixelsPerMillimeter);
    uint16_t errorRadiusLow = ceil(radiusInPixel * (errorLow / 100.0));
    uint16_t errorRadiusHigh = ceil(radiusInPixel * (errorHigh / 100.0));
    uint16_t minRadius = radiusInPixel - errorRadiusLow;
    uint16_t maxRadius = radiusInPixel + errorRadiusHigh;

    // minimal distance between centers
    double houghMinDistance = minRadius * 2;
    // higher threshold of the two passed to the Canny edge detector (lower threshold is twice smaller)
    double houghCannyHigherThreshold = 60;
    // accumulator threshold for the circle centers (if the value is smaller, the more "circles" are detected)
    double houghAccumulatorThreshold = 5;

    std::cout << "radius in pixels: " << std::to_string(radiusInPixel) << " min radius: " << std::to_string(minRadius) << " max radius: " << std::to_string(maxRadius) << " error radius range: " << std::to_string(errorRadiusLow) << ", " << std::to_string(errorRadiusHigh) << std::endl;

    unsigned long long imageIndex = 0;
    bool imageChanged = true;

    cv::Mat elementRect3x3 = getStructuringElement(cv::MORPH_RECT, cv::Size(3,3), cv::Point(1, 1));

    while(true) {
        std::string imagePath = imagePaths[imageIndex];

        auto original = imread(imagePath, cv::IMREAD_COLOR);
        cv::Mat input;
        cv::resize(original, input, cv::Size(), scale, scale);

        // TODO: try out different levels of blur
        cv::Mat blurred;
        cv::GaussianBlur(input, blurred, cv::Size(5, 5), 0, 0);

        cv::Mat hue, saturation, value;
        hsvFromBgr(blurred, hue, saturation, value);

        cv::imshow("input", input);
        cv::imshow("blurred", blurred);
        cv::imshow("hue", hue);
        cv::imshow("saturation", saturation);
        cv::imshow("value", value);

        cv::Mat whitePinkMask;
        {
            cv::Point2d valueFilter {200, 255};

            cv::Mat valueMask;
            cv::inRange(value, valueFilter.x, valueFilter.y, valueMask);

            cv::Mat openedMask;
            cv::morphologyEx(valueMask, openedMask, cv::MORPH_OPEN, elementRect3x3, cv::Point(-1, -1), 1);

            cv::Mat closedMask;
            cv::morphologyEx(openedMask, closedMask, cv::MORPH_CLOSE, elementRect3x3, cv::Point(-1, -1), 1);

            whitePinkMask = closedMask;

            cv::imshow("valueMask", valueMask);
            cv::imshow("openedMask", openedMask);
            cv::imshow("closedMask", closedMask);
            cv::imshow("whitePinkMask", whitePinkMask);
        }

        cv::Mat maskedInput;
        {
            cv::bitwise_and(input, input, maskedInput, whitePinkMask);

            cv::Mat invMask;
            cv::bitwise_not(whitePinkMask, invMask);
            cv::Mat maskedDelta;
            cv::bitwise_and(input, input, maskedDelta, invMask);

            cv::imshow("maskedInput", maskedInput);
            cv::imshow("maskedDelta", maskedDelta);
        }

        std::vector<cv::Vec3f> whitePinkCircles;
        {
            cv::Mat grayscale;
            cv::cvtColor(maskedInput, grayscale, cv::COLOR_BGR2GRAY);

            cv::Mat opened;
            cv::morphologyEx(grayscale, opened, cv::MORPH_OPEN, elementRect3x3, cv::Point(-1, -1), 1);

            cv::Mat edges;
            cv::Canny(opened, edges, houghCannyHigherThreshold / 2, houghCannyHigherThreshold);

            HoughCircles(grayscale, whitePinkCircles, cv::HOUGH_GRADIENT, 1,
                         houghMinDistance, // minimal distance between centers
                         houghCannyHigherThreshold, // higher threshold of the two passed to the Canny edge detector (lower threshold is twice smaller)
                         houghAccumulatorThreshold, // accumulator threshold for the circle centers (if the value is smaller, the more "circles" are detected)
                         minRadius, maxRadius
            );

            cv::imshow("maskedInput grayscale", grayscale);
            cv::imshow("maskedInput grayscale opened", opened);
            cv::imshow("maskedInput grayscale edges", edges);
        }

        {
            std::vector<cv::Vec3f> circles = whitePinkCircles;
            std::vector<cv::Vec3f> filteredCircles1 = filterCircles(circles, table_rect);
            std::vector<cv::Vec3f> filteredCircles2 = filterCircles(filteredCircles1, whitePinkMask, true);

            cv::Mat houghOnMaskedInput = input.clone();
            cv::Mat houghOnMaskedInputAfterCircleFilter1 = input.clone();
            cv::Mat houghOnMaskedInputAfterCircleFilter2 = input.clone();

            drawHoughResult(houghOnMaskedInput, whitePinkCircles);
            drawHoughResult(houghOnMaskedInputAfterCircleFilter1, filteredCircles1);
            drawHoughResult(houghOnMaskedInputAfterCircleFilter2, filteredCircles2);

            cv::imshow("hough on masked input", houghOnMaskedInput);
            cv::imshow("hough on masked input after circle filter 1", houghOnMaskedInputAfterCircleFilter1);
            cv::imshow("hough on masked input after circle filter 2", houghOnMaskedInputAfterCircleFilter2);
        }

        char key = (char) cv::waitKey(30);
        if (key == 'q' || key == 27) {
            break;
        }
        if (key == 97 /* A */) {
            imageIndex = imageIndex == 0 ? imagePaths.size() - 1 : imageIndex - 1;
            imageChanged = true;
        }
        if (key == 100 /* D */) {
            imageIndex = (imageIndex + 1) % (imagePaths.size());
            imageChanged = true;
        }
    }

}

TEST(BallDetectionTest, houghOnSaturation) {

    std::vector<std::string> paths {
            "./resources/test_detection/1.png",
            "./resources/test_detection/2.png",
            "./resources/test_detection/3.png",
            "./resources/test_detection/4.png",
            "./resources/test_detection/5.png",
            "./resources/test_detection/6.png",
            "./resources/test_detection/7.png",
            "./resources/test_detection/8.png",
    };

    double scale = 0.5;
    double resolutionX = 2048 * scale;
    double tableLength = 2130;
    double ballDiameter = 53;
    double error = 30;

    long double pixelsPerMillimeter = static_cast<long double>(resolutionX) / tableLength;
    uint16_t radiusInPixel = ceil((ballDiameter / 2.0) * pixelsPerMillimeter);
    uint16_t errorRadiusRange = ceil(radiusInPixel * (error / 100.0));
    uint16_t minRadius = radiusInPixel - errorRadiusRange;
    uint16_t maxRadius = radiusInPixel + errorRadiusRange;

    // minimal distance between centers
//    double houghMinDistance = minRadius * 2;
    double houghMinDistance = minRadius;
    // higher threshold of the two passed to the Canny edge detector (lower threshold is twice smaller)
    double houghCannyHigherThreshold = 60;
    // accumulator threshold for the circle centers (if the value is smaller, the more "circles" are detected)
    double houghAccumulatorThreshold = 15;

    std::cout << "radius in pixels: " << std::to_string(radiusInPixel) << " min radius: " << std::to_string(minRadius) << " max radius: " << std::to_string(maxRadius) << " error radius range: " << std::to_string(errorRadiusRange) << std::endl;

    for (auto& imagePath : paths) {

        auto original = imread(imagePath, cv::IMREAD_COLOR);
        cv::Mat resized;
        cv::resize(original, resized, cv::Size(), scale, scale);

        cv::Mat blurred;
        cv::GaussianBlur(resized, blurred, cv::Size(5, 5), 0, 0);

        cv::Mat input = blurred;

        cv::Mat hue, saturation, value;
        hsvFromBgr(input, hue, saturation, value);

        std::stringstream saturationTitle;
        saturationTitle << "saturation " << imagePath;
        cv::imshow(saturationTitle.str(), saturation);

        cv::Point2d tableHueFilter {45, 100};
        cv::Point2d tableSaturationFilter {0, 180};
        cv::Point2d tableValueFilter {0, 200};

        cv::Mat tableMask;
        {
            cv::Mat hueTableMask;
            cv::inRange(hue, tableHueFilter.x, tableHueFilter.y, hueTableMask);

            cv::Mat saturationTableMask;
            cv::inRange(saturation, tableSaturationFilter.x, tableSaturationFilter.y, saturationTableMask);

            cv::Mat valueTableMask;
            cv::inRange(value, tableValueFilter.x, tableValueFilter.y, valueTableMask);

            cv::Mat totalTableMask;
            cv::bitwise_and(hueTableMask, saturationTableMask, totalTableMask);
            cv::bitwise_and(totalTableMask, valueTableMask, totalTableMask);

            tableMask = totalTableMask;
        }

        cv::Mat maskedImage;
        cv::bitwise_and(input, input, maskedImage, tableMask);

        std::stringstream tableMaskTitle;
        tableMaskTitle << "tableMask " << imagePath;
        cv::imshow(tableMaskTitle.str(), maskedImage);

        cv::Mat nonTableMask;
        cv::bitwise_not(tableMask, nonTableMask);

        cv::Mat maskedSaturation;
        cv::bitwise_and(saturation, saturation, maskedSaturation, nonTableMask);

        cv::Mat houghOnSaturation;
        {
            std::vector<cv::Vec3f> circles;
            HoughCircles(maskedSaturation, circles, cv::HOUGH_GRADIENT, 1,
                         houghMinDistance, // minimal distance between centers
                         houghCannyHigherThreshold, // higher threshold of the two passed to the Canny edge detector (lower threshold is twice smaller)
                         houghAccumulatorThreshold, // accumulator threshold for the circle centers (if the value is smaller, the more "circles" are detected)
                         minRadius, maxRadius
            );

            input.copyTo(houghOnSaturation);
            drawHoughResult(houghOnSaturation, circles);
        }
        std::stringstream ss;
        ss << "houghOnMaskedSaturation " << imagePath;
        cv::imshow(ss.str(), houghOnSaturation);
    }

    cv::waitKey();
}

TEST(BallDetectionTests, houghOnHueLabels) {
    const auto& imagePath = "./resources/test_detection/8.png";

    double scale = 0.5;
    double resolutionX = 2048 * scale;
    double tableLength = 2130;
    double ballDiameter = 53;
    double errorLow = 20;
    double errorHigh = 0;

    cv::Rect table_rect_original {126, 126, 1813 - 126, 972 - 126};
    cv::Rect table_rect {(int)(table_rect_original.x * scale),
                         (int)(table_rect_original.y * scale),
                         (int)(table_rect_original.width * scale),
                         (int)(table_rect_original.height * scale)};

    long double pixelsPerMillimeter = static_cast<long double>(resolutionX) / tableLength;
    uint16_t radiusInPixel = ceil((ballDiameter / 2.0) * pixelsPerMillimeter);
    uint16_t errorRadiusLow = ceil(radiusInPixel * (errorLow / 100.0));
    uint16_t errorRadiusHigh = ceil(radiusInPixel * (errorHigh / 100.0));
    uint16_t minRadius = radiusInPixel - errorRadiusLow;
    uint16_t maxRadius = radiusInPixel + errorRadiusHigh;

    // minimal distance between centers
    double houghMinDistance = minRadius * 2;
    // higher threshold of the two passed to the Canny edge detector (lower threshold is twice smaller)
    double houghCannyHigherThreshold = 60;
    // accumulator threshold for the circle centers (if the value is smaller, the more "circles" are detected)
    double houghAccumulatorThreshold = 5;

    std::cout << "radius in pixels: " << std::to_string(radiusInPixel) << " min radius: " << std::to_string(minRadius) << " max radius: " << std::to_string(maxRadius) << " error radius range: " << std::to_string(errorRadiusLow) << ", " << std::to_string(errorRadiusHigh) << std::endl;

    auto original = imread(imagePath, cv::IMREAD_COLOR);
    cv::Mat input;
    cv::resize(original, input, cv::Size(), scale, scale);

    std::vector<cv::Mat> images;

    images.push_back(input);

    // TODO: try out different levels of blur
    cv::Mat blurred;
    cv::GaussianBlur(input, blurred, cv::Size(5, 5), 0, 0);

    images.push_back(blurred);

    cv::Mat hue, saturation, value;
    hsvFromBgr(blurred, hue, saturation, value);
    cv::imshow("hue", hue);
    cv::imshow("saturation", saturation);
    cv::imshow("value", value);

    cv::Mat hueLabels(hue.size(), CV_8UC3);
    {
        cv::Mat labels(hue.size(), CV_32S);
//        int labelCount = cv::connectedComponents(redBallMask, labels, 8, CV_32S);
        cv::Mat stats;
        cv::Mat centroids;
        int labelCount = cv::connectedComponentsWithStats(hue, labels, stats, centroids, 8, CV_32S);

        std::cout << "Number of hue labels: " << labelCount << std::endl;

        std::vector<cv::Vec3b> colors(labelCount);
        colors[0] = cv::Vec3b(0, 0, 0);//background
        for (int label = 1; label < labelCount; ++label) {
            int area = stats.at<int>(label, cv::ConnectedComponentsTypes::CC_STAT_AREA );
            cv::Point center(centroids.at<double>(label, 0), centroids.at<double>(label, 1));

            if (area > 1000 && area < 10000) {
//                std::cout << "label: " << std::to_string(label) << " center: " << center << " area: " << std::to_string(area) << std::endl;
                colors[label] = cv::Vec3b( (rand()&255), (rand()&255), (rand()&255) );
            } else {
                colors[label] = cv::Vec3b(0, 0, 0);
            }
        }
        for (int r = 0; r < hueLabels.rows; ++r) {
            for (int c = 0; c < hueLabels.cols; ++c) {
                int label = labels.at<int>(r, c);
                cv::Vec3b &pixel = hueLabels.at<cv::Vec3b>(r, c);
                pixel = colors[label];
            }
        }
        for (int label = 1; label < labelCount; ++label) {
            int area = stats.at<int>(label, cv::ConnectedComponentsTypes::CC_STAT_AREA );
            cv::Point center(centroids.at<double>(label, 0), centroids.at<double>(label, 1));

            if (area > 1000 && area < 10000) {
                cv::circle(hueLabels, center, 1, cv::Scalar(0, 100, 100), 3, cv::LINE_AA);
            }
        }
    }

    images.push_back(hueLabels);

    cv::Mat houghOnHueLabels;
    {
        cv::Mat grayscale;
        cv::cvtColor(hueLabels, grayscale, cv::COLOR_BGR2GRAY);

        std::vector<cv::Vec3f> circles;
        HoughCircles(grayscale, circles, cv::HOUGH_GRADIENT, 1,
                     houghMinDistance, // minimal distance between centers
                     houghCannyHigherThreshold, // higher threshold of the two passed to the Canny edge detector (lower threshold is twice smaller)
                     houghAccumulatorThreshold, // accumulator threshold for the circle centers (if the value is smaller, the more "circles" are detected)
                     minRadius, maxRadius
        );

        input.copyTo(houghOnHueLabels);
        drawHoughResult(houghOnHueLabels, circles);
    }
    images.push_back(houghOnHueLabels);

    imageGallery("Image", images);
}
