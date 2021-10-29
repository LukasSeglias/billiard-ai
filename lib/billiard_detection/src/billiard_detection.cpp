#include <billiard_detection/billiard_detection.hpp>
#include <billiard_debug/billiard_debug.hpp>

#ifndef BILLIARD_DETECTION_DEBUG_VISUAL
    #ifdef NDEBUG
        #undef BILLIARD_DETECTION_DEBUG_VISUAL
    #endif
    #ifndef NDEBUG
        #define BILLIARD_DETECTION_DEBUG_VISUAL 1
    #endif
#endif
#ifndef BILLIARD_DETECTION_DEBUG_PRINT
#ifdef NDEBUG
        #undef BILLIARD_DETECTION_DEBUG_PRINT
    #endif
    #ifndef NDEBUG
        #define BILLIARD_DETECTION_DEBUG_PRINT 1
    #endif
#endif

// TODO: remove this
//#undef BILLIARD_DETECTION_DEBUG_VISUAL
//#undef BILLIARD_DETECTION_DEBUG_PRINT

billiard::detection::StateTracker::StateTracker(const std::shared_ptr<capture::CameraCapture>& capture,
                                                const std::shared_ptr<billiard::detection::DetectionConfig>& config,
                                                const std::function<State(const State& previousState,
                                                                          const cv::Mat&)>& detect,
                                                const std::function<void (const State& previousState,
                                                                          State& currentState,
                                                                          const cv::Mat&)>& classify)
        :
        _exitSignal(),
        _lock(),
        _waiting(),
        _thread(std::thread(StateTracker::work, _exitSignal.get_future(), std::ref(_lock),
                            capture, config, detect, classify, std::ref(_waiting))) {
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

void assignIds(const billiard::detection::State& previousState, billiard::detection::State& currentState) {

    int id = 1;
    // TODO: check previous state (if not empty) and try tracking of balls?
    for (auto& ball : currentState._balls) {
        ball._id = ball._type + "-" + std::to_string(id++);
    }
}

void billiard::detection::StateTracker::work(std::future<void> exitSignal,
          std::mutex& lock,
          const std::shared_ptr<capture::CameraCapture>& capture,
          const std::shared_ptr<billiard::detection::DetectionConfig>& config,
          const std::function<State (const State& previousState, const cv::Mat&)>& detect,
          const std::function<void (const State& previousState, State& currentState, const cv::Mat&)>& classify,
          std::queue<std::promise<State>>& waiting) {
    State previousState;
    while (exitSignal.wait_for(std::chrono::nanoseconds(10)) == std::future_status::timeout) {
#ifndef NDEBUG
        lock.lock();
        if (waiting.empty()) {
            lock.unlock();
            continue;
        }
        lock.unlock();
#endif
        auto image = capture->read();
        auto state = detect(previousState, image.color);
        classify(previousState, state, image.color);
        assignIds(previousState, state);
        previousState = state;
        lock.lock();
        if (!waiting.empty()) {
            // Convert state to internal coordinates
            auto modelState = pixelToModelCoordinates(*config, state);
            while (!waiting.empty()) {
                auto& prom = waiting.front();
                prom.set_value(modelState);
                waiting.pop();
            }
        }
        lock.unlock();
    }
}

namespace billiard::detection {

    DetectedMarkers detect(const cv::Mat& image, const cv::Ptr<cv::aruco::Board>& board) {
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

    CameraToWorldCoordinateSystemConfig configure(const cv::Mat& image,
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

#ifdef BILLIARD_DETECTION_DEBUG_PRINT
                std::cout << "cameraInWorldCoordinatesHomogeneous: " << cameraInWorldCoordinatesHomogeneous
                          << std::endl;
                std::cout << "cameraInWorldCoordinatesMat: " << cameraInWorldCoordinatesMat << std::endl;
                std::cout << "Camera is at " << config.cameraPosInWorldCoordinates << " in the real world" << std::endl;
                std::cout << "f: " << std::to_string(config.focalLength) << " fx: " << std::to_string(fx) << " fy: "
                          << std::to_string(fy) << " s: " << intrinsics.sensorSize << " c: " << config.principalPoint
                          << std::endl;
#endif

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
        // TODO: handle case where v * n = 0 -> no intersection
        double lambda = (planePoint - linePoint).dot(planeNormal) / lineDirection.dot(planeNormal);

        cv::Point3d worldPoint(linePoint + (lambda * lineDirection));

#ifdef BILLIARD_DETECTION_DEBUG_PRINT
        std::cout << "Line: " << linePoint << " + " << std::to_string(lambda) << " * " << lineDirection << std::endl;
        std::cout << "Plane: (p - " << planePoint << ") * " << planeNormal << " = 0" << std::endl;
#endif

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

#ifdef BILLIARD_DETECTION_DEBUG_PRINT
            std::cout << "image: " << imagePoint << " camera: " << imagePointInCameraCoordinatesHomogenous << " world: "
                      << imagePointInWorldCoordinates << std::endl;
#endif

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

    std::vector<cv::Point3d> modelPointsToWorldPoints(const WorldToModelCoordinates& worldToModel,
                                                      const std::vector<cv::Point2d>& modelPoints,
                                                      double z) {

        const cv::Point3d& invTranslation = - cv::Point3d(worldToModel.translation);
        std::vector<cv::Point3d> worldPoints;
        for (const cv::Point2d& modelPoint : modelPoints) {
            cv::Point3d worldPoint = cv::Point3d(modelPoint.x, modelPoint.y, z) + invTranslation;
            worldPoints.push_back(worldPoint);
        }
        return worldPoints;
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

    /**
     * Get rail rectangle in model coordinates
     */
    std::vector<cv::Point2d> getRailRect(double innerTableLength, double innerTableWidth) {
        const cv::Point2d& innerTableCenter { innerTableLength/2, innerTableWidth/2 };
        double railTop = innerTableWidth;
        double railBottom = 0.0;
        double railLeft = 0.0;
        double railRight = innerTableLength;
        return { // In model coordinates
                // Bottom-left
                cv::Point2d { railLeft, railBottom } - innerTableCenter,
                // Top-left
                cv::Point2d { railLeft, railTop } - innerTableCenter,
                // Top-right
                cv::Point2d { railRight, railTop } - innerTableCenter,
                // Bottom-right
                cv::Point2d { railRight, railBottom } - innerTableCenter,
        };
    }

   /**
    * Get table pockets in model coordinates
    */
    std::vector<Pocket> getPockets(double innerTableLength,
                                   double innerTableWidth) {

       // Translation from rail-coordinate system to model-coordinate system
        double railToModelX = innerTableLength/2;
        double railToModelY = innerTableWidth/2;

        double middlePocketRadius = 50;
        double cornerPocketRadius = 50;

        double pocketTop = innerTableWidth - railToModelY;
        double pocketBottom = 0.0 - railToModelY;
        double pocketLeft = 0.0 - railToModelX;
        double pocketRight = innerTableLength - railToModelX;
        double pocketMiddle = innerTableLength/2 - railToModelX;

        return { // In model coordinates
                Pocket { pocketLeft  + 25, pocketBottom + 15, cornerPocketRadius }, // Bottom-left
                Pocket { pocketLeft  + 25, pocketTop    - 15, cornerPocketRadius }, // Top-left
                Pocket { pocketMiddle,        pocketTop    + 15, middlePocketRadius }, // Top-middle
                Pocket { pocketRight - 25, pocketTop    - 15, cornerPocketRadius }, // Top-right
                Pocket { pocketRight - 25, pocketBottom + 15, cornerPocketRadius }, // Bottom-right
                Pocket { pocketMiddle,        pocketBottom - 15, middlePocketRadius }, // Bottom-middle
        };
    }

    inline std::vector<cv::Point> toIntPoints(const std::vector<cv::Point2d>& points) {
        std::vector<cv::Point> intPoints;
        for (auto& imagePoint : points) {
            intPoints.emplace_back((int) imagePoint.x,  (int) imagePoint.y);
        }
        return intPoints;
    }

    cv::Ptr<cv::aruco::Board> createArucoBoard(const ArucoMarkers& markers) {

        const int nMarkers = 4;
        std::vector<int> ids = {0, 1, 2, 3};
        cv::Ptr<cv::aruco::Dictionary> dictionary = cv::aruco::generateCustomDictionary(nMarkers, markers.patternSize);

        const float side = markers.sideLength;

        auto cornerPositions = [&side](const cv::Point3f& bottomLeft) -> std::vector<cv::Point3f> {
            return {
                    bottomLeft + cv::Point3f{ 0, side, 0 },
                    bottomLeft + cv::Point3f{ side, side, 0 },
                    bottomLeft + cv::Point3f{ side, 0, 0 },
                    bottomLeft + cv::Point3f{ 0, 0, 0 },
            };
        };

        cv::Point3f centerOffset{side/2, side/2, 0};
        std::vector<std::vector<cv::Point3f>> objPoints = {
                cornerPositions(markers.bottomLeft - centerOffset), // Marker 0
                cornerPositions(markers.bottomRight - centerOffset), // Marker 1
                cornerPositions(markers.topRight - centerOffset), // Marker 2
                cornerPositions(markers.topLeft - centerOffset), // Marker 3
        };

        cv::Ptr<cv::aruco::Board> board = cv::aruco::Board::create(objPoints, dictionary, ids);
        return board;
    }

#ifdef BILLIARD_DETECTION_DEBUG_PRINT
    void printCoordinates(const std::string& context,
                          const std::vector<cv::Point2d>& imagePoints,
                          const std::vector<cv::Point3d>& worldPoints,
                          const std::vector<cv::Point2d>& modelPoints) {

        for(int i = 0; i < imagePoints.size(); i++) {
            auto& imagePoint = imagePoints[i];
            auto& worldPoint = worldPoints[i];
            auto& modelPoint = modelPoints[i];
            std::cout << "[" << context << "]"
                      << " image point: " << imagePoint
                      << " world point: " << worldPoint
                      << " model point: " << modelPoint
                      << std::endl;
        }
    }
#endif

#ifdef BILLIARD_DETECTION_DEBUG_VISUAL
    inline void drawRailRectLines(cv::Mat& output, const std::vector<cv::Point2d>& points) {

        cv::Scalar color(0, 0, 255);
        int thickness = 1;
        cv::line(output, points[0], points[1], color, thickness);
        cv::line(output, points[1], points[2], color, thickness);
        cv::line(output, points[2], points[3], color, thickness);
        cv::line(output, points[3], points[0], color, thickness);
    }

    inline void drawLines(cv::Mat& output, const std::vector<cv::Point2d>& lines) {

        cv::Scalar color(0, 0, 255);
        int thickness = 1;
        for (int i = 1; i < lines.size(); i+= 2) {
            cv::line(output, lines[i-1], lines[i], color, thickness);
        }
    }

    void drawHoughResult(cv::Mat& image, std::vector<cv::Vec3f>& circles) {
        for(auto c : circles) {
            cv::Point center = cv::Point(c[0], c[1]);
            uint8_t radius = c[2];
            cv::Rect roi(center.x - radius, center.y - radius, radius * 2, radius * 2);
            if (roi.x >= 0 && roi.y >= 0 && roi.width <= image.cols && roi.height <= image.rows) {

                // circle center
                cv::circle(image, center, 1, cv::Scalar(0, 100, 100), 3, cv::LINE_AA);
                // circle outline
                cv::circle(image, center, radius, cv::Scalar(255, 0, 255), 1, cv::LINE_AA);
            }
        }
    }
#endif

    DetectionConfig configure(const cv::Mat& original,
                              const Table& table,
                              const ArucoMarkers& markers,
                              const CameraIntrinsics& intrinsics) {

        DetectionConfig config;

        cv::Ptr<cv::aruco::Board> board = createArucoBoard(markers);
        CameraToWorldCoordinateSystemConfig cameraToWorld = configure(original, board, intrinsics);

        if (!cameraToWorld.valid) {
            config.valid = false;
            return config;
        }
        config.cameraToWorld = cameraToWorld;

        config.ballPlane = {{0, 0, table.ballDiameter/2 - table.arucoHeightAboveInnerTable}, {0, 0, 1}};

        cv::Vec3d railToModel { table.innerTableLength/2, table.innerTableWidth/2, 0.0 };
        config.worldToModel.translation = table.worldToRail - railToModel;

        std::vector<cv::Point2d> railRect = getRailRect(table.innerTableLength, table.innerTableWidth);
        std::vector<Pocket> pockets = getPockets(table.innerTableLength, table.innerTableWidth);
//        std::vector<Pocket> pockets = table.pockets; // TODO: remove this or line above

        // Build inner table mask
        cv::Mat railMask(cv::Size(original.cols, original.rows), CV_8UC1, cv::Scalar(0));
        cv::Mat pocketMask(cv::Size(original.cols, original.rows), CV_8UC1, cv::Scalar(0));
        cv::Mat innerTableMask(cv::Size(original.cols, original.rows), CV_8UC1, cv::Scalar(0));

        // Rails
        {
            std::vector<cv::Point2d> modelPoints = railRect;
            std::vector<cv::Point3d> worldPoints = modelPointsToWorldPoints(config.worldToModel, modelPoints,
                                                                            table.railWorldPointZComponent);
            std::vector<cv::Point2d> imagePoints = worldPointsToImagePoints(config.cameraToWorld, worldPoints);
            cv::fillConvexPoly(railMask, toIntPoints(imagePoints), cv::Scalar(255));
            railMask.copyTo(innerTableMask);

#ifdef BILLIARD_DETECTION_DEBUG_PRINT
            std::cout << "------------------ RAIL-RECT ------------------" << std::endl;
            printCoordinates("Rail-Rect", imagePoints, worldPoints, modelPoints);
#endif
#ifdef BILLIARD_DETECTION_DEBUG_VISUAL
            cv::Mat railOutput = original.clone();
            drawRailRectLines(railOutput, imagePoints);
            cv::resize(railOutput, railOutput, cv::Size(), config.scale, config.scale);
            cv::imshow("rail rectangle", railOutput);
#endif

            // resolutionX = bottom-right - bottom-left
            double resolutionX = (imagePoints[3].x - imagePoints[0].x) * config.scale;
            double tableLength = table.innerTableLength;
            double pixelsPerMillimeterX = resolutionX / tableLength;

            // resolutionY = bottom-left - top-left
            double resolutionY = (imagePoints[0].y - imagePoints[1].y) * config.scale;
            double tableWidth = table.innerTableWidth;
            double pixelsPerMillimeterY = resolutionY / tableWidth;

#ifdef BILLIARD_DETECTION_DEBUG_PRINT
            std::cout << " resolutionX: " << resolutionX
                      << " tableLength: " << tableLength
                      << " resolutionY: " << resolutionY
                      << " tableWidth: " << tableWidth
                      << " pixels per millimeter in X/Y: " << pixelsPerMillimeterX << "/" << pixelsPerMillimeterY
                      << std::endl;
#endif

            double pixelsPerMillimeter = pixelsPerMillimeterX; // Take X because that's the longer axis
            config.pixelsPerMillimeter = pixelsPerMillimeter;
            config.ballRadiusInPixel = ceil((table.ballDiameter / 2.0) * pixelsPerMillimeter);
        }

#ifdef BILLIARD_DETECTION_DEBUG_VISUAL
        // Rail segments
        {
            std::vector<cv::Point2d> modelPoints {};
            for (auto& rail : table.railSegments) {
                modelPoints.emplace_back(rail.start.x, rail.start.y);
                modelPoints.emplace_back(rail.end.x, rail.end.y);
            }
            std::vector<cv::Point3d> worldPoints = modelPointsToWorldPoints(config.worldToModel, modelPoints,
                                                                            table.railWorldPointZComponent);
            std::vector<cv::Point2d> imagePoints = worldPointsToImagePoints(config.cameraToWorld, worldPoints);

            cv::Mat railSegmentOutput = original.clone();
            drawLines(railSegmentOutput, imagePoints);
            cv::resize(railSegmentOutput, railSegmentOutput, cv::Size(), config.scale, config.scale);
            cv::imshow("rail segments", railSegmentOutput);
        }
#endif

        // Pockets
        {
            std::vector<cv::Point2d> centers;
            centers.reserve(pockets.size());
            for (auto& pocket : pockets) centers.emplace_back(pocket.x, pocket.y);

            std::vector<cv::Point2d> modelPoints = centers;
            std::vector<cv::Point3d> worldPoints = modelPointsToWorldPoints(config.worldToModel, modelPoints,
                                                                            table.railWorldPointZComponent);
            std::vector<cv::Point2d> imagePoints = worldPointsToImagePoints(config.cameraToWorld, worldPoints);

            for (int i = 0; i < pockets.size(); i++) {
                auto& pocket = pockets[i];
                int pocketPixelRadius = pocket.radius * (config.pixelsPerMillimeter + 0.1); // TODO: 0.1 because detection
                cv::circle(pocketMask, imagePoints[i], pocketPixelRadius, cv::Scalar{255},
                           cv::LineTypes::FILLED);
                cv::circle(innerTableMask, imagePoints[i], pocketPixelRadius, cv::Scalar{0},
                           cv::LineTypes::FILLED);
            }

#ifdef BILLIARD_DETECTION_DEBUG_VISUAL
            cv::Mat pocketsOutput = original.clone();

            for (int i = 0; i < pockets.size(); i++) {
                auto& pocket = pockets[i];
                int pocketPixelRadius = pocket.radius * config.pixelsPerMillimeter;
                cv::circle(pocketsOutput, imagePoints[i], pocketPixelRadius, cv::Scalar{0, 0, 255}, 1);
            }

            cv::resize(pocketsOutput, pocketsOutput, cv::Size(), config.scale, config.scale);
            cv::imshow("pockets", pocketsOutput);
#endif
        }

        cv::resize(innerTableMask, innerTableMask, cv::Size(), config.scale, config.scale);
        config.innerTableMask = innerTableMask;
        cv::resize(railMask, railMask, cv::Size(), config.scale, config.scale);
        config.railMask = railMask;

#ifdef BILLIARD_DETECTION_DEBUG_VISUAL
        cv::imshow("Rail mask", railMask);
        cv::imshow("Pocket mask", pocketMask);
        cv::imshow("Inner table mask", innerTableMask);

        cv::Mat inputMaskedByInnerTableMask;
        cv::bitwise_and(original, original, inputMaskedByInnerTableMask, innerTableMask);
        cv::imshow("Input masked by inner table mask", inputMaskedByInnerTableMask);

        cv::Mat invInnerTableMask;
        cv::bitwise_not(innerTableMask, invInnerTableMask);
        cv::Mat innerMaskedDelta;
        cv::bitwise_and(original, original, innerMaskedDelta, invInnerTableMask);
        cv::imshow("Input delta masked by inverse inner table mask", innerMaskedDelta);
#endif
        config.valid = true;
        return config;
    }

    State pixelToModelCoordinates(const DetectionConfig& config, const State& input) {

        std::vector<cv::Point2d> imagePoints;
        for(const auto & ball : input._balls) {
            imagePoints.emplace_back(ball._position.x, ball._position.y);
        }

        std::vector<cv::Point3d> worldPoints = billiard::detection::imagePointsToWorldPoints(config.cameraToWorld, config.ballPlane, imagePoints);
        std::vector<cv::Point2d> modelPoints = billiard::detection::worldPointsToModelPoints(config.worldToModel, worldPoints);

#ifdef BILLIARD_DETECTION_DEBUG_PRINT
        std::cout << "------------------ CIRCLES ------------------" << std::endl;
        printCoordinates("Circles", imagePoints, worldPoints, modelPoints);
#endif

        State state;
        for (int i = 0; i < modelPoints.size(); i++) {
            auto& inputBall = input._balls[i];
            auto& point = modelPoints[i];
            Ball ball;
            ball._id = inputBall._id;
            ball._type = inputBall._type;
            ball._position = glm::vec2 {point.x, point.y};
            state._balls.push_back(ball);
        }
        return state;
    }

}