#include <billiard_detection/billiard_detection.hpp>

billiard::detection::StateTracker::StateTracker(const std::shared_ptr<capture::ImageCapture>& imageCapture,
                                                const std::function<State(const State& previousState,
                                                                          const cv::Mat&)>& detect)
        :
        _exitSignal(),
        _lock(),
        _waiting(),
        _thread(std::thread(StateTracker::work, _exitSignal.get_future(), std::ref(_lock), imageCapture, detect,
                            std::ref(_waiting))) {
}

billiard::detection::StateTracker::~StateTracker() {
    _exitSignal.set_value();
    _thread.join();
}

std::future<billiard::detection::State> billiard::detection::StateTracker::capture() {
    std::promise<State> promise;
    auto future = promise.get_future();
    _lock.lock();
    _waiting.emplace(std::move(promise));
    _lock.unlock();
    return future;
}

void billiard::detection::StateTracker::work(std::future<void> exitSignal,
          std::mutex& lock,
          const std::shared_ptr<capture::ImageCapture>& imageCapture,
          const std::function<State (const State& previousState, const cv::Mat&)>& detect,
          std::queue<std::promise<State>>& waiting) {
    State previousState;
    // TODO: Initialize auruco-marker, internal state matrices
    while (exitSignal.wait_for(std::chrono::nanoseconds(10)) == std::future_status::timeout) {
        auto image = imageCapture->read();
        auto state = detect(previousState, image);
        previousState = state;
        lock.lock();
        if (!waiting.empty()) {
            // TODO: Convert state to internal coordinates
            while (!waiting.empty()) {
                auto& prom = waiting.front();
                prom.set_value(state);
                waiting.pop();
            }
        }
        lock.unlock();
    }
}

namespace billiard::detection {

    DetectedMarkers detect(cv::Mat& image, const cv::Ptr<cv::aruco::Board>& board) {
        DetectedMarkers result{};

        cv::Ptr<cv::aruco::DetectorParameters> parameters = cv::aruco::DetectorParameters::create();
        cv::aruco::detectMarkers(image, board->dictionary, result.markerCorners,
                                 result.markerIds, parameters, result.rejectedCandidates);

        result.success = result.markerIds.size() > 0;
        return result;
    }

    void draw(cv::Mat& image, const DetectedMarkers& detection) {
        cv::aruco::drawDetectedMarkers(image, detection.markerCorners, detection.markerIds);
    }

    /**
     * Estimates the pose of a board. According to the OpenCV documentation:
     * "The returned transformation is the one that transforms points from
     * the board coordinate system to the camera coordinate system."
     */
    BoardPoseEstimation estimateBoardPose(const DetectedMarkers& detectionResult,
                                          const cv::Ptr<cv::aruco::Board>& board,
                                          const CameraIntrinsics& intrinsics) {

        BoardPoseEstimation pose;
        pose.markersUsed = cv::aruco::estimatePoseBoard(detectionResult.markerCorners,
                                                        detectionResult.markerIds, board,
                                                        intrinsics.cameraMatrix, intrinsics.distCoeffs,
                                                        pose.rvec, pose.tvec);
        return pose;
    }

    void drawAxis(cv::Mat& image, const BoardPoseEstimation& pose, const CameraIntrinsics& intrinsics) {
        cv::aruco::drawAxis(image, intrinsics.cameraMatrix, intrinsics.distCoeffs, pose.rvec, pose.tvec, 100);
    }

    cv::Mat buildBoardToCameraMatrix(const BoardPoseEstimation& pose) {

        cv::Mat rotationMatrix;
        cv::Rodrigues(pose.rvec, rotationMatrix);

        cv::Mat boardToCamera{cv::Size{4, 4}, CV_64F, cv::Scalar(0.0f)};
        for (int row = 0; row <= 2; row++) {
            for (int col = 0; col <= 2; col++) {
                boardToCamera.at<double>(row, col) = rotationMatrix.at<double>(row, col);
            }
        }
        for (int row = 0; row <= 2; row++) {
            boardToCamera.at<double>(row, 3) = pose.tvec[row];
        }
        boardToCamera.at<double>(3, 3) = 1.0;

        return boardToCamera;
    }

    CameraToWorldCoordinateSystemConfig configure(cv::Mat& image,
                                                  const cv::Ptr<cv::aruco::Board>& board,
                                                  const CameraIntrinsics& intrinsics) {

        DetectedMarkers detection = detect(image, board);

        if (detection.success) {

            BoardPoseEstimation pose = estimateBoardPose(detection, board, intrinsics);
            if (pose.markersUsed > 0) {

                CameraToWorldCoordinateSystemConfig config;
                config.valid = true;
                config.pose = pose;
                config.intrinsics = intrinsics;
                config.boardToCamera = buildBoardToCameraMatrix(pose);
                cv::invert(config.boardToCamera, config.cameraToBoard);

                cv::Vec4d cameraInCameraCoordinates{0, 0, 0, 1};
                cv::Mat cameraInWorldCoordinatesHomogeneous = config.cameraToBoard * cameraInCameraCoordinates;
                cv::Mat cameraInWorldCoordinatesMat = cameraInWorldCoordinatesHomogeneous(cv::Rect{0, 0, 1, 3});
                config.cameraPosInWorldCoordinates = cv::Vec3d(cameraInWorldCoordinatesMat);

                config.principalPoint = {intrinsics.cameraMatrix.at<double>(0, 2),
                                         intrinsics.cameraMatrix.at<double>(1, 2)};
                double fx = intrinsics.cameraMatrix.at<double>(0, 0);
                double fy = intrinsics.cameraMatrix.at<double>(1, 1);
                config.focalLength = fx * intrinsics.sensorSize.x;

                std::cout << "cameraInWorldCoordinatesHomogeneous: " << cameraInWorldCoordinatesHomogeneous
                          << std::endl;
                std::cout << "cameraInWorldCoordinatesMat: " << cameraInWorldCoordinatesMat << std::endl;
                std::cout << "Camera is at " << config.cameraPosInWorldCoordinates << " in the real world" << std::endl;
                std::cout << "f: " << std::to_string(config.focalLength) << " fx: " << std::to_string(fx) << " fy: "
                          << std::to_string(fy) << " s: " << intrinsics.sensorSize << " c: " << config.principalPoint
                          << std::endl;

                return config;
            }
        }
        return CameraToWorldCoordinateSystemConfig{};
    }

    std::vector<cv::Point2d> worldPointsToImagePoints(const CameraToWorldCoordinateSystemConfig& config,
                                                      const std::vector<cv::Point3d>& worldPoints) {

        std::vector<cv::Point2d> imagePoints;
        cv::projectPoints(worldPoints, config.pose.rvec, config.pose.tvec,
                          config.intrinsics.cameraMatrix, config.intrinsics.distCoeffs,
                          imagePoints);
        return imagePoints;
    }

    cv::Point3d linePlaneIntersection(const cv::Vec3d& linePoint, const cv::Vec3d& lineDirection,
                                      const cv::Vec3d& planePoint, const cv::Vec3d& planeNormal) {

        // Given a plane as (p - a) * n = 0
        // where
        // - n is the normal vector of the plane
        // - a is a known point on the plane
        // - p is a variable point on the plane
        //
        // And given a line as p = q + lambda * v
        // where
        // - v is the direction vector of the line
        // - q is a known point on the line
        // - p is a variable point on the line
        // - lambda is the scaling factor of the direction vector v
        //
        // Then the intersection can be found by inserting the line equation into the equation of the plane
        // (q + lambda * v - a) * n = 0
        // q * n + lambda * v * n - a * n = 0
        // lambda * v * n = a * n - q * n
        // lambda * v * n = (a - q) * n
        // lambda = ((a - q) * n) / (v * n)
        //
        // Note that if v * n = 0, then there is no intersection
        //
        double lambda = (planePoint - linePoint).dot(planeNormal) / lineDirection.dot(planeNormal);

        cv::Point3d worldPoint(linePoint + (lambda * lineDirection));

        {
            std::cout << "Line: " << linePoint << " + " << std::to_string(lambda) << " * " << lineDirection
                      << std::endl;
            std::cout << "Plane: (p - " << planePoint << ") * " << planeNormal << " = 0" << std::endl;
        }

        return worldPoint;
    }

    std::vector<cv::Point3d> imagePointsToWorldPoints(const CameraToWorldCoordinateSystemConfig& config,
                                                      const Plane& plane,
                                                      const std::vector<cv::Point2d>& imagePoints) {

        std::vector<cv::Point3d> worldPoints;

        cv::Point2d s = config.intrinsics.sensorSize;
        cv::Point2d c = config.principalPoint;
        double f = config.focalLength;

        for (auto& imagePoint : imagePoints) {

            cv::Vec4d imagePointInCameraCoordinatesHomogenous{(imagePoint.x - c.x) * s.x,
                                                              (imagePoint.y - c.y) * s.y,
                                                              f,
                                                              1};

            cv::Mat imagePointInWorldCoordinatesMat = config.cameraToBoard * imagePointInCameraCoordinatesHomogenous;
            cv::Vec3d imagePointInWorldCoordinates = cv::Vec3d(
                    imagePointInWorldCoordinatesMat.rowRange(cv::Range(0, 3)));

            cv::Vec3d linePoint = config.cameraPosInWorldCoordinates;
            cv::Vec3d lineDirection = imagePointInWorldCoordinates - config.cameraPosInWorldCoordinates;

            cv::Point3d worldPoint = linePlaneIntersection(linePoint, lineDirection, plane.point, plane.normal);

            std::cout << "image: " << imagePoint << " camera: " << imagePointInCameraCoordinatesHomogenous << " world: "
                      << imagePointInWorldCoordinates << std::endl;

            worldPoints.push_back(worldPoint);
        }

        return worldPoints;
    }

    std::vector<cv::Point2d> worldPointsToModelPoints(
            const WorldToModelCoordinates& worldToModel,
            const std::vector<cv::Point3d>& worldPoints) {

        const cv::Point3d& translation = cv::Point3d(worldToModel.translation);
        std::vector<cv::Point2d> modelPoints;
        for (const cv::Point3d& worldPoint : worldPoints) {
            cv::Point3d modelPoint = worldPoint + translation;
            modelPoints.emplace_back(modelPoint.x, modelPoint.y);
        }
        return modelPoints;
    }

    CameraIntrinsics::CameraIntrinsics(const cv::Point2d& focalLength,
                                       const cv::Point2d& principalPoint,
                                       double skew,
                                       const cv::Point3d& radialDistortionCoefficients,
                                       const cv::Point2d& tangentialDistortionCoefficients,
                                       const cv::Point2d& sensorPixelSize) {

        double fx = focalLength.x;
        double fy = focalLength.y;

        double cx = principalPoint.x;
        double cy = principalPoint.y;

        // Format, see here: https://docs.opencv.org/master/d9/d0c/group__calib3d.html#ga3207604e4b1a1758aa66acb6ed5aa65d
        // [ fx skew cx
        //   0  fy   cy
        //   0  0    1  ]
        this->cameraMatrix = (cv::Mat1d(3, 3) << fx, skew, cx, 0, fy, cy, 0, 0, 1);

        double k1 = radialDistortionCoefficients.x;
        double k2 = radialDistortionCoefficients.y;
        double k3 = radialDistortionCoefficients.z;
        double p1 = tangentialDistortionCoefficients.x;
        double p2 = tangentialDistortionCoefficients.y;

        // Format: [k1, k2, p1, p2, k3], see here: https://docs.opencv.org/master/d9/d0c/group__calib3d.html#ga3207604e4b1a1758aa66acb6ed5aa65d
        // Yes, k3 is really supposed to be after p2
        this->distCoeffs = (cv::Mat1d(1, 5) << k1, k2, p1, p2, k3);

        this->sensorSize = sensorPixelSize;
    }

}