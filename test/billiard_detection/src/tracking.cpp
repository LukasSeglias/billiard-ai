#include <gtest/gtest.h>
#include <billiard_detection/billiard_detection.hpp>
#include <billiard_snooker/billiard_snooker.hpp>
#include "config.hpp"
#include "util.hpp"

TEST(DetectionTest, tracking) {

    bool paused = false;
    std::string videoPath = "./resources/spiel_mit_infinity_mode.avi";

    cv::Size imageSize = getImageSize();
    billiard::detection::Table table = getTable();
    billiard::detection::ArucoMarkers markers = getMarkers();
    billiard::detection::CameraIntrinsics intrinsics = getIntrinsics_realsense_hd();

    std::shared_ptr<billiard::detection::DetectionConfig> detectionConfig;

    cv::VideoCapture video = cv::VideoCapture {videoPath};
    if (!video.isOpened()) {
        std::cout << "Unable to open video" << std::endl;
        return;
    }

    cv::Mat frame;
    video.read(frame);

    detectionConfig = std::make_shared<billiard::detection::DetectionConfig>(billiard::detection::configure(frame, table, markers, intrinsics));
    if (!detectionConfig->valid) {
        std::cout << "Unable to configure detection" << std::endl;
        return;
    }

    if (!billiard::snooker::configure(*detectionConfig)) {
        std::cout << "Unable to configure snooker detection" << std::endl;
        return;
    }

    bool finished = false;

    auto capture = [&video, &finished]() {
        cv::Mat frame;
        video.read(frame);
        finished = frame.empty();
        return billiard::capture::CameraFrames { frame, cv::Mat(), cv::Mat() };
    };
    auto classify = [](const billiard::detection::State& previousState,
                       billiard::detection::State& currentState,
                       const cv::Mat& image) {
        billiard::snooker::classify(previousState, currentState, image);
    };
    billiard::detection::StateTracker stateTracker(capture, detectionConfig, billiard::snooker::detect, classify);

    cv::Mat drawn;
    cv::Mat grid;

    while (!finished) {

        if (!paused) {
            billiard::detection::State modelState;

            auto future = stateTracker.capture();
            future.wait();
            modelState = future.get();
        }

        char key = (char) cv::waitKey(5);
        if (key == ' ') {
            paused = !paused;
        } else if (key == 27 /* ESC */) {
            break;
        }
    }

}
