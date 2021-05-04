#include <gtest/gtest.h>
#include <opencv2/opencv.hpp>

TEST(Webcam, live) {
    cv::Mat frame;
    cv::VideoCapture cap;
    cap.open(2);

    if (!cap.isOpened()) {
        std::cerr << "ERROR! Unable to open camera\n";
    }

    std::cout << "Start grabbing" << std::endl
              << "Press any key to terminate" << std::endl;

    while (true) {
        cap.read(frame);

        if (frame.empty()) {
            std::cerr << "ERROR! blank frame grabbed\n";
            break;
        }

        cv::imshow("Live", frame);
        if (cv::waitKey(5) >= 0) break;
    }
}

TEST(Webcam, screenshot) {

    cv::VideoCapture capture(0);
    capture.set(cv::CAP_PROP_FRAME_WIDTH, 1280);
    capture.set(cv::CAP_PROP_FRAME_HEIGHT, 720);
    capture.set(cv::CAP_PROP_FPS, 30);
    if (!capture.isOpened()) {
        std::cerr << "ERROR! Unable to open camera\n";
    }

    int screenshotNumber = 1;
    cv::Mat frame;
    while (capture.read(frame)) {
        if (frame.empty()) {
            std::cerr << "ERROR! blank frame grabbed\n";
            break;
        }

        imshow("Live", frame);

        int key = cv::waitKey(5);
        if (key == 27 /* ESC */) {
            break;
        } else if(key == 32 /* SPACE */) {
            imwrite("Screenshot-" + std::to_string(screenshotNumber++) + ".png", frame);
        }
    }
}
