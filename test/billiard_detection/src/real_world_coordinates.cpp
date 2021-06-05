#include <gtest/gtest.h>
#include <billiard_detection/billiard_detection.hpp>
#include <billiard_snooker/billiard_snooker.hpp>
#include "config.hpp"

TEST(Detection, image_coordinates_to_world_coordinates) {

    using namespace billiard::detection;

    billiard::detection::Table table = getTable();
    billiard::detection::ArucoMarkers markers = getMarkers();

    bool live = false;
    bool matlab_intrinsics = false;

    double innerTableLength = table.innerTableLength; // millimeters
    double innerTableWidth  =  table.innerTableWidth;   // millimeters
    double ballDiameter     = table.ballDiameter;   // millimeters
    double ballRadius       = ballDiameter/2; // millimeters
    cv::Point2d innerTableCenter { innerTableLength/2, innerTableWidth/2 };

    double woodThickness = 44.5;
    std::vector<cv::Point2d> expectedModelPoints = {
            // Ball 1
            cv::Point2d { innerTableLength - (1730+woodThickness+ballRadius),innerTableWidth - (850+woodThickness+ballRadius) } - innerTableCenter,
            // Ball 2
            cv::Point2d { innerTableLength - (880+woodThickness+ballRadius),innerTableWidth - (830+woodThickness+ballRadius) } - innerTableCenter,
            // Ball 3
            cv::Point2d { innerTableLength - (1721+woodThickness+ballRadius),850+woodThickness+ballRadius } - innerTableCenter,
            // Ball 4
            cv::Point2d { innerTableLength - (877+woodThickness+ballRadius),833+woodThickness+ballRadius } - innerTableCenter,
            // Ball 5
            cv::Point2d { 1785+woodThickness+ballRadius,766+woodThickness+ballRadius } - innerTableCenter,
            // Ball 6
            cv::Point2d { 1786+woodThickness+ballRadius,innerTableWidth - (763+woodThickness+ballRadius) } - innerTableCenter,
            // Ball 7
            cv::Point2d { (540+woodThickness+ballRadius),innerTableWidth - (373+woodThickness+ballRadius) } - innerTableCenter,
            // Ball 8
            cv::Point2d { (495+woodThickness+ballRadius),innerTableWidth - (617+woodThickness+ballRadius) } - innerTableCenter,
            // Ball 9
            cv::Point2d { (689+woodThickness+ballRadius),684+woodThickness+ballRadius } - innerTableCenter,
            // Ball 10
            cv::Point2d { innerTableLength - (532+woodThickness+ballRadius),543+woodThickness+ballRadius } - innerTableCenter,
            // Ball 11
            cv::Point2d { (1534+woodThickness+ballRadius),309+woodThickness+ballRadius } - innerTableCenter,
            // Ball 12
            cv::Point2d { (309+woodThickness+ballRadius),288+woodThickness+ballRadius } - innerTableCenter,
            // Ball 13
            cv::Point2d { innerTableLength - (864+woodThickness+ballRadius),401+woodThickness+ballRadius } - innerTableCenter,
            // Ball 14
            cv::Point2d { (1621+woodThickness+ballRadius),751+woodThickness+ballRadius } - innerTableCenter,
    };

    unsigned long long imageIndex = 0;
    bool imageChanged = true;

    std::vector<std::string> imagePaths;
    for(int ball = 1; ball <= expectedModelPoints.size(); ball++) {
        std::stringstream filePath;
        filePath << "./resources/test_real_world_coordinates/real_world_" << std::to_string(ball) << ".png";
        imagePaths.push_back(filePath.str());
    }

    billiard::capture::CameraCapture capture {};
    if (live) {
        if (!capture.open()) {
            std::cerr << "Unable to open image stream" << std::endl;
            return;
        }
    }

    std::shared_ptr<billiard::detection::DetectionConfig> detectionConfig;
    billiard::detection::CameraIntrinsics intrinsics;

    while(true) {
        cv::Mat frame;
        cv::Point2d expectedModelPoint;

        if (live) {
            if (imageChanged) {
                billiard::capture::CameraFrames frames = capture.read();
                frames.color.copyTo(frame);
                cv::resize(frame, frame, cv::Size(1280, 720), 0, 0);
            }
        } else {
            frame = cv::imread(imagePaths[imageIndex]);
            cv::resize(frame, frame, cv::Size(1280, 720), 0, 0);
            expectedModelPoint = expectedModelPoints[imageIndex];
        }

        if (imageChanged) {
            imageChanged = false;

            std::cout << "Image: " << imagePaths[imageIndex] << std::endl;

            if (matlab_intrinsics) {
                std::cout << "using matlab intrinsics" << std::endl;
                intrinsics = getIntrinsics_matlab_hd();
            } else {
                std::cout << "using realsense intrinsics" << std::endl;
                intrinsics = getIntrinsics_realsense_hd();
            }

            detectionConfig = std::make_shared<billiard::detection::DetectionConfig>(billiard::detection::configure(frame, table, markers, intrinsics));
            if (!detectionConfig->valid) {
                std::cout << "Unable to configure detection" << std::endl;
                return;
            }

            if (!billiard::snooker::configure(*detectionConfig)) {
                std::cout << "Unable to configure snooker detection" << std::endl;
                return;
            }

            billiard::detection::State pixelState = billiard::snooker::detect(billiard::detection::State(), frame);
            billiard::detection::State state = billiard::detection::pixelToModelCoordinates(*detectionConfig, pixelState);

            for (auto& ball : state._balls) {

                cv::Point2d modelPoint = cv::Point2d(ball._position.x, ball._position.y);
                std::cout << "model point: " << modelPoint << " should be at " << expectedModelPoint
                          << " dist: " << (modelPoint - expectedModelPoint)
                          << " dist: " << cv::norm(modelPoint - expectedModelPoint) << std::endl;
            }
        }

        cv::imshow("Image", frame);
        char key = (char) cv::waitKey(30);
        if (key == 'q' || key == 27) {
            break;
        } else if (key == 'r') {
            imageChanged = true;
        } else if (key == 'm') {
            matlab_intrinsics = !matlab_intrinsics;
            imageChanged = true;
        }  else if (key == 97 /* A */) {
            imageIndex = imageIndex == 0 ? imagePaths.size() - 1 : imageIndex - 1;
            imageChanged = true;
        } else if (key == 100 /* D */) {
            imageIndex = (imageIndex + 1) % (imagePaths.size());
            imageChanged = true;
        }
    }
}

cv::Ptr<cv::aruco::Board> createArucoBoard2(float markerLengthMilimeters) {
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

TEST(Detection, image_coordinates_to_world_coordinates_old) {

    using namespace billiard::detection;

    bool live = false;

    // Config
    double innerTableLength = 1881.0; // millimeters
    double innerTableWidth  =  943;   // millimeters
    double ballDiameter     = 52.3;   // millimeters
    double ballRadius       = ballDiameter/2; // millimeters
    cv::Point2d innerTableCenter { innerTableLength/2, innerTableWidth/2 };

    const float markerLength = 50; // length of marker in millimeters in the real world
    cv::Ptr<cv::aruco::Board> board = createArucoBoard2(markerLength);
    CameraIntrinsics intrinsics = getIntrinsics_realsense_full_hd();
    Plane plane {{0, 0, 14.2 - ballRadius}, {0, 0, 1}};

    cv::Vec3d worldToRail { 79, -71.5, 0.0 };
    cv::Vec3d railToModel { innerTableCenter.x, innerTableCenter.y, 0.0 };

    WorldToModelCoordinates worldToModel;
    worldToModel.translation = worldToRail - railToModel;

    std::vector<cv::Point2d> expectedImagePoints = {
            cv::Point2d { 166, 957 }, // Ball 1
            cv::Point2d { 929, 926 }, // Ball 2
            cv::Point2d { 163, 150 }, // Ball 3
            cv::Point2d { 931, 170 }, // Ball 4
            cv::Point2d { 1743, 226 }, // Ball 5
            cv::Point2d { 1738, 864 }, // Ball 6
            cv::Point2d { 637, 524 },  // Ball 7
            cv::Point2d { 629, 743 },  // Ball 8
            cv::Point2d { 814, 287 },  // Ball 9
            cv::Point2d { 1273, 413 }, // Ball 10
            cv::Point2d { 1561, 621 }, // Ball 11
            cv::Point2d { 475, 639 },  // Ball 12
            cv::Point2d { 980, 537 },  // Ball 13
            cv::Point2d { 1640, 225 },  // Ball 14
    };
    double woodThickness = 44.5;
    std::vector<cv::Point2d> expectedModelPoints = {
            // Ball 1
            cv::Point2d { innerTableLength - (1730+woodThickness+ballRadius),innerTableWidth - (850+woodThickness+ballRadius) } - innerTableCenter,
            // Ball 2
            cv::Point2d { innerTableLength - (880+woodThickness+ballRadius),innerTableWidth - (830+woodThickness+ballRadius) } - innerTableCenter,
            // Ball 3
            cv::Point2d { innerTableLength - (1721+woodThickness+ballRadius),850+woodThickness+ballRadius } - innerTableCenter,
            // Ball 4
            cv::Point2d { innerTableLength - (877+woodThickness+ballRadius),833+woodThickness+ballRadius } - innerTableCenter,
            // Ball 5
            cv::Point2d { 1785+woodThickness+ballRadius,766+woodThickness+ballRadius } - innerTableCenter,
            // Ball 6
            cv::Point2d { 1786+woodThickness+ballRadius,innerTableWidth - (763+woodThickness+ballRadius) } - innerTableCenter,
            // Ball 7
            cv::Point2d { (540+woodThickness+ballRadius),innerTableWidth - (373+woodThickness+ballRadius) } - innerTableCenter,
            // Ball 8
            cv::Point2d { (495+woodThickness+ballRadius),innerTableWidth - (617+woodThickness+ballRadius) } - innerTableCenter,
            // Ball 9
            cv::Point2d { (689+woodThickness+ballRadius),684+woodThickness+ballRadius } - innerTableCenter,
            // Ball 10
            cv::Point2d { innerTableLength - (532+woodThickness+ballRadius),543+woodThickness+ballRadius } - innerTableCenter,
            // Ball 11
            cv::Point2d { (1534+woodThickness+ballRadius),309+woodThickness+ballRadius } - innerTableCenter,
            // Ball 12
            cv::Point2d { (309+woodThickness+ballRadius),288+woodThickness+ballRadius } - innerTableCenter,
            // Ball 13
            cv::Point2d { innerTableLength - (864+woodThickness+ballRadius),401+woodThickness+ballRadius } - innerTableCenter,
            // Ball 14
            cv::Point2d { (1621+woodThickness+ballRadius),751+woodThickness+ballRadius } - innerTableCenter,
    };

    double railWorldPointZComponent = 0;
    std::vector<std::pair<cv::Point2d, cv::Point2d>> rails;
    std::vector<cv::Point2d> railRect;
    {
        double middlePocketRadius = 50;
        double cornerPocketRadius = 55;
        double railTop = innerTableWidth;
        double railBottom = 0.0;
        double railLeft = 0.0;
        double railRight = innerTableLength;
        rails = { // In model coordinates
                { // Left
                        cv::Point2d { railLeft, cornerPocketRadius } - innerTableCenter,
                        cv::Point2d { railLeft, railTop - cornerPocketRadius } - innerTableCenter
                },
                { // Top-left
                        cv::Point2d { railLeft + cornerPocketRadius, railTop } - innerTableCenter,
                        cv::Point2d { (railRight - railLeft)/2 - middlePocketRadius, railTop } - innerTableCenter
                },
                { // Top-right
                        cv::Point2d { (railRight - railLeft)/2 + middlePocketRadius, railTop } - innerTableCenter,
                        cv::Point2d { railRight - cornerPocketRadius, railTop } - innerTableCenter
                },
                { // Right
                        cv::Point2d { railRight, railTop - cornerPocketRadius } - innerTableCenter,
                        cv::Point2d { railRight, railBottom + cornerPocketRadius } - innerTableCenter
                },
                { // Bottom-right
                        cv::Point2d { railRight - cornerPocketRadius, railBottom } - innerTableCenter,
                        cv::Point2d { (railRight - railLeft)/2 + middlePocketRadius, railBottom } - innerTableCenter
                },
                { // Bottom-left
                        cv::Point2d { (railRight - railLeft)/2 - middlePocketRadius, railBottom } - innerTableCenter,
                        cv::Point2d { railLeft + cornerPocketRadius, railBottom } - innerTableCenter
                }
        };
        railRect = { // In model coordinates
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

    std::vector<std::pair<cv::Point2d, double>> pockets;
    {
        double middlePocketRadius = 70;
        double cornerPocketRadius = 70;
        double pocketTop = innerTableWidth;
        double pocketBottom = 0.0;
        double pocketLeft = 0.0;
        double pocketRight = innerTableLength;
        double pocketMiddle = innerTableLength/2;
        pockets = { // In model coordinates
                { cv::Point2d { pocketLeft + 30,   pocketBottom + 30 } - innerTableCenter, cornerPocketRadius }, // Bottom-left
                { cv::Point2d { pocketLeft + 30,   pocketTop - 30 }    - innerTableCenter, cornerPocketRadius }, // Top-left
                { cv::Point2d { pocketMiddle, pocketTop + 15 }    - innerTableCenter, middlePocketRadius }, // Top-middle
                { cv::Point2d { pocketRight - 30,  pocketTop - 30 }    - innerTableCenter, cornerPocketRadius }, // Top-right
                { cv::Point2d { pocketRight - 30,  pocketBottom + 30 } - innerTableCenter, cornerPocketRadius }, // Bottom-right
                { cv::Point2d { pocketMiddle, pocketBottom - 15 } - innerTableCenter, middlePocketRadius }, // Bottom-middle
        };
    }

    unsigned long long imageIndex = 0;
    bool imageChanged = true;

    std::vector<std::string> imagePaths;
    for(int ball = 1; ball <= expectedImagePoints.size(); ball++) {
        std::stringstream filePath;
        filePath << "./resources/test_real_world_coordinates/real_world_" << std::to_string(ball) << ".png";
        imagePaths.push_back(filePath.str());
    }

    billiard::capture::CameraCapture capture {};
    if (live) {
        if (!capture.open()) {
            std::cerr << "Unable to open image stream" << std::endl;
            return;
        }
    }

    while(true) {
        cv::Mat frame;
        cv::Point2d expectedImagePoint;
        cv::Point2d expectedModelPoint;

        if (live) {
            if (imageChanged) {
                billiard::capture::CameraFrames frames = capture.read();
                frames.color.copyTo(frame);
            }
        } else {
            frame = cv::imread(imagePaths[imageIndex]);
            expectedImagePoint = expectedImagePoints[imageIndex];
            expectedModelPoint = expectedModelPoints[imageIndex];
        }

        double scale = 0.5;
        cv::Mat resizedColor;
        cv::resize(frame, resizedColor, cv::Size(), scale, scale);
        cv::imshow("Color", resizedColor);

        int pixelRadius = 30;
        cv::Mat frameCopy;
        frame.copyTo(frameCopy);
        cv::circle(frameCopy, expectedImagePoint, 1, cv::Scalar{0, 255, 0}, 1);

        if (expectedImagePoint.x != 0 && expectedImagePoint.y != 0) {
            cv::Mat ballImage = frameCopy(cv::Rect{(int)expectedImagePoint.x - pixelRadius, (int)expectedImagePoint.y - pixelRadius, 2 * pixelRadius, 2 * pixelRadius});
            cv::Mat ballImageScaled;
            cv::resize(ballImage, ballImageScaled, cv::Size(), 4.0, 4.0);
            cv::imshow("Ball truth", ballImageScaled);
        }

        CameraToWorldCoordinateSystemConfig config = billiard::detection::configure(frame, board, intrinsics);
        if (config.valid) {

            std::vector<cv::Point3d> worldPoints = imagePointsToWorldPoints(config, plane, {expectedImagePoint});
            std::vector<cv::Point2d> imagePoints = worldPointsToImagePoints(config, worldPoints);
            std::vector<cv::Point2d> modelPoints = worldPointsToModelPoints(worldToModel, worldPoints);

            cv::Point2d& imagePoint = imagePoints[0];
            cv::Point3d& worldPoint = worldPoints[0];
            cv::Point2d& modelPoint = modelPoints[0];

            cv::Mat output = frame.clone();
            cv::circle(output, imagePoint, 5, cv::Scalar{0, 255, 0}, 1);
            cv::resize(output, output, cv::Size(), scale, scale);

            cv::imshow("Output", output);
            std::cout << "image point: " << imagePoint << " should be at: " << expectedImagePoint << std::endl;
            std::cout << "world point: " << worldPoint << std::endl;
            std::cout << "model point: " << modelPoint << " should be at " << expectedModelPoint
                      << " dist: " << (modelPoint - expectedModelPoint)
                      << " dist: " << cv::norm(modelPoint - expectedModelPoint) << std::endl;

            cv::Mat mask(cv::Size(frame.cols, frame.rows), CV_8UC1, cv::Scalar(0));

            double tableAspectRatio = innerTableLength/innerTableWidth;
            cv::Mat rectMask(cv::Size((int)(tableAspectRatio * 512), 512), CV_8UC1, cv::Scalar(255));

            // Display rails
            {
                cv::Mat railOutput = frame.clone();

                std::stringstream outputStream;

                for(auto& segment : rails) {
                    std::vector<cv::Point2d> modelPoints = { segment.first, segment.second };
                    std::vector<cv::Point3d> worldPoints = modelPointsToWorldPoints(worldToModel, modelPoints, railWorldPointZComponent);
                    std::vector<cv::Point2d> imagePoints = worldPointsToImagePoints(config, worldPoints);

                    outputStream << "[RAILS] " << "model: " << modelPoints[0] << " world: " << worldPoints[0] << " image: " << imagePoints[0] << "" << std::endl;
                    outputStream << "[RAILS] " << "model: " << modelPoints[1] << " world: " << worldPoints[1] << " image: " << imagePoints[1] << "" << std::endl;

                    cv::line(railOutput, imagePoints[0], imagePoints[1], cv::Scalar(255, 0, 0), 2);
                }

                std::cout << outputStream.str() << std::flush;

                cv::resize(railOutput, railOutput, cv::Size(), scale, scale);
                cv::imshow("rails", railOutput);
            }

            // Display rail-rect
            {
                cv::Mat railOutput = frame.clone();

                std::vector<cv::Point2d> modelPoints = railRect;
                std::vector<cv::Point3d> worldPoints = modelPointsToWorldPoints(worldToModel, modelPoints, railWorldPointZComponent);
                std::vector<cv::Point2d> imagePoints = worldPointsToImagePoints(config, worldPoints);

                std::stringstream outputStream;
                for (int i = 0; i < railRect.size(); i++) {
                    outputStream << "[RAIL-RECT] " << "model: " << modelPoints[i] << " world: " << worldPoints[i] << " image: " << imagePoints[i] << "" << std::endl;
                }
                std::cout << outputStream.str() << std::flush;

                cv::Scalar color(0, 0, 255);
                int thickness = 1;
                cv::line(railOutput, imagePoints[0], imagePoints[1], color, thickness);
                cv::line(railOutput, imagePoints[1], imagePoints[2], color, thickness);
                cv::line(railOutput, imagePoints[2], imagePoints[3], color, thickness);
                cv::line(railOutput, imagePoints[3], imagePoints[0], color, thickness);

                cv::Point2d bottomLeft = imagePoints[0];
                cv::Point2d topLeft = imagePoints[1];
                cv::Point2d topRight = imagePoints[2];
                cv::Point2d bottomRight = imagePoints[3];

                std::vector<cv::Point> intImagePoints = {
                        cv::Point(bottomLeft.x, bottomLeft.y),
                        cv::Point(topLeft.x, topLeft.y),
                        cv::Point(topRight.x, topRight.y),
                        cv::Point(bottomRight.x, bottomRight.y)
                };
                cv::fillConvexPoly(mask, intImagePoints, cv::Scalar(255));

                cv::resize(railOutput, railOutput, cv::Size(), scale, scale);
                cv::imshow("rail rectangle", railOutput);
            }

            // Display pockets
            {
                cv::Mat pocketsOutput = frame.clone();

                for(auto& pocket : pockets) {
                    std::vector<cv::Point2d> modelPoints = {pocket.first};
                    std::vector<cv::Point3d> worldPoints = modelPointsToWorldPoints(worldToModel, modelPoints, railWorldPointZComponent);
                    std::vector<cv::Point2d> imagePoints = worldPointsToImagePoints(config, worldPoints);

                    // TODO: correct conversion between millimeter-radius to pixel-radius
                    int pocketPixelRadius = pocket.second * 0.6;
                    cv::circle(pocketsOutput, imagePoints[0], pocketPixelRadius, cv::Scalar{0, 0, 255}, 1);

                    cv::circle(mask, imagePoints[0], pocketPixelRadius, cv::Scalar{0}, cv::LineTypes::FILLED);
                }

                cv::resize(pocketsOutput, pocketsOutput, cv::Size(), scale, scale);
                cv::imshow("pockets", pocketsOutput);
            }

            cv::resize(mask, mask, cv::Size(), scale, scale);

            cv::Mat maskedInput;
            cv::bitwise_and(resizedColor, resizedColor, maskedInput, mask);

            cv::imshow("Mask", mask);
            cv::imshow("input masked", maskedInput);

            // Transform rectangular mask to fit image
            // Both lists of points are: bottom-left, top-left, top-right, bottom-right
//            std::vector<cv::Point2d> transformedMaskPoints = { bottomLeft, topLeft, topRight, bottomRight };
//            std::vector<cv::Point2d> maskPoints = { cv::Point2d(0, rectMask.rows), cv::Point2d(0, 0), cv::Point2d(rectMask.cols, 0), cv::Point2d(rectMask.cols, rectMask.rows) };
//            cv::Mat rectangleHomography = cv::findHomography(maskPoints, transformedMaskPoints, cv::RANSAC);
//
//            if (!rectangleHomography.empty()) {
//                cv::Mat warpedRectMask;
//                cv::warpPerspective(rectMask, warpedRectMask, rectangleHomography, frame.size());
//
//                cv::resize(warpedRectMask, warpedRectMask, cv::Size(), scale, scale);
//
//                cv::Mat maskedInput;
//                cv::bitwise_and(resizedColor, resizedColor, maskedInput, warpedRectMask);
//
//                cv::imshow("warped rectangle mask", warpedRectMask);
//                cv::imshow("input masked by warped rectangle mask", maskedInput);
//            }
        }

        char key = (char) cv::waitKey(30);
        if (key == 'q' || key == 27) {
            break;
        } else if (key == 'r') {
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
