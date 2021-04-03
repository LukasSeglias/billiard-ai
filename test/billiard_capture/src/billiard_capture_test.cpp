#include <gtest/gtest.h>
#include <billiard_capture/billiard_capture.hpp>
#include <librealsense2/rs.hpp>
#include <librealsense2/hpp/rs_internal.hpp>
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

TEST(test, test) {
    cv::Mat frame;
    cv::VideoCapture cap;
    cap.open(2);

    if (!cap.isOpened()) {
        std::cerr << "ERROR! Unable to open camera\n";
    }

    std::cout << "Start grabbing" << std::endl
         << "Press any key to terminate" << std::endl;

    while (true)
    {
        cap.read(frame);

        if (frame.empty()) {
            std::cerr << "ERROR! blank frame grabbed\n";
            break;
        }

        cv::imshow("Live", frame);
        if (cv::waitKey(5) >= 0)
            break;
    }
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

cv::Mat toMat(const rs2::frame& frame, int type) {
    const int depthWidth = frame.as<rs2::video_frame>().get_width();
    const int depthHeight = frame.as<rs2::video_frame>().get_height();
    return cv::Mat(cv::Size(depthWidth, depthHeight), type, (void*)frame.get_data(), cv::Mat::AUTO_STEP);
}

TEST(Realsense, tryout) {

    rs2::context ctx;
    std::cout << "librealsense " << RS2_API_VERSION_STR << std::endl;
    std::cout << "RealSense devices connected: " << ctx.query_devices().size() << std::endl;

    rs2::config cfg;
    cfg.enable_stream(RS2_STREAM_COLOR, 1920, 1080, RS2_FORMAT_BGR8, 30);
//    cfg.enable_stream(RS2_STREAM_INFRARED, 1280, 720, RS2_FORMAT_Y8, 30);
    cfg.enable_stream(RS2_STREAM_DEPTH, 848, 480, RS2_FORMAT_Z16, 30);

    rs2::pipeline pipe;
    pipe.start(cfg);

    cv::namedWindow("Depth", cv::WINDOW_AUTOSIZE);
    cv::namedWindow("Color", cv::WINDOW_AUTOSIZE);
//    cv::namedWindow("Infrared", cv::WINDOW_AUTOSIZE);

    while (cv::waitKey(1) < 0) {
        rs2::frameset data = pipe.wait_for_frames(); // Wait for next set of frames from the camera

        rs2::colorizer color_map;
        cv::Mat depth = toMat(data.get_depth_frame().apply_filter(color_map), CV_8UC3);
        cv::Mat color = toMat(data.get_color_frame(), CV_8UC3);
//        cv::Mat infrared = toMat(data.get_infrared_frame(), CV_8UC1);

        cv::imshow("Depth", depth);
        cv::imshow("Color", color);
//        cv::imshow("Infrared", infrared);
    }
}


const int max_value_H = 360/2;
const int max_value = 255;
const std::string result_window_name = "Realsense Postprocessing";
int decimationMagnitude = 2;
int decimationMagnitudeMin = 2;
int decimationMagnitudeMax = 8;
int spatialMagnitude = 2;
int spatialMagnitudeMin = 1;
int spatialMagnitudeMax = 6;

static void onDecimationMagnitudeChange(int pos, void * userData) {
    decimationMagnitude = cv::max(cv::min(pos, decimationMagnitudeMax), decimationMagnitudeMin);
    cv::setTrackbarPos("Decimate Magnitude", result_window_name, decimationMagnitude);
}

static void onSpatialMagnitudeChange(int pos, void * userData) {
    spatialMagnitude = cv::max(cv::min(pos, spatialMagnitudeMax), spatialMagnitudeMin);
    cv::setTrackbarPos("Spatial filter magnitude", result_window_name, spatialMagnitude);
}

TEST(Realsense, tryOutPostProcessing) {
    cv::namedWindow(result_window_name, cv::WINDOW_AUTOSIZE);

    cv::createTrackbar("Decimate Magnitude", result_window_name, &decimationMagnitude, decimationMagnitudeMax, onDecimationMagnitudeChange);
    cv::createTrackbar("Spatial filter magnitude", result_window_name, &spatialMagnitude, spatialMagnitudeMax, onSpatialMagnitudeChange);

    rs2::context ctx;
    std::cout << "librealsense " << RS2_API_VERSION_STR << std::endl;
    std::cout << "RealSense devices connected: " << ctx.query_devices().size() << std::endl;

    rs2::config cfg;
    cfg.enable_stream(RS2_STREAM_COLOR, 1920, 1080, RS2_FORMAT_BGR8, 30);
    cfg.enable_stream(RS2_STREAM_DEPTH, 848, 480, RS2_FORMAT_Z16, 30);

    rs2::pipeline pipe;
    pipe.start(cfg);

    rs2::decimation_filter decimationFilter;
    rs2::spatial_filter spatialFilter;

    cv::namedWindow("Depth", cv::WINDOW_AUTOSIZE);
    cv::namedWindow("Color", cv::WINDOW_AUTOSIZE);

    while (cv::waitKey(1) < 0) {

        decimationFilter.set_option(RS2_OPTION_FILTER_MAGNITUDE, decimationMagnitude);
        spatialFilter.set_option(RS2_OPTION_FILTER_MAGNITUDE, spatialMagnitude);

        rs2::frameset data = pipe.wait_for_frames(); // Wait for next set of frames from the camera

        rs2::colorizer color_map;
        cv::Mat depth = toMat(data.get_depth_frame()
                .apply_filter(decimationFilter)
                .apply_filter(color_map), CV_8UC3);
        cv::Mat color = toMat(data.get_color_frame(), CV_8UC3);

        cv::imshow("Depth", depth);
        cv::imshow("Color", color);
    }
}
