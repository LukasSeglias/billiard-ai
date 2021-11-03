#include <gtest/gtest.h>
#include <billiard_capture/billiard_capture.hpp>
#include <iostream>

TEST(ImageCapture, open_read_close) {
    // Assumption: We assume here that the MAC-address of the camera is known.
    // If this is not the case, the MAC-address can be found by using the eBUS-Player found in the eBUS SDK.
    std::string mac = "00:11:1c:f5:a0:f4";

    billiard::capture::ImageCapture capture {};
    if (capture.open(mac)) {

        cv::Mat frame = capture.read();
        cv::imshow("Frame", frame);
        while (true) {

            int key = cv::waitKey(50);
            if (key != -1) std::cout << "key: " << key << "pressed" << std::endl;
            if (key == 27 /* ESC */) break;
//            if (key == 13 /* Enter */) {
                frame = capture.read();

                frame *= 5;
                std::cout << "Frame: " << frame.cols << " x " << frame.rows << " channels: " << frame.channels() << std::endl;

                cv::imshow("Frame", frame);
//            }
        }
    }
    capture.close();
}

TEST(CameraCapture, open_read_close) {

    billiard::capture::CameraCapture capture {};

    if (capture.open()) {

        cv::namedWindow("Color", cv::WINDOW_AUTOSIZE);
        cv::namedWindow("Depth", cv::WINDOW_AUTOSIZE);

        while (cv::waitKey(1) < 0) {
            billiard::capture::CameraFrames frames = capture.read();

            if (!frames.depth.empty()) {
                cv::imshow("Depth", frames.colorizedDepth);
            }
            cv::imshow("Color", frames.color);
        }
        capture.close();
    }
}

TEST(CameraCapture, record_video) {

    // TODO: currently have to copy
    //       D:\Dev\billiard-ai\cmake-build-release\ext\opencv\opencv\x64\vc16\bin\opencv_videoio_ffmpeg452_64.dll
    //       into the directory where this test runs, since this dll is not copied

    billiard::capture::CameraCapture capture {};

    if (capture.open()) {

        while (true) {
            billiard::capture::CameraFrames frames = capture.read();

            if (!frames.depth.empty()) {
                cv::imshow("Depth", frames.colorizedDepth);
            }
            if (!frames.color.empty()) {
                cv::imshow("Color", frames.color);
            }

            char key = (char) cv::waitKey(1);
            if (key == 'q' || key == 27 /* ESC */) {
                break;
            }
            if (key == 'r') {
                capture.toggleRecording();
            }
        }
    }
}
