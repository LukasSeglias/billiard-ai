#include "util.hpp"
#include <fstream>

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

                if (expectedBall._type != actualBall._type) {
                    stats.wrongType.push_back(actualBallIndex);
                }
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
            std::string type = ballJson.contains("type") ? ballJson["type"] : "???";

            DetectionBall ball;
            ball.position = position;
            ball.type = type;
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
        ball._type = detectionBall.type;
        ball._id = "???";
        state._balls.push_back(ball);
    }
    return state;
}

void drawBalls(cv::Mat& image, std::vector<billiard::detection::Ball>& balls) {
    for(auto& c : balls) {
        cv::Point center = cv::Point(c._position.x, c._position.y);
        uint8_t radius = 15;
        cv::Rect roi(center.x - radius, center.y - radius, radius * 2, radius * 2);
        if (roi.x >= 0 && roi.y >= 0 && roi.width <= image.cols && roi.height <= image.rows) {

            // circle center
            cv::circle(image, center, 1, cv::Scalar(0, 100, 100), 3, cv::LINE_AA);
            // circle outline
            cv::circle(image, center, radius, cv::Scalar(255, 0, 255), 1, cv::LINE_AA);
        }
    }
}

void drawTypes(cv::Mat& image, std::vector<billiard::detection::Ball>& balls) {
    for(auto& c : balls) {
        int fontFace = cv::FONT_HERSHEY_PLAIN;
        double fontScale = 0.5;
        int spacing = 15;
        int thickness = 1;

        cv::Point origin = cv::Point(c._position.x, c._position.y + spacing);

        std::string text = c._type;
        cv::Size textSize = cv::getTextSize(text, fontFace, fontScale, thickness, NULL);

        cv::Rect roi(origin.x, origin.y, textSize.width, textSize.height);
        if (roi.x >= 0 && roi.y >= 0 && roi.width <= image.cols && roi.height <= image.rows) {

            cv::putText(image, text, origin, fontFace, fontScale, cv::Scalar {255, 0, 0}, thickness);
        }
    }
}

cv::Mat drawDetectedBallsGrid(const cv::Mat& input, const billiard::detection::State& pixelState, int tileSize, int tilesPerLine) {

    cv::Scalar backgroundColor {255, 255, 255};
    int padding = 10;
    int totalTiles = pixelState._balls.size();
    int numberOfTileLines = totalTiles / tilesPerLine + 1;
    int imageWidth = tileSize * tilesPerLine + padding * (tilesPerLine + 1);
    int imageHeight = numberOfTileLines * tileSize + padding * (numberOfTileLines + 1);

#if 0
    std::cout
            << "Ball tiles: " << " "
            << "totalTiles=" << totalTiles << " "
            << "tileSize=" << tileSize << " "
            << "tilesPerLine=" << tilesPerLine << " "
            << "numberOfTileLines=" << numberOfTileLines << " "
            << "imageWidth=" << imageWidth << " "
            << "imageHeight=" << imageHeight << " "
            << std::endl;
#endif

    float ballRadiusInPixels = 30;

    cv::Mat result {imageHeight, imageWidth, CV_8UC3, backgroundColor};

    int tileIndex = 0;
    for (auto& ball : pixelState._balls) {

        auto& position = ball._position;
        cv::Rect roi {
                (int) (position.x - ballRadiusInPixels),
                (int) (position.y - ballRadiusInPixels),
                (int) ballRadiusInPixels * 2,
                (int) ballRadiusInPixels * 2
        };
        if (roi.x >= 0 && roi.y >= 0 && roi.width <= input.cols && roi.height <= input.rows) {

            cv::Mat ballImage = input(roi);
            cv::Mat ballImageScaled;
            cv::resize(ballImage, ballImageScaled, cv::Size {tileSize, tileSize});

            int colIndex = tileIndex % tilesPerLine;
            int rowIndex = tileIndex / tilesPerLine;
            int x = colIndex * tileSize + padding * (colIndex + 1);
            int y = rowIndex * tileSize + padding * (rowIndex + 1);
            cv::Rect resultRoi {cv::Point(x, y), cv::Size {ballImageScaled.cols, ballImageScaled.rows}};
            cv::Mat dst = result(resultRoi);
            ballImageScaled.copyTo(dst);

#if 0
            std::cout
                    << "    Ball tile: " << " "
                    << "colIndex=" << colIndex << " "
                    << "rowIndex=" << rowIndex << " "
                    << "x=" << x << " "
                    << "y=" << y << " "
                    << std::endl;
#endif

        } else {
            std::cout << "unable to cut out ball image since roi is not inside image" << std::endl;
        }
        tileIndex++;
    }
    return result;
}

