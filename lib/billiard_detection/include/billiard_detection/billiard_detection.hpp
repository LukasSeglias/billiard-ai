#pragma once

#include <billiard_capture/billiard_capture.hpp>
#include <future>
#include <thread>
#include <mutex>
#include <memory>
#include <functional>
#include <queue>

#include "macro_definition.hpp"
#include <opencv2/opencv.hpp>
#include <opencv2/aruco.hpp>
#include "type.hpp"


namespace billiard::detection {

    struct EXPORT_BILLIARD_DETECTION_LIB WorldToModelCoordinates {
        cv::Vec3d translation;
    };

    struct EXPORT_BILLIARD_DETECTION_LIB Plane {
        cv::Vec3d point {0, 0, 0};
        cv::Vec3d normal {0, 0, 1};
        Plane() = default;
        Plane(const cv::Vec3d& point, const cv::Vec3d& normal): point(point), normal(normal) {};
    };

    struct EXPORT_BILLIARD_DETECTION_LIB ArucoMarkers {
        int patternSize;
        float sideLength;
        cv::Point3f bottomLeft;
        cv::Point3f bottomRight;
        cv::Point3f topRight;
        cv::Point3f topLeft;
    };

    struct EXPORT_BILLIARD_DETECTION_LIB Table {
        double innerTableLength; // Length between the rails on the long side, in millimeters
        double innerTableWidth;  // Length between the rails on the short side, in millimeters
        double ballDiameter;     // in millimeters
        double arucoHeightAboveInnerTable; // in millimeters
        double railWorldPointZComponent; // in millimeters
        cv::Vec3d worldToRail;
    };

    struct EXPORT_BILLIARD_DETECTION_LIB CameraIntrinsics {
        /**
         * Format, see here: https://docs.opencv.org/master/d9/d0c/group__calib3d.html#ga3207604e4b1a1758aa66acb6ed5aa65d
         * [ fx skew cx
         *   0  fy   cy
         *   0  0    1  ]
         */
        cv::Mat cameraMatrix;

        /**
         * Format, see here: https://docs.opencv.org/master/d9/d0c/group__calib3d.html#ga3207604e4b1a1758aa66acb6ed5aa65d
         * [k1, k2, p1, p2, k3]
         * Yes, k3 is really supposed to be after p2
         */
        cv::Mat distCoeffs;

        /**
         * Sensor pixel size in x and y direction in milimeters.
         */
        cv::Point2d sensorSize;

        CameraIntrinsics() {}

        CameraIntrinsics(const cv::Point2d& focalLength,
                         const cv::Point2d& principalPoint,
                         double skew,
                         const cv::Point3d& radialDistortionCoefficients,
                         const cv::Point2d& tangentialDistortionCoefficients,
                         const cv::Point2d& sensorPixelSize);
    };

    struct EXPORT_BILLIARD_DETECTION_LIB BoardPoseEstimation {
        int markersUsed = 0;
        cv::Vec3d rvec; // Rotation vector (Rodrigues)
        cv::Vec3d tvec; // Translation vector
    };

    struct EXPORT_BILLIARD_DETECTION_LIB CameraToWorldCoordinateSystemConfig {
        bool valid = false;
        BoardPoseEstimation pose;
        CameraIntrinsics intrinsics;
        cv::Mat boardToCamera;
        cv::Mat cameraToBoard;
        cv::Vec3d cameraPosInWorldCoordinates;
        cv::Point2d principalPoint;
        double focalLength;
    };

    struct EXPORT_BILLIARD_DETECTION_LIB DetectionConfig {
        bool valid = false;

        // Scaling of input image
        double scale = 1.0;

        int ballRadiusInPixel;

        // Binary mask (255=true) to determine possible locations of balls (only on the green inner table)
        // Scaled by DetectionConfig::scale
        cv::Mat innerTableMask;

        // Plane where all the ball centers are located in the real world (aruco marker frame of reference)
        Plane ballPlane;

        WorldToModelCoordinates worldToModel;

        CameraToWorldCoordinateSystemConfig cameraToWorld;
    };

    EXPORT_BILLIARD_DETECTION_LIB DetectionConfig configure(const cv::Mat& original,
                                                            const Table& table,
                                                            const ArucoMarkers& markers,
                                                            const CameraIntrinsics& intrinsics);

    EXPORT_BILLIARD_DETECTION_LIB State detect(const State& previousState, const cv::Mat& image);

    class EXPORT_BILLIARD_DETECTION_LIB StateTracker {
    public:
        StateTracker(const std::shared_ptr<capture::CameraCapture>& capture,
                     const std::shared_ptr<billiard::detection::DetectionConfig>& config,
                     const std::function<State(const State& previousState, const cv::Mat&)>& detect); // TODO: Add Configuration for aruco initialization
        ~StateTracker();

        std::future<State> capture();

    private:
        std::promise<void> _exitSignal;
        std::mutex _lock;
        std::queue<std::promise<State>> _waiting;
        std::thread _thread;

        static void work(std::future<void> exitSignal,
                         std::mutex& lock,
                         const std::shared_ptr<capture::CameraCapture>& capture,
                         const std::shared_ptr<billiard::detection::DetectionConfig>& config,
                         const std::function<State (const State& previousState, const cv::Mat&)>& detect,
                         std::queue<std::promise<State>>& waiting); // TODO: Pass configuration for auruco configuration
    };

    struct EXPORT_BILLIARD_DETECTION_LIB DetectedMarkers {
        bool success;
        std::vector<int> markerIds;
        std::vector<std::vector<cv::Point2f>> markerCorners;
        std::vector<std::vector<cv::Point2f>> rejectedCandidates;
    };

    EXPORT_BILLIARD_DETECTION_LIB DetectedMarkers detect(cv::Mat& image, const cv::Ptr<cv::aruco::Board>& board);

    EXPORT_BILLIARD_DETECTION_LIB void draw(cv::Mat& image, const DetectedMarkers& detection);

    /**
     * Estimates the pose of a board. According to the OpenCV documentation:
     * "The returned transformation is the one that transforms points from
     * the board coordinate system to the camera coordinate system."
     */
    EXPORT_BILLIARD_DETECTION_LIB BoardPoseEstimation estimateBoardPose(const DetectedMarkers& detectionResult,
                                                                        const cv::Ptr<cv::aruco::Board>& board,
                                                                        const CameraIntrinsics& intrinsics);

    EXPORT_BILLIARD_DETECTION_LIB void drawAxis(cv::Mat& image,
                                                const BoardPoseEstimation& pose,
                                                const CameraIntrinsics& intrinsics);

    EXPORT_BILLIARD_DETECTION_LIB CameraToWorldCoordinateSystemConfig configure(const cv::Mat& image,
                                                                                const cv::Ptr<cv::aruco::Board>& board,
                                                                                const CameraIntrinsics& intrinsics);

    EXPORT_BILLIARD_DETECTION_LIB std::vector<cv::Point2d> worldPointsToImagePoints(
            const CameraToWorldCoordinateSystemConfig& config,
            const std::vector<cv::Point3d>& worldPoints);

    EXPORT_BILLIARD_DETECTION_LIB std::vector<cv::Point2d> worldPointsToModelPoints(
            const WorldToModelCoordinates& worldToModel,
            const std::vector<cv::Point3d>& worldPoints);

    EXPORT_BILLIARD_DETECTION_LIB std::vector<cv::Point3d> modelPointsToWorldPoints(
            const WorldToModelCoordinates& worldToModel,
            const std::vector<cv::Point2d>& modelPoints,
            double z);

    /**
     * Translate image points to world points located on a plane
     */
    EXPORT_BILLIARD_DETECTION_LIB std::vector<cv::Point3d> imagePointsToWorldPoints(
            const CameraToWorldCoordinateSystemConfig& config,
            const Plane& plane,
            const std::vector<cv::Point2d>& imagePoints);

    EXPORT_BILLIARD_DETECTION_LIB State pixelToModelCoordinates(const DetectionConfig& config, const State& input);
}