#include <opencv2/opencv.hpp>
#include <iostream>
#include <billiard_detection/billiard_detection.hpp>
#include <nlohmann/json.hpp>

struct DetectionBall {
    std::string type;
    glm::vec2 position;
    glm::vec2 pixelPosition;
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
billiard::detection::State toPixelState(const std::vector<DetectionBall>& detectionBalls);

struct DetectionStats {
    DetectionTestCase testcase;
    std::vector<int> matchedExpectedBalls;
    std::vector<int> matchedActualBalls;
    std::vector<int> wrongType;
    std::vector<int> lostBalls;
    std::vector<int> ghostBalls;
    std::vector<float> distances;
    std::vector<float> sortedDistances; // sorted from highest to lowest
    float totalDistance;
};

std::ostream& operator<<(std::ostream& os, const std::vector<float>& vec);
std::ostream& operator<<(std::ostream& os, const DetectionStats& stats);
DetectionStats compare(const DetectionTestCase& testcase, const billiard::detection::State& expected, const billiard::detection::State& actual);

void drawBalls(cv::Mat& image, std::vector<billiard::detection::Ball>& balls);
void drawTypes(cv::Mat& image, std::vector<billiard::detection::Ball>& balls);
cv::Mat drawDetectedBallsGrid(const cv::Mat& input, const billiard::detection::State& pixelState, int tileSize, int tilesPerLine);

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
