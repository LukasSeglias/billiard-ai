#pragma once

#include <billiard_detection/billiard_detection.hpp>

inline cv::Size getImageSize() {
    return cv::Size {1280, 720};
}

inline billiard::detection::CameraIntrinsics getIntrinsics_matlab_hd() {
    return billiard::detection::CameraIntrinsics {
            cv::Point2d { 777.3083, 777.6202 },
            cv::Point2d { 656.1982, 357.4891 },
            0.1715,
            cv::Point3d { 0.1316, -0.3062, 0.2113 },
            cv::Point2d { 0.0010, -0.0015 },
            cv::Point2d { 0.0014, 0.0014 }
    };
}

inline billiard::detection::CameraIntrinsics getIntrinsics_realsense_hd() {
    return billiard::detection::CameraIntrinsics {
            cv::Point2d { 917.126, 917.236 },
            cv::Point2d { 649.895, 359.575 },
            0.0,
            cv::Point3d { 0.0, 0.0, 0.0 },
            cv::Point2d { 0.0, 0.0 },
            cv::Point2d { 0.0014, 0.0014 }
    };
}

inline billiard::detection::CameraIntrinsics getIntrinsics_realsense_full_hd() {
    return billiard::detection::CameraIntrinsics {
            cv::Point2d { 1375.69, 1375.85 },
            cv::Point2d { 974.842, 539.363 },
            0.0,
            cv::Point3d { 0.0, 0.0, 0.0 },
            cv::Point2d { 0.0, 0.0 },
            cv::Point2d { 0.0014, 0.0014 }
    };
}

inline billiard::detection::Table getTable() {
    billiard::detection::Table table;
    table.innerTableLength = 1881.0;
    table.innerTableWidth = 943.0;
    table.ballDiameter = 52.3;
    table.railWorldPointZComponent = 0.0;
    table.arucoHeightAboveInnerTable = 38.1;
    table.worldToRail = cv::Vec3d(79.0, -71.5, 0.0);
    return table;
}

inline billiard::detection::ArucoMarkers getMarkers() {
    billiard::detection::ArucoMarkers markers;
    markers.patternSize = 3;
    markers.sideLength  = 50;
    markers.bottomLeft  = cv::Point3f{0, 0, 0};
    markers.bottomRight = cv::Point3f{1722, 0, 0};
    markers.topRight    = cv::Point3f{1722, 1084, 0};
    markers.topLeft     = cv::Point3f{0, 1085, 0};
    return markers;
}
