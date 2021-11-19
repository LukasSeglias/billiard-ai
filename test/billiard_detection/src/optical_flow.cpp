#include <gtest/gtest.h>
#include <opencv2/opencv.hpp>
#include <iostream>
#include <billiard_detection/billiard_detection.hpp>
#include <billiard_snooker/billiard_snooker.hpp>
#include "config.hpp"
#include <algorithm>

std::vector<cv::Point2f> stateToPoints(const billiard::detection::State& state) {
    std::vector<cv::Point2f> points {};
    for (auto& ball : state._balls) {
        points.push_back(cv::Point2f {ball._position.x, ball._position.y});
    }
    return points;
}

struct TrackedBall {
    bool tracked;
    float distance;
    int previousStateIndex;
    int currentStateIndex;
    TrackedBall(bool tracked, float distance, int previousStateIndex, int currentStateIndex):
        tracked(tracked),
        distance(distance),
        previousStateIndex(previousStateIndex),
        currentStateIndex(currentStateIndex) {
    }
};

std::vector<TrackedBall> tryTracking(const billiard::detection::State& previousState, const billiard::detection::State& currentState, float maxTrackingDistanceSquared) {

    std::vector<TrackedBall> trackedBalls;
    trackedBalls.reserve(currentState._balls.size());
    std::vector<bool> assigned(previousState._balls.size(), false);

    for (int currentBallIndex = 0; currentBallIndex < currentState._balls.size(); currentBallIndex++) {
        auto& currentBall = currentState._balls[currentBallIndex];
        glm::vec2 currentPosition = currentBall._position;
        float minDistanceSquared = 2.0f * maxTrackingDistanceSquared; // just some big number
        int bestMatchIndex = -1;

        for (int previousBallIndex = 0; previousBallIndex < previousState._balls.size(); previousBallIndex++) {
            auto& previousBall = previousState._balls[previousBallIndex];
            if (assigned[previousBallIndex]) {
                continue;
            }
            glm::vec2 distance = currentPosition - previousBall._position;
            float distanceSquared = glm::dot(distance, distance);
            if (distanceSquared < maxTrackingDistanceSquared) {
                if (distanceSquared < minDistanceSquared) {
                    minDistanceSquared = distanceSquared;
                    bestMatchIndex = previousBallIndex;
                }
            }
        }

        if (bestMatchIndex >= 0) {
            assigned[bestMatchIndex] = true;
            float distance = glm::sqrt(minDistanceSquared);
            trackedBalls.push_back(TrackedBall { true, distance, bestMatchIndex, currentBallIndex});
        } else {
            trackedBalls.push_back(TrackedBall{ false, 0.0f, -1, currentBallIndex });
        }
    }
    return trackedBalls;
}

TEST(OpticalFlow, test) {

    bool ballsAreStationary = true;
    bool looped = true;
    std::string file = "./resources/2.avi";
//    std::string file = "D:\\Billiard-Videos\\detection and classification videos\\video_3_color.avi";

    float maxTrackingDistanceSquared = 10.0f * 10.0f; // In Pixels

    cv::VideoCapture capture {file};
    if (!capture.isOpened()) {
        std::cerr << "Unable to open file" << std::endl;
        return;
    }

    cv::Scalar untrackedColor {0, 0, 0};
    std::vector<cv::Scalar> colors;
    cv::RNG rng;
    for(int i = 0; i < 100; i++) {
        int r = rng.uniform(0, 256);
        int g = rng.uniform(0, 256);
        int b = rng.uniform(0, 256);
        colors.push_back(cv::Scalar(r,g,b));
    }

    cv::Size imageSize = getImageSize();
    billiard::detection::Table table = getTable();
    billiard::detection::ArucoMarkers markers = getMarkers();
    billiard::detection::CameraIntrinsics intrinsics = getIntrinsics_realsense_hd();

    std::shared_ptr<billiard::detection::DetectionConfig> detectionConfig;

    cv::Mat old_frame;
    capture.read(old_frame);

    detectionConfig = std::make_shared<billiard::detection::DetectionConfig>(billiard::detection::configure(old_frame, table, markers, intrinsics));
    if (!detectionConfig->valid) {
        std::cout << "Unable to configure detection" << std::endl;
        return;
    }

    if (!billiard::snooker::configure(*detectionConfig)) {
        std::cout << "Unable to configure snooker detection" << std::endl;
        return;
    }

    billiard::detection::State previousPixelState = billiard::snooker::detect(billiard::detection::State(), old_frame);
    billiard::detection::State previousState = billiard::detection::pixelToModelCoordinates(*detectionConfig, previousPixelState);
    billiard::detection::State currentPixelState;
    billiard::detection::State currentState;

    cv::Mat old_gray;
    cvtColor(old_frame, old_gray, cv::COLOR_BGR2GRAY);

//    std::vector<cv::Point2f> p0;
    std::vector<cv::Point2f> previousDetectionPoints = stateToPoints(previousPixelState);
    std::vector<cv::Point2f> currentDetectionPoints;
    std::vector<cv::Point2f> currentFlowPoints;
//    cv::goodFeaturesToTrack(old_gray, p0, 100, 0.3, 7, cv::noArray(), 7, false, 0.04);

    int counter = 0;
    const int detectionInterval = 1; // every N frames
    bool paused = false;
    int ballPixelRadius = (int)(1.3f * detectionConfig->ballRadiusInPixel);

    auto cutRoi = [ballPixelRadius](float x, float y, const cv::Mat& input, const cv::Size& size) {
        cv::Rect roi {
                (int) (x - ballPixelRadius),
                (int) (y - ballPixelRadius),
                (int) ballPixelRadius * 2,
                (int) ballPixelRadius * 2
        };
        if (roi.x >= 0 && roi.y >= 0 && roi.width <= input.cols && roi.height <= input.rows) {

            cv::Mat ballImage = input(roi);
            cv::Mat ballImageScaled;
            cv::resize(ballImage, ballImageScaled, size);
            return ballImageScaled;
        }
        return cv::Mat {};
    };

    struct StabilityComparison {
        int id;
        cv::Mat detectionRoi;
        cv::Mat stabilizedRoi;
        StabilityComparison() {}
        StabilityComparison(int id, cv::Mat detectionRoi, cv::Mat stabilizedRoi): id(id), detectionRoi(detectionRoi), stabilizedRoi(stabilizedRoi) {}
    };

    cv::Size roiSize {128, 128};
    std::unordered_map<int, cv::Mat> detectionRoiPerBall;
    std::unordered_map<int, cv::Mat> stabilizedRoiPerBall;
    std::unordered_map<int, StabilityComparison> stabilityComparisonPerBall;
    std::unordered_map<int, double> totalDetectionDistancePerBall;
    std::unordered_map<int, double> totalStabilizedDistancePerBall;
    std::unordered_map<int, int> totalTrackingsPerBall;

    cv::Mat trackingLines = cv::Mat::zeros(old_frame.size(), old_frame.type());
    cv::Mat detectionAndTrackingLines = cv::Mat::zeros(old_frame.size(), old_frame.type());
    while(true) {

        if (!paused) {
            cv::Mat frame;
            capture.read(frame);
            if (frame.empty()) {
                if (looped && capture.open(file)) {
                    capture.read(frame);
                } else {
                    break;
                }
            }

            cv::Mat detectionOutput;
            frame.copyTo(detectionOutput);
            cv::Mat trackingOutput;
            frame.copyTo(trackingOutput);
            cv::Mat detectionAndTrackingOutput;
            frame.copyTo(detectionAndTrackingOutput);

            cv::Mat frame_gray;
            cvtColor(frame, frame_gray, cv::COLOR_BGR2GRAY);

            counter++;
            if (counter == detectionInterval) {
                counter = 0;
                currentPixelState = billiard::snooker::detect(billiard::detection::State(), frame);
                currentState = billiard::detection::pixelToModelCoordinates(*detectionConfig, currentPixelState);
                currentDetectionPoints = stateToPoints(currentPixelState);
            }

            std::vector<TrackedBall> pixelTracking = tryTracking(previousPixelState, currentPixelState, maxTrackingDistanceSquared);
            int tracked = 0;
            int notTracked = 0;
            for (auto& trackedBall : pixelTracking) {
                if (trackedBall.tracked) {
                    tracked++;
                } else {
                    notTracked++;
                }
            }

            std::string trackingText = "Tracked: " + std::to_string(tracked) + "/" + std::to_string(tracked+notTracked);
            cv::putText(frame, trackingText, cv::Point2i {25, 25}, cv::FONT_HERSHEY_PLAIN, 1.0, cv::Scalar {255, 0, 0});
            std::cout << "Tracked: " << tracked << " not tracked: " << notTracked << std::endl;

            std::vector<uchar> status;
            std::vector<float> err;
            cv::TermCriteria criteria = cv::TermCriteria((cv::TermCriteria::COUNT) + (cv::TermCriteria::EPS), 10, 0.03);
            // TODO: use buildOpticalFlowPyramid
            cv::calcOpticalFlowPyrLK(old_gray, frame_gray, previousDetectionPoints, currentFlowPoints, status, err, cv::Size(15,15), 2, criteria);

            std::vector<cv::Point2f> displayPoints(currentState._balls.size(), cv::Point2f {0, 0});

            for (int i = 0; i < previousDetectionPoints.size(); i++) {
//                cv::Scalar& color = colors[i];
                cv::Scalar& color = colors[0];
                auto& currentFlowPoint = currentFlowPoints[i];
                line(trackingLines, currentFlowPoint, previousDetectionPoints[i], color, 2);
                circle(trackingOutput, currentFlowPoint, 5, color, -1);
                circle(trackingOutput, currentFlowPoint, ballPixelRadius, color, 2);
            }

//            float pixelSigma = glm::sqrt(frame.cols*frame.cols + frame.rows*frame.rows);
            float pixelSigma = glm::sqrt(maxTrackingDistanceSquared);
            for (auto& trackedBall : pixelTracking) {

                glm::vec2& detectedPosition = currentPixelState._balls[trackedBall.currentStateIndex]._position;
                cv::Point2f detectedPoint { detectedPosition.x, detectedPosition.y };

                if (trackedBall.tracked) {
                    float distance = trackedBall.distance;
                    float alpha = cv::exp(-distance*distance/pixelSigma);
                    glm::vec2& previousDetectedPosition = previousPixelState._balls[trackedBall.previousStateIndex]._position;
                    cv::Point2f& previousDetectedPoint = previousDetectionPoints[trackedBall.previousStateIndex];
                    cv::Point2f& flowPoint = currentFlowPoints[trackedBall.previousStateIndex];
                    glm::vec2 flowPosition { flowPoint.x, flowPoint.y };

    //                std::cout << "movement: "
    //                    << (detectedPoint-previousPixelState._balls[trackedBall.previousStateIndex])

                    std::cout << "distance: " << distance << " alpha: " << alpha << " "
                              << "flow: " << "(" << flowPosition.x << ", " << flowPosition.y << ")" << " "
                              << "detected: " << "(" << detectedPosition.x << ", " << detectedPosition.y << ")" << " "
                              << "flow vs detected: " << glm::length(flowPosition - detectedPosition)
                              << std::endl;

                    cv::Point2f displayPoint {
                            (1.0f - alpha) * detectedPoint.x + alpha * flowPoint.x,
                            (1.0f - alpha) * detectedPoint.y + alpha * flowPoint.y
                    };
                    displayPoints[trackedBall.currentStateIndex] = displayPoint;

                    if (ballsAreStationary) {
                        totalDetectionDistancePerBall[trackedBall.previousStateIndex] += trackedBall.distance;
                        totalStabilizedDistancePerBall[trackedBall.previousStateIndex] += cv::norm(displayPoint - previousDetectedPoint);
                        totalTrackingsPerBall[trackedBall.previousStateIndex]++;
                    }

//                    cv::Scalar& color = colors[trackedBall.previousStateIndex];
                    cv::Scalar& color = colors[0];
                    line(detectionAndTrackingLines, displayPoint, previousDetectedPoint, color, 2);
                    circle(detectionAndTrackingOutput, displayPoint, 5, color, -1);
                    circle(detectionAndTrackingOutput, displayPoint, ballPixelRadius, color, 2);

                    circle(detectionOutput, detectedPoint, 5, color, -1);
                    circle(detectionOutput, detectedPoint, ballPixelRadius, color, 2);

                    StabilityComparison comparison {
                            (int) glm::length(previousDetectedPosition),
                            cutRoi(detectedPoint.x, detectedPoint.y, detectionOutput, roiSize),
                            cutRoi(displayPoint.x, displayPoint.y, detectionAndTrackingOutput, roiSize)
                    };
                    stabilityComparisonPerBall[trackedBall.previousStateIndex] = comparison;

                } else {
//                    cv::Scalar& color = untrackedColor;
                    cv::Scalar& color = colors[0];
                    glm::vec2& detectedPosition = currentPixelState._balls[trackedBall.currentStateIndex]._position;
                    cv::Point2f detectedPoint { detectedPosition.x, detectedPosition.y};

                    displayPoints[trackedBall.currentStateIndex] = detectedPoint;
                    circle(detectionAndTrackingOutput, detectedPoint, 5, color, -1);

                    circle(detectionOutput, detectedPoint, 5, color, -1);
                    circle(detectionOutput, detectedPoint, ballPixelRadius, color, 2);
                }
            }

            std::string distanceOutput = "Distance diff: ";

            double detectionDistance = 0.0;
            double stabilizedDistance = 0.0;
            for (auto& entry : totalTrackingsPerBall) {
                double totalDetectionDistance = totalDetectionDistancePerBall[entry.first];
                double totalStabilizedDistance = totalStabilizedDistancePerBall[entry.first];
                int totalTrackings = entry.second;
                double averageDetectionDistance = totalDetectionDistance / (double)totalTrackings;
                double averageStabilizedDistance = totalStabilizedDistance / (double)totalTrackings;

                detectionDistance += totalDetectionDistance;
                stabilizedDistance += totalStabilizedDistance;

                distanceOutput += std::to_string(averageDetectionDistance - averageStabilizedDistance) + " ";
            }
            std::cout << distanceOutput << std::endl;
            std::cout << "detection: " << detectionDistance << " stabilized: " << stabilizedDistance << std::endl;

            std::vector<StabilityComparison> stabilityComparisons;
            for (auto& entry : stabilityComparisonPerBall) {
                stabilityComparisons.push_back(entry.second);
            }
            std::sort(stabilityComparisons.begin(), stabilityComparisons.end(), [](const StabilityComparison& a, const StabilityComparison& b) {
                return a.id < b.id;
            });

            std::vector<cv::Mat> rois;
            for (int i = 0; i < stabilityComparisons.size(); i++) {
                StabilityComparison comparison = stabilityComparisons[i];
                rois.push_back(comparison.detectionRoi);
                rois.push_back(comparison.stabilizedRoi);
            }

            int tilesPerLine = 10;
            int padding = 10;
            auto& tileSize = roiSize.width;
            int totalTiles = rois.size();
            int numberOfTileLines = totalTiles / tilesPerLine + 1;
            int imageWidth = tileSize * tilesPerLine + padding * (tilesPerLine + 1);
            int imageHeight = numberOfTileLines * tileSize + padding * (numberOfTileLines + 1);
            cv::Mat result {imageHeight, imageWidth, CV_8UC3, cv::Scalar{255, 255, 255}};
            for (int i = 0; i < rois.size(); i++) {
                auto& image = rois[i];
                int tileIndex = i;
                int colIndex = tileIndex % tilesPerLine;
                int rowIndex = tileIndex / tilesPerLine;
                int x = colIndex * tileSize + padding * (colIndex + 1);
                int y = rowIndex * tileSize + padding * (rowIndex + 1);
                cv::Rect resultRoi {cv::Point(x, y), cv::Size {image.cols, image.rows}};
                cv::Mat dst = result(resultRoi);
                image.copyTo(dst);
            }

            cv::Mat trackingResult;
            cv::add(trackingOutput, trackingLines, trackingResult);

            cv::Mat detectionAndTrackingResult;
            cv::add(detectionAndTrackingOutput, detectionAndTrackingLines, detectionAndTrackingResult);

            cv::imshow("detection & tracking", detectionAndTrackingResult);
            cv::imshow("detection", detectionOutput);
            cv::imshow("tracking", trackingResult);
            cv::imshow("result", result);

            old_frame = frame.clone();
            old_gray = frame_gray.clone();
            previousPixelState = currentPixelState;
            previousState = currentState;
            previousDetectionPoints = currentDetectionPoints;
        }

        int keyboard = cv::waitKey(25);
        if (keyboard == 'q' || keyboard == 27) {
            break;
        } else if (keyboard == ' ') {
            paused = !paused;
        }
    }
}
