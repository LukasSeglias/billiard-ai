#include <gtest/gtest.h>
#include <billiard_detection/billiard_detection.hpp>
#include <billiard_snooker/billiard_snooker.hpp>
#include "config.hpp"
#include "util.hpp"

TEST(DetectionTest, detection_errors_over_time) {

    bool autoplay = false;
    std::string testcasePath = "./resources/video_detection/testcases.json";
    DetectionTestCases testcases = loadDetectionTestCases(testcasePath);

    cv::Size imageSize = getImageSize();
    billiard::detection::Table table = getTable();
    billiard::detection::ArucoMarkers markers = getMarkers();
    billiard::detection::CameraIntrinsics intrinsics = getIntrinsics_realsense_hd();

    std::shared_ptr<billiard::detection::DetectionConfig> detectionConfig;

    cv::VideoCapture video;

    std::vector<cv::Mat> frames {};
    int frameIndex = 0;
    bool imageChanged = true;

    DetectionTestCase testcase;
    int testcaseIndex = 0;
    bool testcaseChanged = true;

    cv::Mat frame;
    cv::Mat drawn;
    cv::Mat grid;
    cv::Mat classificationErrorsGrid;

    while(true) {

        if (testcaseChanged) {
            testcaseChanged = false;
            frames.clear();

            testcase = testcases.cases[testcaseIndex];
            std::string videoPath = testcase.image;

            std::cout << "Testcase " << std::to_string(testcaseIndex) << ": " << videoPath << std::endl;

            video = cv::VideoCapture {videoPath};
            if (!video.isOpened()) {
                std::cout << "Unable to open video" << std::endl;
                return;
            }

            if (video.read(frame)) {
                frames.push_back(frame);
            }

            detectionConfig = std::make_shared<billiard::detection::DetectionConfig>(billiard::detection::configure(frames.at(0), table, markers, intrinsics));
            if (!detectionConfig->valid) {
                std::cout << "Unable to configure detection" << std::endl;
                return;
            }

            if (!billiard::snooker::configure(*detectionConfig)) {
                std::cout << "Unable to configure snooker detection" << std::endl;
                return;
            }
        }

        if (imageChanged) {
            imageChanged = false;

            std::cout << "frame " << std::to_string(frameIndex) << std::endl;
            frame = frames.at(frameIndex);
            cv::resize(frame, frame, imageSize);

            billiard::detection::State pixelState = billiard::snooker::detect(billiard::detection::State(), frame);
            billiard::snooker::classify(billiard::detection::State(), pixelState, frame);
            billiard::detection::State state = billiard::detection::pixelToModelCoordinates(*detectionConfig, pixelState);

            frame.copyTo(drawn);
            drawBalls(drawn, pixelState._balls);
            drawTypes(drawn, pixelState._balls);

            billiard::detection::State expected = toState(testcase.balls);
            DetectionStats stats = compare(testcase, expected, state);

            billiard::detection::State classificationErrors;
            for (int i = 0; i < stats.wrongType.size(); i++) {
                int actualIndex = stats.wrongType[i];
                auto& ball = pixelState._balls[actualIndex];
                std::cout << "Frame " << frameIndex << ": Ball " << ball._id << " was classified incorrectly as " << ball._type << std::endl;
                classificationErrors._balls.push_back(ball);

//                std::string imageName = "classification_error_" + std::to_string(frameIndex) + ".png";
//                cv::imwrite(imageName, frame);
            }
            classificationErrorsGrid = drawDetectedBallsGrid(drawn, classificationErrors, 128, 8);

            // Sort by distance to origin to have balls at same positions in grid
            std::sort(pixelState._balls.begin(), pixelState._balls.end(), [](const billiard::detection::Ball& ball1, const billiard::detection::Ball& ball2) {
                float dist1 = glm::length(ball1._position);
                float dist2 = glm::length(ball2._position);
                return dist1 < dist2;
            });
            grid = drawDetectedBallsGrid(drawn, pixelState, 128, 8);

        }

//        cv::imshow("Frame", frame);
//        cv::imshow("drawn", drawn);
        cv::imshow("grid", grid);
        cv::imshow("classification errors", classificationErrorsGrid);

        if (autoplay) {
            frameIndex++;
            imageChanged = true;
            cv::Mat temp;
            if (video.read(temp)) {
                frames.push_back(temp);
            }
            if (frameIndex >= frames.size()) {
                testcaseIndex++;
            }
            if (testcaseIndex >= testcases.cases.size()) {
                return;
            }
        } else {
            char key = (char) cv::waitKey(1);
            if (key == 27 /* ESC */) {
                break;
            } else if (key == 97 /* A */) {
                frameIndex = frameIndex == 0 ? 0 : frameIndex - 1;
                imageChanged = true;
            } else if (key == 100 /* D */) {
                frameIndex++;
                if (frameIndex >= frames.size()) {
                    cv::Mat temp;
                    if (video.read(temp)) {
                        frames.push_back(temp);
                    } else {
                        frameIndex = 0;
                    }
                }
                imageChanged = true;
//            } else if (key == 'q' /* Q */) {
//                frameIndex = frameIndex >= 10 ? frameIndex - 10 : frames.size() - 1;
//                imageChanged = true;
//            } else if (key == 'e' /* E */) {
//                frameIndex = (frameIndex + 10) % (frames.size());
//                imageChanged = true;
            } else if (key == ' ') {
                std::string imageName = "video_detection_" + std::to_string(frameIndex) + ".png";
                cv::imwrite(imageName, frame);
            } else if (key == 'n') {
                testcaseIndex = (testcaseIndex + 1) % (testcases.cases.size());
                frameIndex = 0;
                testcaseChanged = true;
                imageChanged = true;
            }
        }
    }

}
