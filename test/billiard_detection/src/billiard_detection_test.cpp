#include <gtest/gtest.h>
#include <billiard_detection/billiard_detection.hpp>
#include <billiard_snooker/billiard_snooker.hpp>
#include "config.hpp"
#include <nlohmann/json.hpp>
#include <fstream>
#include "util.hpp"

struct DetectionBall {
    glm::vec2 position;
};

struct DetectionTestCase {
    std::string image;
    std::vector<DetectionBall> balls;
};

struct DetectionTestCases {
    std::vector<DetectionTestCase> cases;
};

DetectionTestCases merge(const DetectionTestCases& testcases1, const DetectionTestCases& testcases2);
DetectionTestCases detectionTestCases(nlohmann::json& json);
DetectionTestCases loadDetectionTestCases(const std::string& configFilepath);
billiard::detection::State toState(const std::vector<DetectionBall>& detectionBalls);

struct DetectionStats {
    DetectionTestCase testcase;
    std::vector<int> matchedExpectedBalls;
    std::vector<int> matchedActualBalls;
    std::vector<int> lostBalls;
    std::vector<int> ghostBalls;
    std::vector<float> distances;
    std::vector<float> sortedDistances; // sorted from highest to lowest
    float totalDistance;
};

std::ostream& operator<<(std::ostream& os, const std::vector<float>& vec);
std::ostream& operator<<(std::ostream& os, const DetectionStats& stats);
DetectionStats compare(const DetectionTestCase& testcase, const billiard::detection::State& expected, const billiard::detection::State& actual);

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

DetectionStats compare(const DetectionTestCase& testcase, const billiard::detection::State& expected, const billiard::detection::State& actual) {
    DetectionStats stats;
    stats.totalDistance = 0.0f;
    stats.testcase = testcase;
    stats.distances = std::vector<float>(expected._balls.size(), 698349.0f);
    stats.matchedExpectedBalls = std::vector<int>(expected._balls.size(), -1);
    stats.matchedActualBalls = std::vector<int>(actual._balls.size(), -1);

    for (int expectedBallIndex = 0; expectedBallIndex < expected._balls.size(); expectedBallIndex++) {
        auto& expectedBall = expected._balls[expectedBallIndex];

        for (int actualBallIndex = 0; actualBallIndex < actual._balls.size(); actualBallIndex++) {
            if (stats.matchedActualBalls[actualBallIndex] >= 0) continue;
            auto& actualBall = actual._balls[actualBallIndex];

            glm::vec2 distance = expectedBall._position - actualBall._position;
            float distanceInMM = glm::length(distance);
            if (distanceInMM < 20 /*mm*/) {

                stats.matchedExpectedBalls[expectedBallIndex] = actualBallIndex;
                stats.matchedActualBalls[actualBallIndex] = expectedBallIndex;
                stats.distances[expectedBallIndex] = distanceInMM;
                stats.totalDistance += distanceInMM;
            }
        }
    }

    for (int expectedBallIndex = 0; expectedBallIndex < expected._balls.size(); expectedBallIndex++) {
        if (stats.matchedExpectedBalls[expectedBallIndex] < 0) {
            stats.lostBalls.push_back(expectedBallIndex);
        }
    }
    for (int actualBallIndex = 0; actualBallIndex < actual._balls.size(); actualBallIndex++) {
        if (stats.matchedActualBalls[actualBallIndex] < 0) {
            stats.ghostBalls.push_back(actualBallIndex);
        }
    }

    stats.sortedDistances = stats.distances;
    std::sort(stats.sortedDistances.begin(), stats.sortedDistances.end(), [](const float& d1, const float& d2) {
        return d1 > d2;
    });

    return stats;
}

DetectionTestCases merge(const DetectionTestCases& testcases1, const DetectionTestCases& testcases2) {
    DetectionTestCases result;
    result.cases.insert(result.cases.end(), testcases1.cases.begin(), testcases1.cases.end());
    result.cases.insert(result.cases.end(), testcases2.cases.begin(), testcases2.cases.end());
    return result;
}

DetectionTestCases detectionTestCases(nlohmann::json& json) {

    std::vector<DetectionTestCase> cases;
    for (auto& caseJson : json["cases"]) {

        std::vector<DetectionBall> balls;
        for (auto& ballJson : caseJson["balls"]) {

            glm::vec2 position { ballJson["x"], ballJson["y"] };

            DetectionBall ball;
            ball.position = position;
            balls.push_back(ball);
        }

        DetectionTestCase testcase;
        testcase.image = caseJson["image"];
        testcase.balls = balls;
        cases.push_back(testcase);
    }

    DetectionTestCases testcases;
    testcases.cases = cases;
    return testcases;
}

DetectionTestCases loadDetectionTestCases(const std::string& configFilepath) {
    std::ifstream configFile{configFilepath};
    nlohmann::json json;
    configFile >> json;
    return detectionTestCases(json);
}

billiard::detection::State toState(const std::vector<DetectionBall>& detectionBalls) {
    billiard::detection::State state;
    for (auto& detectionBall : detectionBalls) {

        billiard::detection::Ball ball;
        ball._position = detectionBall.position;
        ball._type = "???";
        ball._id = "???";
        state._balls.push_back(ball);
    }
    return state;
}

std::ostream& operator<<(std::ostream& os, const std::vector<float>& vec) {
    os << "[";
    for (auto value : vec) {
        os << value <<  ", ";
    }
    os << "]";
    return os;
}

std::ostream& operator<<(std::ostream& os, const DetectionStats& stats) {
    os << "lost=" << stats.lostBalls.size() << " ghosts=" << stats.ghostBalls.size() << " total distance=" << std::to_string(stats.totalDistance) << " distances=" << stats.sortedDistances;
    return os;
}
