#include <gtest/gtest.h>
#include <billiard_detection/billiard_detection.hpp>
#include <billiard_snooker/billiard_snooker.hpp>
#include "config.hpp"
#include <nlohmann/json.hpp>
#include "util.hpp"

TEST(BallDetectionTests, detection_accuracy) {

    std::vector<std::string> testcasesPaths = {
        "./resources/test_detection/with_projector_on/with_halo/testcases.json"
    };

    cv::Size imageSize = getImageSize();
    billiard::detection::Table table = getTable();
    billiard::detection::ArucoMarkers markers = getMarkers();
    billiard::detection::CameraIntrinsics intrinsics = getIntrinsics_realsense_hd();

    DetectionTestCases testcases;
    for (auto& testcasePath : testcasesPaths) {
        testcases = merge(testcases, loadDetectionTestCases(testcasePath));
    }

    unsigned long long testcaseIndex = 0;
    bool imageChanged = true;

    std::shared_ptr<billiard::detection::DetectionConfig> detectionConfig;

    std::string imagePath;
    cv::Mat frame;
    cv::Mat drawn;
    cv::Mat actualGrid;
    cv::Mat badGrid;

    while(true) {

        if (imageChanged) {
            imageChanged = false;

            DetectionTestCase testcase = testcases.cases[testcaseIndex];
            imagePath = testcase.image;
            frame = cv::imread(imagePath, cv::IMREAD_COLOR);
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

            billiard::detection::State pixelState = billiard::snooker::detect(billiard::detection::State(), frame);
            billiard::detection::State state = billiard::detection::pixelToModelCoordinates(*detectionConfig, pixelState);

            frame.copyTo(drawn);
            drawBalls(drawn, pixelState._balls);

            billiard::detection::State expected = toState(testcase.balls);

//            expectedGrid = drawDetectedBallsGrid(frame, expected, 128, 8); // TODO: this does not work
            actualGrid = drawDetectedBallsGrid(drawn, pixelState, 128, 8);

            DetectionStats stats = compare(testcase, expected, state);
            std::cout << "Case: " << testcase.image << std::endl
                      << "  Stats: " << stats << std::endl;

            billiard::detection::State badDetections;
            for (int i = 0; i < expected._balls.size(); i++) {
                float distance = stats.distances[i];
                if (distance > 3.0f /*mm*/) {
                    int actualIndex = stats.matchedExpectedBalls[i];
                    if (actualIndex >= 0) {
                        badDetections._balls.push_back(pixelState._balls[actualIndex]);
                    }
                }
            }
            badGrid = drawDetectedBallsGrid(drawn, badDetections, 128, 8);
        }

        cv::imshow("Image", frame);
        cv::imshow("actual grid", actualGrid);
        cv::imshow("bad grid", badGrid);

        char key = (char) cv::waitKey(30);
        if (key == 'q' || key == 27) {
            break;
        } else if (key == 97 /* A */) {
            testcaseIndex = testcaseIndex == 0 ? testcases.cases.size() - 1 : testcaseIndex - 1;
            imageChanged = true;
        } else if (key == 100 /* D */) {
            testcaseIndex = (testcaseIndex + 1) % (testcases.cases.size());
            imageChanged = true;
        }
    }
}

TEST(BallDetectionTests, stats) {

    std::vector<std::string> testcasesPaths = {
            "./resources/test_detection/with_projector_on/with_halo/testcases.json"
    };

    cv::Size imageSize = getImageSize();
    billiard::detection::Table table = getTable();
    billiard::detection::ArucoMarkers markers = getMarkers();
    billiard::detection::CameraIntrinsics intrinsics = getIntrinsics_realsense_hd();

    DetectionTestCases testcases;
    for (auto& testcasePath : testcasesPaths) {
        testcases = merge(testcases, loadDetectionTestCases(testcasePath));
    }

    std::vector<DetectionStats> allStats;

    for (int testcaseIndex = 0; testcaseIndex < testcases.cases.size(); testcaseIndex++) {

        DetectionTestCase testcase = testcases.cases[testcaseIndex];
        std::string imagePath = testcase.image;
        cv::Mat frame = cv::imread(imagePath, cv::IMREAD_COLOR);
        cv::resize(frame, frame, imageSize);

        auto detectionConfig = std::make_shared<billiard::detection::DetectionConfig>(billiard::detection::configure(frame, table, markers, intrinsics));
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

        billiard::detection::State expected = toState(testcase.balls);

        DetectionStats stats = compare(testcase, expected, state);
        allStats.push_back(stats);
    }

    for (auto& stats : allStats) {
        std::cout << "Case: " << stats.testcase.image << std::endl
                  << "  Stats: " << stats << std::endl;
    }
}
