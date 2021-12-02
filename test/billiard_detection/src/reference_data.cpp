#include <gtest/gtest.h>
#include <opencv2/opencv.hpp>
#include <iostream>
#include <billiard_detection/billiard_detection.hpp>
#include <billiard_snooker/billiard_snooker.hpp>
#include "config.hpp"
#include "util.hpp"
#include <chrono>

void onReferenceDataMouseClick(int event, int x, int y, int flags, void* userdata);

billiard::detection::State pixelState;
billiard::detection::State state;
int selectedBallIndex = -1;
cv::Point2i clickedPixel {0, 0};

TEST(BallDetectionTests, record_reference_data) {

    std::vector<std::string> imagePaths = {
//            "./resources/test_detection/with_projector_on/without_halo/1.png",
//            "./resources/test_detection/with_projector_on/without_halo/2.png",
//            "./resources/test_detection/with_projector_on/without_halo/3.png",
//            "./resources/test_detection/with_projector_on/without_halo/4.png",
//            "./resources/test_detection/with_projector_on/without_halo/5.png",
//            "./resources/test_detection/with_projector_on/without_halo/6.png",
//            "./resources/test_detection/with_projector_on/without_halo/7.png",
//            "./resources/test_detection/with_projector_on/without_halo/8.png",
//            "./resources/test_detection/with_projector_on/without_halo/9.png",
//            "./resources/test_detection/with_projector_on/without_halo/10.png",
//            "./resources/test_detection/with_projector_on/without_halo/11.png",
//            "./resources/test_detection/with_projector_on/without_halo/12.png",
//            "./resources/test_detection/with_projector_on/without_halo/13.png",
//            "./resources/test_detection/with_projector_on/without_halo/14.png",
//            "./resources/test_detection/with_projector_on/without_halo/15.png",
//            "./resources/test_detection/with_projector_on/without_halo/16.png",
//            "./resources/test_detection/with_projector_on/without_halo/17.png",
//            "./resources/test_detection/with_projector_on/without_halo/18.png",
//            "./resources/test_detection/with_projector_on/without_halo/19.png",
//            "./resources/test_detection/with_projector_on/without_halo/20.png",
//            "./resources/test_detection/with_projector_on/without_halo/21.png",
//            "./resources/test_detection/with_projector_on/without_halo/22.png",
//            "./resources/test_detection/with_projector_on/without_halo/23.png",
//            "./resources/test_detection/with_projector_on/without_halo/24.png",
//            "./resources/test_detection/with_projector_on/with_halo/1.png",
//            "./resources/test_detection/with_projector_on/with_halo/2.png",
//            "./resources/test_detection/with_projector_on/with_halo/3.png",
//            "./resources/test_detection/with_projector_on/with_halo/4.png",
//            "./resources/test_detection/with_projector_on/with_halo/5.png",
//            "./resources/test_detection/with_projector_on/with_halo/6.png",
//            "./resources/test_detection/with_projector_on/with_halo/11.png",
//            "./resources/test_detection/with_projector_on/with_halo/12.png",
//            "./resources/test_detection/with_projector_on/with_halo/13.png",
//            "./resources/test_detection/with_projector_on/with_halo/14.png",
//            "./resources/test_detection/with_projector_on/with_halo/15.png",
//            "./resources/test_detection/with_projector_on/with_halo/16.png",
//            "./resources/test_detection/with_projector_on/with_halo/17.png",
//            "./resources/test_detection/with_projector_on/with_halo/18.png",
//            "./resources/test_detection/with_projector_on/with_halo/19.png",
            "./resources/test_detection/with_projector_on/with_halo_2/1.png",
            "./resources/test_detection/with_projector_on/with_halo_2/2.png",
            "./resources/test_detection/with_projector_on/with_halo_2/3.png",
            "./resources/test_detection/with_projector_on/with_halo_2/4.png",
            "./resources/test_detection/with_projector_on/with_halo_2/5.png",
            "./resources/test_detection/with_projector_on/with_halo_2/6.png",
            "./resources/test_detection/with_projector_on/with_halo_2/7.png",
            "./resources/test_detection/with_projector_on/with_halo_2/8.png",
            "./resources/test_detection/with_projector_on/with_halo_2/9.png",
            "./resources/test_detection/with_projector_on/with_halo_2/10.png",
            "./resources/test_detection/with_projector_on/with_halo_2/11.png",
            "./resources/test_detection/with_projector_on/with_halo_2/12.png",
            "./resources/test_detection/with_projector_on/with_halo_2/13.png",
            "./resources/test_detection/with_projector_on/with_halo_2/14.png",
            "./resources/test_detection/with_projector_on/with_halo_2/15.png",
            "./resources/test_detection/with_projector_on/with_halo_2/16.png",
            "./resources/test_detection/with_projector_on/with_halo_2/17.png",
            "./resources/test_detection/with_projector_on/with_halo_2/18.png",
            "./resources/test_detection/with_projector_on/with_halo_2/19.png",
            "./resources/test_detection/with_projector_on/with_halo_2/20.png",
            "./resources/test_detection/with_projector_on/with_halo_2/21.png",
            "./resources/test_detection/with_projector_on/with_halo_2/22.png",
            "./resources/test_detection/with_projector_on/with_halo_2/23.png",
            "./resources/test_detection/with_projector_on/with_halo_2/24.png",
            "./resources/test_detection/with_projector_on/with_halo_2/25.png",
            "./resources/test_detection/with_projector_on/with_halo_2/26.png",
            "./resources/test_detection/with_projector_on/with_halo_2/27.png",
            "./resources/test_detection/with_projector_on/with_halo_2/28.png",
            "./resources/test_detection/with_projector_on/with_halo_2/29.png",
            "./resources/test_detection/with_projector_on/with_halo_2/30.png",
            "./resources/test_detection/with_projector_on/with_halo_2/31.png",
            "./resources/test_detection/with_projector_on/with_halo_2/32.png"
    };

    cv::Size imageSize = getImageSize();
    billiard::detection::Table table = getTable();
    billiard::detection::ArucoMarkers markers = getMarkers();
    billiard::detection::CameraIntrinsics intrinsics = getIntrinsics_realsense_hd();

    unsigned long long imageIndex = 0;
    bool imageChanged = true;
    bool detect = false;
    bool redraw = true;

    std::shared_ptr<billiard::detection::DetectionConfig> detectionConfig;

    std::string frameName = "Image";
    cv::namedWindow(frameName);
    cv::setMouseCallback(frameName, onReferenceDataMouseClick);

    std::string imagePath;
    cv::Mat frame;
    cv::Mat drawn;
    cv::Mat grid;

    while(true) {

        if (imageChanged) {
            imageChanged = false;
            redraw = true;
            selectedBallIndex = -1;
            pixelState = billiard::detection::State{};
            state = billiard::detection::State{};

            imagePath = imagePaths[imageIndex];
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
        }

        if (detect) {
            detect = false;
            selectedBallIndex = -1;

            pixelState = billiard::snooker::detect(billiard::detection::State(), frame);
            billiard::snooker::classify(billiard::detection::State(), pixelState, frame);
            state = billiard::detection::pixelToModelCoordinates(*detectionConfig, pixelState);

            redraw = true;
        }

        if (redraw) {
            redraw = false;

            frame.copyTo(drawn);
            drawBalls(drawn, pixelState._balls);

            grid = drawDetectedBallsGrid(drawn, pixelState, 128, 8);
        }

        cv::imshow(frameName, drawn);
        cv::imshow("Grid", grid);

        char key = (char) cv::waitKey(30);
        if (key == 'q' || key == 27) {
            break;
        } else if (key == 'v') {
            detect = true;
        } else if (key == 'j') {
            if (selectedBallIndex >= 0) {
                pixelState._balls[selectedBallIndex]._position += glm::vec2 { -1, 0 };
                redraw = true;
            }
        } else if (key == 'l') {
            if (selectedBallIndex >= 0) {
                pixelState._balls[selectedBallIndex]._position += glm::vec2 { +1, 0 };
                redraw = true;
            }
        } else if (key == 'k') {
            if (selectedBallIndex >= 0) {
                pixelState._balls[selectedBallIndex]._position += glm::vec2 { 0, +1 };
                redraw = true;
            }
        } else if (key == 'i') {
            if (selectedBallIndex >= 0) {
                pixelState._balls[selectedBallIndex]._position += glm::vec2 { 0, -1 };
                redraw = true;
            }
        } else if (key == 'w') {

            state = billiard::detection::pixelToModelCoordinates(*detectionConfig, pixelState);

            std::cout << "------------------------------------------------------------" << std::endl;
            std::cout << "{" << std::endl;
            std::cout << R"("image": ")" << imagePath << "\"," << std::endl;
            std::cout << "\"balls\": [" << std::endl;
            for (int i = 0; i < state._balls.size(); i++) {
                auto& ball = state._balls[i];
                auto& pixelBall = pixelState._balls[i];
                cv::Point2d modelPoint = cv::Point2d(ball._position.x, ball._position.y);
                cv::Point2d pixelPoint = cv::Point2d(pixelBall._position.x, pixelBall._position.y);
                std::cout << "{ " << std::endl;
                std::cout << R"("type": ")" << ball._type << "\"," << std::endl;
                std::cout << R"("x": )" << modelPoint.x << ", " << R"("y": )" << modelPoint.y << "," << std::endl;
                std::cout << R"("pixelX": )" << pixelPoint.x << ", " << R"("pixelY": )" << pixelPoint.y << "" << std::endl;
                if (i < state._balls.size() - 1) {
                    std::cout << " }," << std::endl;
                } else {
                    std::cout << " }" << std::endl;
                }
            }
            std::cout << "]" << std::endl;
            std::cout << "}" << std::endl;
            std::cout << "------------------------------------------------------------" << std::endl;

        } else if (key == 'r') {

            pixelState._balls.erase(pixelState._balls.begin() + selectedBallIndex);
            selectedBallIndex = -1;
            redraw = true;

        } else if (key == 'n') {

            billiard::detection::Ball ball;
            ball._type = "UNKNOWN";
            ball._position = glm::vec2 {frame.cols/2, frame.rows/2};
            pixelState._balls.push_back(ball);
            selectedBallIndex = pixelState._balls.size() - 1;
            redraw = true;

        } else if (key == 97 /* A */) {
            imageIndex = imageIndex == 0 ? imagePaths.size() - 1 : imageIndex - 1;
            imageChanged = true;
        } else if (key == 100 /* D */) {
            imageIndex = (imageIndex + 1) % (imagePaths.size());
            imageChanged = true;
        }
    }

}

TEST(BallDetectionTests, display_reference_data) {

    bool writeClassificationImages = false;
    std::string classificationImagesOutputFolder = "to_be_classified_balls/";
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
    cv::Mat badDetectionGrid;
    cv::Mat badClassificationGrid;

    std::string frameName = "Image";
    cv::namedWindow(frameName);
    cv::setMouseCallback(frameName, onReferenceDataMouseClick);

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

            billiard::detection::State actualPixelState = billiard::snooker::detect(billiard::detection::State(), frame);
            billiard::snooker::classify(billiard::detection::State(), actualPixelState, frame);
            billiard::detection::State modelState = billiard::detection::pixelToModelCoordinates(*detectionConfig, actualPixelState);

            frame.copyTo(drawn);
            drawBalls(drawn, actualPixelState._balls);
            drawTypes(drawn, actualPixelState._balls);

            billiard::detection::State expectedPixelState = toPixelState(testcase.balls);
            pixelState = expectedPixelState; // For clicking and selecting a ball
            billiard::detection::State expectedState = toState(testcase.balls);

            if (!expectedPixelState._balls.empty()) {
                frame.copyTo(expectedDrawn);
                drawBalls(expectedDrawn, expectedPixelState._balls);
                drawTypes(expectedDrawn, expectedPixelState._balls);
                expectedGrid = drawDetectedBallsGrid(expectedDrawn, expectedPixelState, 128, 8);
            } else {
                expectedGrid = cv::Mat();
            }
            actualGrid = drawDetectedBallsGrid(drawn, actualPixelState, 128, 8);

            DetectionStats stats = compare(testcase, expectedState, modelState);
            std::cout << "Case: " << testcase.image << std::endl
                      << "  Stats: " << stats << std::endl;

            billiard::detection::State badDetections;
            for (int i = 0; i < expectedState._balls.size(); i++) {
                float distance = stats.distances[i];
                if (distance > 3.0f /*mm*/) {
                    int actualIndex = stats.matchedExpectedBalls[i];
                    if (actualIndex >= 0) {
                        badDetections._balls.push_back(actualPixelState._balls[actualIndex]);
                    }
                }
            }
            badDetectionGrid = drawDetectedBallsGrid(drawn, badDetections, 128, 10);

            billiard::detection::State badClassifications;
            for (int i = 0; i < expectedPixelState._balls.size(); i++) {
                auto& expectedBall = expectedPixelState._balls[i];
                int actualIndex = stats.matchedExpectedBalls[i];
                if (actualIndex >= 0) {
                    auto& actualBall = actualPixelState._balls[actualIndex];
                    if (actualBall._type != expectedBall._type) {
                        badClassifications._balls.push_back(actualBall);
                    }
                }
            }
            badClassificationGrid = drawDetectedBallsGrid(drawn, badClassifications, 128, 10);

            if (writeClassificationImages) {
                cv::Mat detectedBallsImage = frame.clone();
                int ballIndex = 0;
                for (auto& ball : expectedPixelState._balls) {

                    cv::Point2d pixelPoint = cv::Point2d(ball._position.x, ball._position.y);

                    int ballRadius = detectionConfig->ballRadiusInPixel;

                    float paddingFactor = 0.5;
                    float radius = (1.0f + paddingFactor) * ballRadius;

                    const cv::Rect& roi = cv::Rect{
                            (int) (pixelPoint.x - radius),
                            (int) (pixelPoint.y - radius),
                            (int) (2 * radius),
                            (int) (2 * radius)
                    };
                    cv::Mat ballImage = frame(roi);

                    cv::putText(detectedBallsImage, std::to_string(ballIndex+1), pixelPoint, cv::FONT_HERSHEY_PLAIN, 1.0, cv::Scalar{255, 0, 0});

                    {
                        std::string filename = classificationImagesOutputFolder + ball._type + "/" + std::to_string(testcaseIndex + 1) + std::string("_") + std::to_string(ballIndex + 1) + std::string(".png");
                        if (writeClassificationImages) cv::imwrite(filename, ballImage);
                        if (!writeClassificationImages) cv::imshow(filename, ballImage);
                    }

                    ballIndex++;
                }
                {
                    std::string filename = classificationImagesOutputFolder + std::string("Balls_") + std::to_string(testcaseIndex + 1) + std::string(".png");
                    if (writeClassificationImages) cv::imwrite(filename, detectedBallsImage);
                    if (!writeClassificationImages) cv::imshow(filename, detectedBallsImage);
                }
            }
        }

        if (selectedBallIndex >= 0) {

            std::cout << "clicked ball at "
                      << pixelState._balls[selectedBallIndex]._position.x << ", " << pixelState._balls[selectedBallIndex]._position.y
                      << std::endl;

            selectedBallIndex = -1;
        }

        if (clickedPixel != cv::Point2i { 0, 0 }) {
            // Print pixel and corresponding model coordinate of clicked point,
            // in order to check how much change a one pixel difference makes in model coordinates.
            billiard::detection::State clickedPixelState;
            int clickedBallIndex = -1;
            for (int dy = -1; dy <= 1; dy++) {
                for (int dx = -1; dx <= 1; dx++) {
                    billiard::detection::Ball pseudoBall;
                    pseudoBall._position.x = clickedPixel.x + dx;
                    pseudoBall._position.y = clickedPixel.y + dy;
                    clickedPixelState._balls.push_back(pseudoBall);
                    if (dx == 0 && dy == 0) {
                        clickedBallIndex = clickedPixelState._balls.size() - 1;
                    }
                }
            }
            billiard::detection::State clickedModelState = billiard::detection::pixelToModelCoordinates(*detectionConfig, clickedPixelState);
            std::cout << "click at: " << "(" << clickedPixel.x << ", " << clickedPixel.y << ")" << std::endl;
            auto& centerModelBall = clickedModelState._balls[clickedBallIndex];
            for (int i = 0; i < clickedModelState._balls.size(); i++) {
                auto& pixelBall = clickedPixelState._balls[i];
                auto& modelBall = clickedModelState._balls[i];
                std::cout << "    "
                          << "pixel: " << "(" << pixelBall._position.x << ", " << pixelBall._position.y << ")" << " "
                          << "model: " << "(" << modelBall._position.x << ", " << modelBall._position.y << ")" << " "
                          << "distance: " << glm::length(centerModelBall._position - modelBall._position)
                          << std::endl;
            }

            clickedPixel = cv::Point2i { 0, 0 };
        }

        cv::imshow(frameName, frame);
        if (!expectedGrid.empty()) cv::imshow("expected grid", expectedGrid);
        cv::imshow("actual grid", actualGrid);
        cv::imshow("bad detections grid", badDetectionGrid);
        cv::imshow("bad classifications grid", badClassificationGrid);

        char key = (char) cv::waitKey(30);

        if (writeClassificationImages) {
            testcaseIndex++;
            imageChanged = true;
            if (testcaseIndex >= testcases.cases.size()) {
                break;
            }
        } else {
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

void onReferenceDataMouseClick(int event, int x, int y, int flags, void* userdata) {

    if (event != cv::EVENT_LBUTTONDOWN) {
        return;
    }

    clickedPixel = cv::Point2i {x, y};
    selectedBallIndex = -1;

    for (int i = 0; i < pixelState._balls.size(); i++) {
        auto& ball = pixelState._balls[i];
        if (glm::length(ball._position - glm::vec2 { x, y }) < 10) {
            selectedBallIndex = i;
        }
    }

}

