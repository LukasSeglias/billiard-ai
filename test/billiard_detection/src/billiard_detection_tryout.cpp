#include <gtest/gtest.h>
#include <opencv2/opencv.hpp>
#include <iostream>
#include <billiard_detection/billiard_detection.hpp>
#include <billiard_snooker/billiard_snooker.hpp>
#include <billiard_capture/billiard_capture.hpp>
#include <librealsense2/rs.hpp>
#include "config.hpp"

TEST(BallDetectionTests, snooker) {

    bool live = false;
    std::vector<std::string> imagePaths = {
            "./resources/test_detection/1.png",
            "./resources/test_detection/2.png",
            "./resources/test_detection/3.png",
            "./resources/test_detection/4.png",
            "./resources/test_detection/5.png",
            "./resources/test_detection/6.png",
            "./resources/test_detection/7.png",
            "./resources/test_detection/8.png",
            "./resources/test_detection/9.png",
            "./resources/test_detection/10.png",
            "./resources/test_detection/11.png",
            "./resources/test_detection/12.png",
            "./resources/test_detection/13.png",
            "./resources/test_detection/14.png",
            "./resources/test_detection/15.png",
            "./resources/test_detection/16.png",
            "./resources/test_detection/17.png",
            "./resources/test_detection/18.png",
            "./resources/test_detection/19.png",
            "./resources/test_detection/20.png",
    };

    std::vector<int> expectedBallCounts = {
            20,
            20,
            20,
            20,
            20,
            20,
            20,
            20,
            20,
            20,
            20,
            20,
            20,
            22,
            22,
            22,
            21,
            20,
            19,
            17
    };

    cv::Size imageSize = getImageSize();
    billiard::detection::Table table = getTable();
    billiard::detection::ArucoMarkers markers = getMarkers();
    billiard::detection::CameraIntrinsics intrinsics = getIntrinsics_realsense_hd();

    unsigned long long imageIndex = 0;
    bool imageChanged = true;

    billiard::capture::CameraCapture capture {};
    if (live) {
        if (!capture.open()) {
            std::cerr << "Unable to open image stream" << std::endl;
            return;
        }
    }

    std::shared_ptr<billiard::detection::DetectionConfig> detectionConfig;

    while(true) {
        cv::Mat frame;

        if (live) {
            if (imageChanged) {
                billiard::capture::CameraFrames frames = capture.read();
                frames.color.copyTo(frame);
            }
        } else {
            std::string imagePath = imagePaths[imageIndex];
            frame = imread(imagePath, cv::IMREAD_COLOR);
            cv::resize(frame, frame, imageSize);
        }

        if (imageChanged) {
            imageChanged = false;

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

            for (auto& ball : state._balls) {

                cv::Point2d modelPoint = cv::Point2d(ball._position.x, ball._position.y);
                std::cout << "model point: " << modelPoint << std::endl;
            }
        }

        char key = (char) cv::waitKey(30);
        if (key == 'q' || key == 27) {
            break;
        } else if (key == 'l') {
            imageChanged = true;
        } else if (key == 97 /* A */) {
            imageIndex = imageIndex == 0 ? imagePaths.size() - 1 : imageIndex - 1;
            imageChanged = true;
        } else if (key == 100 /* D */) {
            imageIndex = (imageIndex + 1) % (imagePaths.size());
            imageChanged = true;
        }
    }

}
