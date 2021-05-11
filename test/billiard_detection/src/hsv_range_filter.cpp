#include <gtest/gtest.h>
#include <opencv2/opencv.hpp>

const int max_value_H = 360/2;
const int max_value = 255;
const std::string result_window_name = "Filter";
int low_H1 = 0, low_H2, low_S = 0, low_V = 0;
int high_H1 = max_value_H, high_H2 = max_value_H, high_S = max_value, high_V = max_value;

static void on_low_H1_thresh_trackbar(int, void *) {
    low_H1 = cv::min(high_H1-1, low_H1);
    cv::setTrackbarPos("Low H 1", result_window_name, low_H1);
}
static void on_high_H1_thresh_trackbar(int, void *) {
    high_H1 = cv::max(high_H1, low_H1+1);
    cv::setTrackbarPos("High H 1", result_window_name, high_H1);
}
static void on_low_H2_thresh_trackbar(int, void *) {
    low_H2 = cv::min(high_H2-1, low_H2);
    cv::setTrackbarPos("Low H 2", result_window_name, low_H2);
}
static void on_high_H2_thresh_trackbar(int, void *) {
    high_H2 = cv::max(high_H2, low_H2+1);
    cv::setTrackbarPos("High H 2", result_window_name, high_H2);
}
static void on_low_S_thresh_trackbar(int, void *) {
    low_S = cv::min(high_S-1, low_S);
    cv::setTrackbarPos("Low S", result_window_name, low_S);
}
static void on_high_S_thresh_trackbar(int, void *) {
    high_S = cv::max(high_S, low_S+1);
    cv::setTrackbarPos("High S", result_window_name, high_S);
}
static void on_low_V_thresh_trackbar(int, void *) {
    low_V = cv::min(high_V-1, low_V);
    cv::setTrackbarPos("Low V", result_window_name, low_V);
}
static void on_high_V_thresh_trackbar(int, void *) {
    high_V = cv::max(high_V, low_V+1);
    cv::setTrackbarPos("High V", result_window_name, high_V);
}

TEST(BallDetectionTests, hsvRangeFilter) {

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

    cv::namedWindow(result_window_name, cv::WINDOW_NORMAL);
    cv::createTrackbar("Low H 1", result_window_name, &low_H1, max_value_H, on_low_H1_thresh_trackbar);
    cv::createTrackbar("High H 1", result_window_name, &high_H1, max_value_H, on_high_H1_thresh_trackbar);
    cv::createTrackbar("Low H 2", result_window_name, &low_H2, max_value_H, on_low_H2_thresh_trackbar);
    cv::createTrackbar("High H 2", result_window_name, &high_H2, max_value_H, on_high_H2_thresh_trackbar);
    cv::createTrackbar("Low S", result_window_name, &low_S, max_value, on_low_S_thresh_trackbar);
    cv::createTrackbar("High S", result_window_name, &high_S, max_value, on_high_S_thresh_trackbar);
    cv::createTrackbar("Low V", result_window_name, &low_V, max_value, on_low_V_thresh_trackbar);
    cv::createTrackbar("High V", result_window_name, &high_V, max_value, on_high_V_thresh_trackbar);

    unsigned long long imageIndex = 0;
    bool imageChanged = true;

    cv::Mat input;
    cv::Mat grayScaleImage;
    cv::Mat hsv;
    cv::Mat hue, saturation, value;

    float scale = 0.5;

    while(true) {
        std::string imagePath = imagePaths[imageIndex];

        if (imageChanged) {
            imageChanged = false;
            auto original = imread(imagePath, cv::IMREAD_COLOR);
            cv::resize(original, input, cv::Size(), scale, scale);

            cvtColor(input, grayScaleImage, cv::COLOR_BGR2GRAY);
            cv::medianBlur(grayScaleImage, grayScaleImage, 5);

            cv::cvtColor(input, hsv, cv::COLOR_BGR2HSV);
            std::vector<cv::Mat> channels;
            cv::split(hsv, channels);
            hue = channels[0];
            saturation = channels[1];
            value = channels[2];
        }

        cv::imshow("hue", hue);
        cv::imshow("saturation", saturation);
        cv::imshow("value", value);
        cv::imshow("input", input);

        cv::Mat hueMask1;
        cv::inRange(hue, low_H1, high_H1, hueMask1);

        cv::Mat hueMask2;
        cv::inRange(hue, low_H2, high_H2, hueMask2);

        cv::Mat totalHueMask;
        cv::bitwise_or(hueMask1, hueMask2, totalHueMask);

        cv::Mat svMask;
        cv::inRange(hsv, cv::Scalar(0, low_S, low_V), cv::Scalar(max_value_H, high_S, high_V), svMask);

        cv::Mat mask;
        cv::bitwise_and(totalHueMask, svMask, mask);

        cv::Mat result;
        cv::bitwise_and(input, input, result, mask);
        cv::imshow("result", result);
        cv::imshow("mask", mask);

        char key = (char) cv::waitKey(30);
        if (key == 'q' || key == 27) {
            break;
        }
        if (key == 97 /* A */) {
            imageIndex = imageIndex == 0 ? imagePaths.size() - 1 : imageIndex - 1;
            imageChanged = true;
        }
        if (key == 100 /* D */) {
            imageIndex = (imageIndex + 1) % (imagePaths.size());
            imageChanged = true;
        }
    }
}
