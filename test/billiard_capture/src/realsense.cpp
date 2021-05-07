#include <gtest/gtest.h>
#include <billiard_capture/billiard_capture.hpp>
#include <librealsense2/rs.hpp>
#include <librealsense2/hpp/rs_internal.hpp>
#include <iostream>

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

bool safe_get_intrinsics(const rs2::video_stream_profile& profile, rs2_intrinsics& intrinsics) {
    bool ret = false;
    try {
        intrinsics = profile.get_intrinsics();
        ret = true;
    }
    catch (...) {
    }

    return ret;
}

TEST(RealSense, getCalibrationParameters) {

    rs2::context ctx;
    auto devices = ctx.query_devices();

    if (devices.size() == 0) {
        std::cout << "No device found" << std::endl;
        return;
    }

    auto dev = *devices.begin();

    for (auto&& sensor : dev.query_sensors()) {

        for (auto&& profile : sensor.get_stream_profiles()) {

            if (profile.stream_type() == rs2_stream::RS2_STREAM_COLOR && profile.fps() == 30) {

                if (auto video = profile.as<rs2::video_stream_profile>()) {

                    if (video.format() == rs2_format::RS2_FORMAT_BGR8 && video.width() >= 1280) {
                        rs2_intrinsics intrinsics{};
                        if (safe_get_intrinsics(video, intrinsics)) {
                            std::cout << "intrinsics " << " fps: " << profile.fps() << " width: " << intrinsics.width << " height: " << intrinsics.height << " model: " << intrinsics.model << " coeffs: " << intrinsics.coeffs << " fx: " << intrinsics.fx << " fy: " << intrinsics.fy << " cx: " << intrinsics.ppx << " cy: " << intrinsics.ppy <<  std::endl;
                        }
                    }
                }
            }
        }
    }
}


TEST(RealSense, screenshot) {

    double scale = 0.5;

    billiard::capture::CameraCapture capture {};

    if (capture.open()) {

        cv::namedWindow("Color", cv::WINDOW_AUTOSIZE);
        cv::namedWindow("Depth", cv::WINDOW_AUTOSIZE);

        int screenshotNumber = 2;
        while (true) {
            billiard::capture::CameraFrames frames = capture.read();

            if (!frames.depth.empty()) {
                cv::imshow("Depth", frames.colorizedDepth);
            }
            cv::Mat resizedColor;
            cv::resize(frames.color, resizedColor, cv::Size(0,0), scale, scale);

            cv::circle(resizedColor, cv::Point2i{resizedColor.cols / 2, resizedColor.rows / 2}, 3, cv::Scalar{255, 0, 0}, 1);
            cv::imshow("Color", resizedColor);

            int key = cv::waitKey(5);
            if (key == 27 /* ESC */) {
                break;
            } else if(key == 32 /* SPACE */) {
                imwrite("Screenshot-" + std::to_string(screenshotNumber++) + ".png", frames.color);
            }
        }
        capture.close();
    }
}