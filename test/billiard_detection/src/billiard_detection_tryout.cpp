#include <gtest/gtest.h>
#include <opencv2/opencv.hpp>
#include <iostream>
#include <billiard_detection/billiard_detection.hpp>
#include <billiard_snooker/billiard_snooker.hpp>
#include <billiard_capture/billiard_capture.hpp>
#include <librealsense2/rs.hpp>
#include "config.hpp"
#include <chrono>


TEST(BallDetectionTests, snooker_with_projector) {

    int a = 0; // TODO: remove
    bool live = false;
    std::vector<std::string> imagePaths = {
//            "./resources/test_detection_with_projector/1.png",
//            "./resources/test_detection_with_projector/2.png",
//            // DONE: TODO 3: green: hole in saturation-mask
//            "./resources/test_detection_with_projector/3.png",
//            "./resources/test_detection_with_projector/4.png",
//            "./resources/test_detection_with_projector/5.png",
//            "./resources/test_detection_with_projector/6.png",
//            // DONE: TODO 7: black: because of pocket-mask?
//            // TODO 7: red: why?
//            "./resources/test_detection_with_projector/7.png",
//            // DONE: TODO 8: brown: killed by filter 2 (saturation-mask)?
//            // DONE: TODO 8: green: killed by filter 2 (saturation-mask)?
//            // DONE: TODO 8: black: killed by filter 2 (hole in mask)?
//            "./resources/test_detection_with_projector/8.png",
//            "./resources/test_detection_with_projector/9.png",
//            "./resources/test_detection_with_projector/10.png",
//            "./resources/test_detection_with_projector/11.png",
//            "./resources/test_detection_with_projector/12.png",
//            "./resources/test_detection_with_projector/13.png",
//            "./resources/test_detection_with_projector/14.png",
//            "./resources/test_detection_with_projector/15.png",
//            "./resources/test_detection_with_projector/16.png",
//            "./resources/test_detection_with_projector/17.png",
//            "./resources/test_detection_with_projector/18.png",
//            "./resources/test_detection_with_projector/19.png",
//            "./resources/test_detection_with_projector/20.png",
//            "./resources/test_detection_with_projector/21.png",
//            "./resources/test_detection_with_projector/22.png",
//            "./resources/test_detection_with_projector/23.png",
//            // TODO 24: statt zwei roten wird eine "grosse" erkannt
//            "./resources/test_detection_with_projector/24.png",
            "./resources/test_detection_with_projector_with_live_texts/1.png",
            "./resources/test_detection_with_projector_with_live_texts/2.png",
            // TODO 3: red ball in top-left corner is detected poorly
            "./resources/test_detection_with_projector_with_live_texts/3.png",
            "./resources/test_detection_with_projector_with_live_texts/4.png",
            "./resources/test_detection_with_projector_with_live_texts/5.png",
            "./resources/test_detection_with_projector_with_live_texts/6.png",
            "./resources/test_detection_with_projector_with_live_texts/7.png",
            "./resources/test_detection_with_projector_with_live_texts/8.png",
            "./resources/test_detection_with_projector_with_live_texts/9.png",
            "./resources/test_detection_with_projector_with_live_texts/10.png",
            "./resources/test_detection_with_projector_with_live_texts/11.png",
            // TODO 12: extra black circle near blue ball in top-right corner
            "./resources/test_detection_with_projector_with_live_texts/12.png",
            "./resources/test_detection_with_projector_with_live_texts/13.png",
            "./resources/test_detection_with_projector_with_live_texts/14.png",
            "./resources/test_detection_with_projector_with_live_texts/15.png",
            "./resources/test_detection_with_projector_with_live_texts/16.png",
            "./resources/test_detection_with_projector_with_live_texts/17.png",
            "./resources/test_detection_with_projector_with_live_texts/18.png",
            "./resources/test_detection_with_projector_with_live_texts/19.png",
            "./resources/test_detection_with_projector_with_live_texts/20.png",
            "./resources/test_detection_with_projector_with_live_texts/21.png",
            "./resources/test_detection_with_projector_with_live_texts/22.png",
            "./resources/test_detection_with_projector_with_live_texts/23.png",
    };

    std::vector<int> expectedBallCounts = {
        22,
        22,
        22,
        22,
        22,
        22,
        22,
        22,
        22,
        22,
        22,
        22,
        22,
        22,
        22,
        22,
        22,
        22,
        22,
        22,
        22,
        22,
        22,
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

            auto t1 = std::chrono::high_resolution_clock::now();
            billiard::detection::State pixelState = billiard::snooker::detect(billiard::detection::State(), frame);
            billiard::detection::State state = billiard::detection::pixelToModelCoordinates(*detectionConfig, pixelState);
            auto t2 = std::chrono::high_resolution_clock::now();

            std::chrono::duration<double, std::milli> ms_double = t2 - t1;
            std::cout << "Took " << ms_double.count() << "ms" << std::endl;

            for (auto& ball : state._balls) {

                cv::Point2d modelPoint = cv::Point2d(ball._position.x, ball._position.y);
                std::cout << "model point: " << modelPoint << std::endl;
            }

            if (!live) {
                std::string expectedBallCountStr = imageIndex < expectedBallCounts.size() ? std::to_string(expectedBallCounts[imageIndex]) : "";
                std::cout << "" << std::to_string(state._balls.size()) << "/" << expectedBallCountStr << " balls detected" << std::endl;
            }
        }

#ifdef NDEBUG
        if (!live) {
            imageIndex++;
            imageChanged = true;
            if (imageIndex == imagePaths.size()) {
                return;
            }
        }
#endif

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

            auto t1 = std::chrono::high_resolution_clock::now();
            billiard::detection::State pixelState = billiard::snooker::detect(billiard::detection::State(), frame);
            billiard::detection::State state = billiard::detection::pixelToModelCoordinates(*detectionConfig, pixelState);
            auto t2 = std::chrono::high_resolution_clock::now();

            std::chrono::duration<double, std::milli> ms_double = t2 - t1;
            std::cout << "Took " << ms_double.count() << "ms" << std::endl;

            for (auto& ball : state._balls) {

                cv::Point2d modelPoint = cv::Point2d(ball._position.x, ball._position.y);
                std::cout << "model point: " << modelPoint << std::endl;
            }

            if (!live) {
                std::cout << "" << std::to_string(state._balls.size()) << "/" << std::to_string(expectedBallCounts[imageIndex]) << " balls detected" << std::endl;
            }
        }

#ifdef NDEBUG
        imageIndex++;
        imageChanged = true;
        if (imageIndex == imagePaths.size()) {
            return;
        }
#endif

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

TEST(BallDetectionTests, snooker_performance) {

    const int NUMBER_OF_ROUNDS = 50;
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

    cv::Size imageSize = getImageSize();
    billiard::detection::Table table = getTable();
    billiard::detection::ArucoMarkers markers = getMarkers();
    billiard::detection::CameraIntrinsics intrinsics = getIntrinsics_realsense_hd();

    std::shared_ptr<billiard::detection::DetectionConfig> detectionConfig;

    double totalTimeMs = 0.0;
    int totalRuns = 0;

    for(int i = 0; i < NUMBER_OF_ROUNDS; i++) {
        for(auto& imagePath : imagePaths) {
            cv::Mat frame = imread(imagePath, cv::IMREAD_COLOR);
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

            auto t1 = std::chrono::high_resolution_clock::now();
            billiard::detection::State pixelState = billiard::snooker::detect(billiard::detection::State(), frame);
            billiard::detection::State state = billiard::detection::pixelToModelCoordinates(*detectionConfig, pixelState);
            auto t2 = std::chrono::high_resolution_clock::now();

            std::chrono::duration<double, std::milli> ms_double = t2 - t1;
//            std::cout << "Took " << ms_double.count() << "ms" << std::endl;

            totalTimeMs += ms_double.count();
            totalRuns++;
        }
    }

    std::cout << "--------------------------------------" << std::endl;
    std::cout << "Average time: " << (totalTimeMs/totalRuns) << "ms" << std::endl;
}

TEST(BallDetectionTests, snooker_write_detected_images) {

    bool writeImages = false;
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

    cv::Size imageSize = getImageSize();
    billiard::detection::Table table = getTable();
    billiard::detection::ArucoMarkers markers = getMarkers();
    billiard::detection::CameraIntrinsics intrinsics = getIntrinsics_realsense_hd();

    std::shared_ptr<billiard::detection::DetectionConfig> detectionConfig;

    for (int i = 0; i < imagePaths.size(); i++) {
        std::string imagePath = imagePaths[i];
        cv::Mat frame = imread(imagePath, cv::IMREAD_COLOR);
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

        for (auto& ball : pixelState._balls) {

            cv::Point2d pixelPoint = cv::Point2d(ball._position.x, ball._position.y);
            cv::Point integerPixelPoint{(int)pixelPoint.x, (int)pixelPoint.y};

            int ballRadius = detectionConfig->ballRadiusInPixel;

            cv::Mat frameCopy = frame.clone();
            cv::circle(frameCopy, integerPixelPoint, 1, cv::Scalar(0, 100, 100), 1, cv::LINE_AA);
            cv::circle(frameCopy, integerPixelPoint, ballRadius, cv::Scalar(0, 100, 100), 1, cv::LINE_AA);

            float paddingFactor = 0.5;
            float radius = (1.0f + paddingFactor) * ballRadius;

            cv::Mat ballImage = frameCopy(cv::Rect{
                    (int) (pixelPoint.x - radius),
                    (int) (pixelPoint.y - radius),
                    (int) (2 * radius),
                    (int) (2 * radius)
            });

            cv::Mat ballImageEnlarged;
            float scale = 16.0;
            cv::resize(ballImage, ballImageEnlarged, cv::Size(), scale, scale, cv::INTER_CUBIC);

            {
                std::string filename = std::string("detected_balls/Image_") + std::to_string(i) + std::string("_ball_") + std::to_string(integerPixelPoint.x) + std::string("_") + std::to_string(integerPixelPoint.y) + std::string("_original.png");
                if (writeImages) cv::imwrite(filename, ballImage);
                if (!writeImages) cv::imshow(filename, ballImage);
            }
            {
                std::string filename = std::string("detected_balls/Image_") + std::to_string(i) + std::string("_ball_") + std::to_string(integerPixelPoint.x) + std::string("_") + std::to_string(integerPixelPoint.y) + std::string("_larger.png");
                if (writeImages) cv::imwrite(filename, ballImageEnlarged);
                if (!writeImages) cv::imshow(filename, ballImageEnlarged);
            }
        }
        if (!writeImages) cv::waitKey();
    }
}
