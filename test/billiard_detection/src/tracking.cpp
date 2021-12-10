#include <gtest/gtest.h>
#include <billiard_detection/billiard_detection.hpp>
#include <billiard_snooker/billiard_snooker.hpp>
#include "config.hpp"
#include "util.hpp"

TEST(DetectionTest, tracking) {

    bool paused = false;
    std::string videoPath = "./resources/infinity_modus_2.avi";

    cv::Size imageSize = getImageSize();
    billiard::detection::Table table = getTable();
    billiard::detection::ArucoMarkers markers = getMarkers();
    billiard::detection::CameraIntrinsics intrinsics = getIntrinsics_realsense_hd();

    std::shared_ptr<billiard::detection::DetectionConfig> detectionConfig;

    cv::VideoCapture video = cv::VideoCapture {videoPath};
    if (!video.isOpened()) {
        std::cout << "Unable to open video" << std::endl;
        return;
    }

    cv::Mat frame;
    video.read(frame);
    cv::resize(frame, frame, imageSize);

    detectionConfig = std::make_shared<billiard::detection::DetectionConfig>(billiard::detection::configure(frame, table, markers, intrinsics));
    if (!detectionConfig->valid) {
        std::cout << "Unable to configure detection" << std::endl;
        return;
    }

    if (!billiard::snooker::configure(*detectionConfig)) {
        std::cout << "Unable to configure snooker detection" << std::endl;
        return;
    }

    bool finished = false;

    auto capture = [&video, &finished, &imageSize]() {
        cv::Mat frame;
        video.read(frame);
        if (!frame.empty()) cv::resize(frame, frame, imageSize);
        finished = frame.empty();
        return billiard::capture::CameraFrames { frame, cv::Mat(), cv::Mat() };
    };
    auto classify = [](const billiard::detection::State& previousState,
                       billiard::detection::State& currentState,
                       const cv::Mat& image) {
        billiard::snooker::classify(previousState, currentState, image);
    };
    billiard::detection::StateTracker stateTracker(capture, detectionConfig, billiard::snooker::detect, classify);

    cv::Mat drawn;
    cv::Mat grid;

    while (!finished) {

        if (!paused) {
            billiard::detection::State modelState;

            auto future = stateTracker.capture();
            future.wait();
            modelState = future.get();
        }

        char key = (char) cv::waitKey(5);
        if (key == ' ') {
            paused = !paused;
        } else if (key == 27 /* ESC */) {
            break;
        }
    }

}

TEST(DetectionTest, tracking_calculate_movement) {

    bool paused = false;
    std::string testcasePath = "./resources/video_detection/testcases.json";
    DetectionTestCases testcases = loadDetectionTestCases(testcasePath);
    DetectionTestCase testcase = testcases.cases[2];
    std::string videoPath = testcase.image;

    cv::Size imageSize = getImageSize();
    billiard::detection::Table table = getTable();
    billiard::detection::ArucoMarkers markers = getMarkers();
    billiard::detection::CameraIntrinsics intrinsics = getIntrinsics_realsense_hd();

    std::shared_ptr<billiard::detection::DetectionConfig> detectionConfig;

    cv::VideoCapture video = cv::VideoCapture {videoPath};
    if (!video.isOpened()) {
        std::cout << "Unable to open video" << std::endl;
        return;
    }

    cv::Mat frame;
    video.read(frame);
    cv::resize(frame, frame, imageSize);

    detectionConfig = std::make_shared<billiard::detection::DetectionConfig>(billiard::detection::configure(frame, table, markers, intrinsics));
    if (!detectionConfig->valid) {
        std::cout << "Unable to configure detection" << std::endl;
        return;
    }

    if (!billiard::snooker::configure(*detectionConfig)) {
        std::cout << "Unable to configure snooker detection" << std::endl;
        return;
    }

    bool finished = false;

    auto capture = [&video, &finished, &imageSize]() {
        cv::Mat frame;
        video.read(frame);
        if (!frame.empty()) cv::resize(frame, frame, imageSize);
        finished = frame.empty();
        return billiard::capture::CameraFrames { frame, cv::Mat(), cv::Mat() };
    };
    auto classify = [](const billiard::detection::State& previousState,
                       billiard::detection::State& currentState,
                       const cv::Mat& image) {
        billiard::snooker::classify(previousState, currentState, image);
    };

    auto detect = [](const billiard::detection::State& previousState,
                     const cv::Mat& image) {
        if (image.empty()) {
            return billiard::detection::State {};
        }
        return billiard::snooker::detect(previousState, image);
    };

    billiard::detection::StateTracker stateTracker(capture, detectionConfig, detect, classify);

    cv::Mat drawn;
    cv::Mat grid;

    float maxTrackingDistanceSquared = 25*25; // in millimeters
    std::vector<float> deltaMovements;
    std::vector<float> referenceMovements;
    int lost = 0;
    int frames = 0;
    int skipMovementRecordingForFrames = 30;
    billiard::detection::State previousModelState;

    std::vector<std::vector<float>> movementsByReferenceBallIndex;
    for (int j = 0; j < testcase.balls.size(); j++) {
        movementsByReferenceBallIndex.push_back(std::vector<float> {});
    }

    while (!finished) {

        if (!paused) {
            billiard::detection::State modelState;

            auto future = stateTracker.capture();
            future.wait();
            modelState = future.get();

            if (modelState._balls.empty()) {
                break;
            }

            if (skipMovementRecordingForFrames-- <= 0) {
                frames++;

                for (int i = 0; i < modelState._balls.size(); i++) {

                    auto& detectedBall = modelState._balls[i];
                    billiard::detection::Ball* matchedPreviousBall = nullptr;
                    int matchedReferenceBallIndex = -1;

                    for (int j = 0; j < testcase.balls.size(); j++) {

                        auto& referenceBall = testcase.balls[j];

                        glm::vec2 delta = detectedBall._position - referenceBall.position;
                        if (glm::dot(delta, delta) < maxTrackingDistanceSquared) {
                            // Found match
                            float distance = glm::length(delta);
                            referenceMovements.push_back(distance);
                            matchedReferenceBallIndex = j;
                            movementsByReferenceBallIndex[matchedReferenceBallIndex].push_back(distance);
                            break;
                        }
                    }

                    for (int j = 0; j < previousModelState._balls.size(); j++) {

                        auto& previousBall = previousModelState._balls[j];

                        glm::vec2 delta = detectedBall._position - previousBall._position;
                        if (glm::dot(delta, delta) < maxTrackingDistanceSquared) {
                            // Found match
                            matchedPreviousBall = &previousModelState._balls[j];;
                            deltaMovements.push_back(glm::length(delta));

                            break;
                        }
                    }

                    if (!matchedPreviousBall) {
                        lost++;
                    }
                    if (matchedReferenceBallIndex == -1) {
                        lost++;
                    }
                }
            }

            previousModelState = modelState;
        }

        char key = (char) cv::waitKey(5);
        if (key == ' ') {
            paused = !paused;
        } else if (key == 27 /* ESC */) {
            break;
        }
    }

    for (int j = 0; j < testcase.balls.size(); j++) {
        std::vector<float> movements = movementsByReferenceBallIndex[j];
        std::sort(movements.begin(), movements.end());
        FiveNumberSummary summary = calculateFiveNumberSummary(movements);
        std::cout << "Reference movement summary for reference ball " << j << ": " << summary << std::endl;
    }

    std::sort(referenceMovements.begin(), referenceMovements.end());
    FiveNumberSummary referenceSummary = calculateFiveNumberSummary(referenceMovements);
    std::cout << "Total reference movement summary: " << "count=" << referenceMovements.size() << ", " << referenceSummary << std::endl;

    std::sort(deltaMovements.begin(), deltaMovements.end());
    FiveNumberSummary deltaSummary = calculateFiveNumberSummary(deltaMovements);
    std::cout << "Total delta movement summary: " << "count=" << deltaMovements.size() << ", " << deltaSummary << std::endl;

    float totalDeltaMovement = 0.0f;
    for (auto movement : deltaMovements) {
        totalDeltaMovement += movement;
    }
    float totalReferenceMovement = 0.0f;
    for (auto movement : referenceMovements) {
        totalReferenceMovement += movement;
    }
    std::cout << "Total delta movement: " << totalDeltaMovement << "mm," << " "
              << frames << " frames" << ", "
              << "lost: " << lost
              << std::endl;

    std::cout << "Total reference movement: " << totalReferenceMovement << "mm," << " "
              << frames << " frames" << ", "
              << "lost: " << lost
              << std::endl;
}

