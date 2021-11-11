#include <gtest/gtest.h>
#include <opencv2/opencv.hpp>
#include <iostream>
#include <billiard_detection/billiard_detection.hpp>
#include <billiard_snooker/billiard_snooker.hpp>
#include "config.hpp"
#include "util.hpp"
#include <chrono>

void onImageMouseClick(int event, int x, int y, int flags, void* userdata);

billiard::detection::State pixelState;
billiard::detection::State state;
int selectedBallIndex = -1;

TEST(BallDetectionTests, record_reference_data) {

    std::vector<std::string> imagePaths = {
            "./resources/test_detection/with_projector_on/without_halo/1.png",
            "./resources/test_detection/with_projector_on/without_halo/2.png",
            "./resources/test_detection/with_projector_on/without_halo/3.png",
            "./resources/test_detection/with_projector_on/without_halo/4.png",
            "./resources/test_detection/with_projector_on/without_halo/5.png",
            "./resources/test_detection/with_projector_on/without_halo/6.png",
            "./resources/test_detection/with_projector_on/without_halo/7.png",
            "./resources/test_detection/with_projector_on/without_halo/8.png",
            "./resources/test_detection/with_projector_on/without_halo/9.png",
            "./resources/test_detection/with_projector_on/without_halo/10.png",
            "./resources/test_detection/with_projector_on/without_halo/11.png",
            "./resources/test_detection/with_projector_on/without_halo/12.png",
            "./resources/test_detection/with_projector_on/without_halo/13.png",
            "./resources/test_detection/with_projector_on/without_halo/14.png",
            "./resources/test_detection/with_projector_on/without_halo/15.png",
            "./resources/test_detection/with_projector_on/without_halo/16.png",
            "./resources/test_detection/with_projector_on/without_halo/17.png",
            "./resources/test_detection/with_projector_on/without_halo/18.png",
            "./resources/test_detection/with_projector_on/without_halo/19.png",
            "./resources/test_detection/with_projector_on/without_halo/20.png",
            "./resources/test_detection/with_projector_on/without_halo/21.png",
            "./resources/test_detection/with_projector_on/without_halo/22.png",
            "./resources/test_detection/with_projector_on/without_halo/23.png",
            "./resources/test_detection/with_projector_on/without_halo/24.png",
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
//            "./resources/test_detection/with_projector_on/with_halo/19.png"
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
    cv::setMouseCallback(frameName, onImageMouseClick);

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

void onImageMouseClick(int event, int x, int y, int flags, void* userdata) {

    if (event != cv::EVENT_LBUTTONDOWN) {
        return;
    }

    selectedBallIndex = -1;

    for (int i = 0; i < pixelState._balls.size(); i++) {
        auto& ball = pixelState._balls[i];
        if (glm::length(ball._position - glm::vec2 { x, y }) < 10) {
            selectedBallIndex = i;
        }
    }

}

