#include <opencv2/opencv.hpp>
#include <iostream>
#include <billiard_detection/billiard_detection.hpp>

void drawBalls(cv::Mat& image, std::vector<billiard::detection::Ball>& balls);
cv::Mat drawDetectedBallsGrid(const cv::Mat& input, const billiard::detection::State& pixelState, int tileSize, int tilesPerLine);
