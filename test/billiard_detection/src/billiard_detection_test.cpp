#include <gtest/gtest.h>
#include <billiard_detection/billiard_detection.hpp>
#include <billiard_snooker/billiard_snooker.hpp>
#include "config.hpp"
#include <nlohmann/json.hpp>
#include "util.hpp"

TEST(BallDetectionTests, detection_accuracy) {

    bool autoplay = true;
    std::vector<std::string> testcasesPaths = {
        "./resources/test_detection/with_projector_on/with_halo_2/testcases.json"
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
    cv::Mat expectedDrawn;
    cv::Mat expectedGrid;
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

            billiard::detection::State expectedPixelState = toPixelState(testcase.balls);
            billiard::detection::State expectedState = toState(testcase.balls);

            if (!expectedPixelState._balls.empty()) {
                frame.copyTo(expectedDrawn);
                drawBalls(expectedDrawn, expectedPixelState._balls);
                expectedGrid = drawDetectedBallsGrid(expectedDrawn, expectedPixelState, 128, 8);
            } else {
                expectedGrid = cv::Mat();
            }
            actualGrid = drawDetectedBallsGrid(drawn, pixelState, 128, 8);

            DetectionStats stats = compare(testcase, expectedState, state);
            std::cout << "Case: " << testcase.image << std::endl
                      << "  Stats: " << stats << std::endl;

            billiard::detection::State badDetections;
            for (int i = 0; i < expectedState._balls.size(); i++) {
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
        if (!expectedGrid.empty()) cv::imshow("expected grid", expectedGrid);
        cv::imshow("actual grid", actualGrid);
        cv::imshow("bad grid", badGrid);

        if (autoplay) {
            testcaseIndex++;
            if (testcaseIndex >= testcases.cases.size()) {
                return;
            }
            imageChanged = true;

        } else {
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
}

// See: https://en.wikipedia.org/wiki/Five-number_summary
struct FiveNumberSummary {
    float min;
    float lowerQuartile;
    float median;
    float upperQuartile;
    float max;
};

FiveNumberSummary calculateFiveNumberSummary(const std::vector<float>& sortedValues);
std::ostream& operator<<(std::ostream& os, const FiveNumberSummary& summary);

TEST(BallDetectionTests, stats) {

    std::vector<std::string> testcasesPaths = {
            "./resources/test_detection/with_projector_on/with_halo_2/testcases.json"
    };

    cv::Size imageSize = getImageSize();
    billiard::detection::Table table = getTable();
    billiard::detection::ArucoMarkers markers = getMarkers();
    billiard::detection::CameraIntrinsics intrinsics = getIntrinsics_realsense_hd();

    DetectionTestCases testcases;
    for (auto& testcasePath : testcasesPaths) {
        testcases = merge(testcases, loadDetectionTestCases(testcasePath));
    }

    int totalGhosts = 0;
    int totalLost = 0;
    std::vector<float> allPixelDistances;
    std::vector<float> allModelDistances;
    std::vector<DetectionStats> allPixelStats;
    std::vector<DetectionStats> allModelStats;

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
        billiard::detection::State modelState = billiard::detection::pixelToModelCoordinates(*detectionConfig, pixelState);

        billiard::detection::State expectedPixelState = toPixelState(testcase.balls);
        billiard::detection::State expectedModelState = toState(testcase.balls);

        DetectionStats modelStats = compare(testcase, expectedModelState, modelState);
        allModelStats.push_back(modelStats);
        for (auto& distance : modelStats.distances) {
            allModelDistances.push_back(distance);
        }

        if (!expectedPixelState._balls.empty()) {
            DetectionStats pixelStats = compare(testcase, expectedPixelState, pixelState);
            allPixelStats.push_back(pixelStats);
            for (auto& distance : pixelStats.distances) {
                allPixelDistances.push_back(distance);
            }
        }

        totalLost += modelStats.lostBalls.size();
        totalGhosts += modelStats.ghostBalls.size();
    }

    for (auto& stats : allModelStats) {
        std::cout << "Case: " << stats.testcase.image << std::endl
                  << "  Model stats: " << stats << std::endl;
    }

    std::cout << "Total: lost=" << totalLost << ", ghosts=" << totalGhosts << std::endl;
    if (!allPixelDistances.empty()) {
        std::sort(allPixelDistances.begin(), allPixelDistances.end());
        FiveNumberSummary pixelSummary = calculateFiveNumberSummary(allPixelDistances);
        std::cout << "Pixel distance: " << pixelSummary << std::endl;
    }
    std::sort(allModelDistances.begin(), allModelDistances.end());
    FiveNumberSummary modelSummary = calculateFiveNumberSummary(allModelDistances);
    std::cout << "Model distance: " << modelSummary << std::endl;

}

FiveNumberSummary calculateFiveNumberSummary(const std::vector<float>& sortedValues) {
    assert(!sortedValues.empty());
    FiveNumberSummary summary;
    summary.min = sortedValues.at(0);
    summary.lowerQuartile = sortedValues.at((int)((float)sortedValues.size() * 0.25f));
    summary.median = sortedValues.at(sortedValues.size() / 2);
    summary.upperQuartile = sortedValues.at((int)((float)sortedValues.size() * 0.75f));
    summary.max = sortedValues.at(sortedValues.size() - 1);
    return summary;
}

std::ostream& operator<<(std::ostream& os, const FiveNumberSummary& summary) {
    os << "min=" << std::to_string(summary.min) << ", "
       << "lower quartile=" << std::to_string(summary.lowerQuartile) << ", "
       << "median=" << std::to_string(summary.median) << ", "
       << "upper quartile=" << std::to_string(summary.upperQuartile) << ", "
       << "max=" << std::to_string(summary.max) << " ";
    return os;
}
