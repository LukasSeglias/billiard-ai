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

TEST(BallDetectionTests, stats) {

    bool writeBadDetectionImages = false;
    float criticalModelDistance = 7; // In millimeters

    std::vector<std::string> testcasesPaths = {
            "./resources/test_detection/with_projector_on/without_halo/testcases.json",
            "./resources/test_detection/with_projector_on/with_halo/testcases.json",
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

    std::cout << "Got " << testcases.cases.size() << " images" << std::endl;

    int badDetectionNumber = 1;

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

            for (int i = 0; i < expectedModelState._balls.size(); i++) {
                auto& modelDistance = modelStats.distances[i];
                auto& pixelDistance = pixelStats.distances[i];
                auto actualPixelBall = pixelState._balls[pixelStats.matchedActualBalls[i]];

                if (modelDistance > criticalModelDistance) {
                    auto& pixelBall = expectedPixelState._balls[i];

                    double radius = detectionConfig->ballRadiusInPixel;
                    const cv::Rect& roi = cv::Rect{
                            (int) ((double) pixelBall._position.x - radius),
                            (int) ((double) pixelBall._position.y - radius),
                            (int) (2 * radius),
                            (int) (2 * radius)
                    };
                    cv::Point2i roiCenterPoint { (int)radius, (int)radius };
                    cv::Point2i expectedPixelPoint { (int)pixelBall._position.x, (int)pixelBall._position.y };
                    cv::Point2i actualPixelPoint { (int)actualPixelBall._position.x, (int)actualPixelBall._position.y };

                    cv::Mat ballImage = frame(roi);
                    cv::circle(ballImage, roiCenterPoint, 1, cv::Scalar(255, 255, 0), 1, cv::LINE_AA);
                    cv::circle(ballImage, actualPixelPoint - expectedPixelPoint + roiCenterPoint, 1, cv::Scalar(0, 255, 255), 1, cv::LINE_AA);

                    if (writeBadDetectionImages) cv::imwrite("bad_detection_" + std::to_string(badDetectionNumber++) + "_" + std::to_string(modelDistance) + "_" + std::to_string(pixelDistance) + ".png", ballImage);
                }
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
        std::cout << "Pixel distance: count=" << allPixelDistances.size() << ", " << pixelSummary << std::endl;
    }
    std::sort(allModelDistances.begin(), allModelDistances.end());
    FiveNumberSummary modelSummary = calculateFiveNumberSummary(allModelDistances);
    std::cout << "Model distance: count=" << allModelDistances.size() << ", " << modelSummary << std::endl;

}


TEST(BallDetectionTests, stats_per_type) {

    std::vector<std::string> testcasesPaths = {
            "./resources/test_detection/with_projector_on/with_halo_2/testcases.json"
    };

    std::vector<std::string> labels = {
            "BROWN",
            "PINK",
            "RED",
            "BLACK",
            "YELLOW",
            "WHITE",
            "BLUE",
            "GREEN",
            "UNKNOWN"
    };

    cv::Size imageSize = getImageSize();
    billiard::detection::Table table = getTable();
    billiard::detection::ArucoMarkers markers = getMarkers();
    billiard::detection::CameraIntrinsics intrinsics = getIntrinsics_realsense_hd();

    DetectionTestCases testcases;
    for (auto& testcasePath : testcasesPaths) {
        testcases = merge(testcases, loadDetectionTestCases(testcasePath));
    }

    std::cout << "Got " << testcases.cases.size() << " images" << std::endl;

    std::unordered_map<std::string, FiveNumberSummary> pixelSummaryPerLabel;
    std::unordered_map<std::string, FiveNumberSummary> modelSummaryPerLabel;
    std::unordered_map<std::string, int> countPerLabel;

    for (auto& label : labels) {

        std::vector<float> allPixelDistances;
        std::vector<float> allModelDistances;

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
            for (int i = 0; i < expectedModelState._balls.size(); i++) {
                if (expectedModelState._balls[i]._type == label) {
                    auto& distance = modelStats.distances[i];
                    allModelDistances.push_back(distance);
                }
            }

            if (!expectedPixelState._balls.empty()) {
                DetectionStats pixelStats = compare(testcase, expectedPixelState, pixelState);

                for (int i = 0; i < expectedModelState._balls.size(); i++) {
                    if (expectedModelState._balls[i]._type == label) {
                        auto& distance = pixelStats.distances[i];
                        allPixelDistances.push_back(distance);
                    }
                }
            }
        }

        std::cout << "Label: " << label << std::endl;
        if (!allPixelDistances.empty()) {
            std::sort(allPixelDistances.begin(), allPixelDistances.end());
            FiveNumberSummary pixelSummary = calculateFiveNumberSummary(allPixelDistances);
            std::cout << "Pixel distance: count=" << allPixelDistances.size() << ", " << pixelSummary << std::endl;
            pixelSummaryPerLabel[label] = pixelSummary;
        }
        if (!allModelDistances.empty()) {
            std::sort(allModelDistances.begin(), allModelDistances.end());
            FiveNumberSummary modelSummary = calculateFiveNumberSummary(allModelDistances);
            std::cout << "Model distance: count=" << allModelDistances.size() << ", " << modelSummary << std::endl;
            modelSummaryPerLabel[label] = modelSummary;
            countPerLabel[label] = allModelDistances.size();
        }
    }

    std::cout << "Kugelfarbe & Anzahl Kugeln & Minimum & Unteres Quartil & Median & Oberes Quartil & Maximum" << "\\\\" << std::endl;
    for (auto& label : labels) {

        if (modelSummaryPerLabel.count(label) == 0) continue;
        FiveNumberSummary summary = modelSummaryPerLabel[label];
        int count = countPerLabel[label];

        std::cout << label
                << " " << "&" << " "
                << std::to_string(count) << ""
                << " " << "&" << " "
                << std::to_string(summary.min) << "mm"
                << " " << "&" << " "
                << std::to_string(summary.lowerQuartile) << "mm"
                << " " << "&" << " "
                << std::to_string(summary.median) << "mm"
                << " " << "&" << " "
                << std::to_string(summary.upperQuartile) << "mm"
                << " " << "&" << " "
                << std::to_string(summary.max) << "mm"
                << " " << "\\\\" << " "
                << std::endl;
    }
}

