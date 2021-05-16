#include <gtest/gtest.h>
#include <billiard_detection/billiard_detection.hpp>
#include <billiard_capture/billiard_capture.hpp>
#include <librealsense2/rs.hpp>

TEST(ArUcoMarkers, generate) {

    // Based on the tutorial on https://docs.opencv.org/4.5.1/d5/dae/tutorial_aruco_detection.html

    const int nMarkers = 4;
    const int markerSize = 3;
    cv::Ptr<cv::aruco::Dictionary> dictionary = cv::aruco::generateCustomDictionary(nMarkers, markerSize);

    for (int i = 0; i < nMarkers; i++) {
        cv::Mat markerImage;
        cv::aruco::drawMarker(dictionary, i, 200, markerImage, 1);
        cv::imshow("marker_3_" + std::to_string(i) + ".png", markerImage);
        cv::imwrite("marker_3_" + std::to_string(i) + ".png", markerImage);
    }
    cv::waitKey();
}

TEST(DISABLED_ArUcoMarkers, detection) {

    // Based on the tutorial on https://docs.opencv.org/4.5.1/d5/dae/tutorial_aruco_detection.html

    const int nMarkers = 4;
    const int markerSize = 6;
    cv::Ptr<cv::aruco::Dictionary> dictionary = cv::aruco::generateCustomDictionary(nMarkers, markerSize);

    cv::Mat frame;
    cv::VideoCapture capture;
    capture.open(0);
    if (!capture.isOpened()) {
        std::cerr << "ERROR! Unable to open camera\n";
    }
    while (capture.read(frame)) {
        if (frame.empty()) {
            std::cerr << "ERROR! blank frame grabbed\n";
            break;
        }
        imshow("Live", frame);

        std::vector<int> markerIds;
        std::vector<std::vector<cv::Point2f>> markerCorners, rejectedCandidates;
        cv::Ptr<cv::aruco::DetectorParameters> parameters = cv::aruco::DetectorParameters::create();
        cv::aruco::detectMarkers(frame, dictionary, markerCorners, markerIds, parameters, rejectedCandidates);

        cv::Mat result = frame.clone();
        if (!markerIds.empty()) {
            cv::aruco::drawDetectedMarkers(result, markerCorners, markerIds);
        }
        cv::imshow("Result", result);

        if (cv::waitKey(5) >= 0) break;
    }
}

// Matlab camera calibration output for Webcam Lukas:
// focal length: [9.945224073035316e+02,9.923185492548951e+02]
// principal point: [6.332908183101254e+02,3.683501984998175e+02]
// skew: 0.686462846096602
// radial distortion: [0.116267318681632,-0.488225060983731,0.625286665345480]
// tangential distortion: [-0.003430984981564,-0.001422152228439]
// intrinsics:
// [9.945224073035316e+02,0,                     0;
// 0.686462846096602,     9.923185492548951e+02, 0;
// 6.332908183101254e+02, 3.683501984998175e+02, 1]
// Mean reprojection error: 0.318533449543939

cv::Mat getCameraMatrix() {

//    double f[] = {9.945224073035316e+02, 9.923185492548951e+02}; // Webcam Lukas
    double f[] = {1375.69, 1375.85}; // RealSense color image 1920x1080
    double fx = f[0];
    double fy = f[1];

//    double c[] = {6.332908183101254e+02,3.683501984998175e+02}; // Webcam Lukas
    double c[] = {974.842, 539.363}; // RealSense color image 1920x1080
    double cx = c[0];
    double cy = c[1];

//    double skew = 0.686462846096602; // Webcam Lukas
    double skew = 0; // Unknown for RealSense color image 1920x1080

    // Format, see here: https://docs.opencv.org/master/d9/d0c/group__calib3d.html#ga3207604e4b1a1758aa66acb6ed5aa65d
    // [ fx skew cx
    //   0  fy   cy
    //   0  0    1  ]
    cv::Mat cameraMatrix = (cv::Mat1d(3, 3) << fx, skew, cx, 0, fy, cy, 0, 0, 1);
    return cameraMatrix;
}

cv::Mat getDistCoeffs() {

//    double radialDistortion[] = {0.116267318681632,-0.488225060983731,0.625286665345480}; // Webcam Lukas
    double radialDistortion[] = {0, 0, 0}; // RealSense
    double k1 = radialDistortion[0];
    double k2 = radialDistortion[1];
    double k3 = radialDistortion[2];
//    double tangentialDistortion[] = {-0.003430984981564,-0.001422152228439}; // Webcam Lukas
    double tangentialDistortion[] = {0, 0}; // RealSense
    double p1 = tangentialDistortion[0];
    double p2 = tangentialDistortion[1];

    // Format: [k1, k2, p1, p2, k3], see here: https://docs.opencv.org/master/d9/d0c/group__calib3d.html#ga3207604e4b1a1758aa66acb6ed5aa65d
    // Yes, k3 is really supposed to be after p2
    cv::Mat distCoeffs = (cv::Mat1d(1, 5) << k1, k2, p1, p2, k3);
    return distCoeffs;
}

TEST(DISABLED_CameraCalibration, undistort) {

    cv::VideoCapture capture(0);
    capture.set(cv::CAP_PROP_FRAME_WIDTH, 1280);
    capture.set(cv::CAP_PROP_FRAME_HEIGHT, 720);
    capture.set(cv::CAP_PROP_FPS, 30);
    if (!capture.isOpened()) {
        std::cerr << "ERROR! Unable to open camera\n";
    }

    cv::Mat frame;
    while (capture.read(frame)) {
        if (frame.empty()) {
            std::cerr << "ERROR! blank frame grabbed\n";
            break;
        }

        imshow("Live", frame);

        cv::Mat cameraMatrix = getCameraMatrix();
        cv::Mat distCoeffs = getDistCoeffs();

        cv::Mat undistorted;
        cv::undistort(frame, undistorted, cameraMatrix, distCoeffs);
        cv::imshow("Undistorted", undistorted);

        if (cv::waitKey(5) >= 0) break;
    }
}

TEST(ArUcoMarkers, estimatePosesOfSingleMarkers) {

    // Based on the tutorial on https://docs.opencv.org/4.5.1/d5/dae/tutorial_aruco_detection.html
    // and the tutorial on https://docs.opencv.org/4.5.1/db/da9/tutorial_aruco_board_detection.html
    const int nMarkers = 4;
    const int markerSize = 6;
    cv::Ptr<cv::aruco::Dictionary> dictionary = cv::aruco::generateCustomDictionary(nMarkers, markerSize);

    cv::VideoCapture capture(0);
    capture.set(cv::CAP_PROP_FRAME_WIDTH, 1280);
    capture.set(cv::CAP_PROP_FRAME_HEIGHT, 720);
    capture.set(cv::CAP_PROP_FPS, 30);
    if (!capture.isOpened()) {
        std::cerr << "ERROR! Unable to open camera\n";
    }

    cv::Mat frame;
    while (capture.read(frame)) {
        if (frame.empty()) {
            std::cerr << "ERROR! blank frame grabbed\n";
            break;
        }

        imshow("Live", frame);

        std::vector<int> markerIds;
        std::vector<std::vector<cv::Point2f>> markerCorners, rejectedCandidates;
        cv::Ptr<cv::aruco::DetectorParameters> parameters = cv::aruco::DetectorParameters::create();
        cv::aruco::detectMarkers(frame, dictionary, markerCorners, markerIds, parameters, rejectedCandidates);

        cv::Mat detection = frame.clone();
        if (!markerIds.empty()) {
            cv::aruco::drawDetectedMarkers(detection, markerCorners, markerIds);
        }
        cv::imshow("Detection", detection);

        cv::Mat cameraMatrix = getCameraMatrix();
        cv::Mat distCoeffs = getDistCoeffs();

        cv::Mat undistorted;
        cv::undistort(frame, undistorted, cameraMatrix, distCoeffs);
        cv::imshow("Undistorted", undistorted);

        if (!markerIds.empty()) {
            std::vector<cv::Vec3d> rvecs, tvecs;
            cv::aruco::estimatePoseSingleMarkers(markerCorners, 0.05, cameraMatrix, distCoeffs, rvecs, tvecs);

            cv::Mat poseEstimation = frame.clone();
            for (int i = 0; i < rvecs.size(); ++i) {
                auto rvec = rvecs[i];
                auto tvec = tvecs[i];
                cv::aruco::drawAxis(poseEstimation, cameraMatrix, distCoeffs, rvec, tvec, 0.1);
            }
            cv::imshow("Pose estimation", poseEstimation);
        }

        if (cv::waitKey(5) >= 0) break;
    }
}

TEST(ArUcoMarkers, realsense_estimatePosesOfSingleMarkers) {

    // Based on the tutorial on https://docs.opencv.org/4.5.1/d5/dae/tutorial_aruco_detection.html
    // and the tutorial on https://docs.opencv.org/4.5.1/db/da9/tutorial_aruco_board_detection.html
    const int nMarkers = 4;
    const int markerSize = 3;
    cv::Ptr<cv::aruco::Dictionary> dictionary = cv::aruco::generateCustomDictionary(nMarkers, markerSize);

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

//            cv::circle(resizedColor, cv::Point2i{resizedColor.cols / 2, resizedColor.rows / 2}, 3, cv::Scalar{255, 0, 0}, 1);
            cv::imshow("Color", resizedColor);

            cv::Mat frame;
            resizedColor.copyTo(frame);

            std::vector<int> markerIds;
            std::vector<std::vector<cv::Point2f>> markerCorners, rejectedCandidates;
            cv::Ptr<cv::aruco::DetectorParameters> parameters = cv::aruco::DetectorParameters::create();
            cv::aruco::detectMarkers(frame, dictionary, markerCorners, markerIds, parameters, rejectedCandidates);

            cv::Mat detection = frame.clone();
            if (!markerIds.empty()) {
                cv::aruco::drawDetectedMarkers(detection, markerCorners, markerIds);
            }
            cv::imshow("Detection", detection);

            cv::Mat cameraMatrix = getCameraMatrix();
            cv::Mat distCoeffs = getDistCoeffs();

            cv::Mat undistorted;
            cv::undistort(frame, undistorted, cameraMatrix, distCoeffs);
            cv::imshow("Undistorted", undistorted);

            if (!markerIds.empty()) {
                std::vector<cv::Vec3d> rvecs, tvecs;
                cv::aruco::estimatePoseSingleMarkers(markerCorners, 0.06, cameraMatrix, distCoeffs, rvecs, tvecs);

                cv::Mat poseEstimation = frame.clone();
                for (int i = 0; i < rvecs.size(); ++i) {
                    auto rvec = rvecs[i];
                    auto tvec = tvecs[i];
                    cv::aruco::drawAxis(poseEstimation, cameraMatrix, distCoeffs, rvec, tvec, 0.1);
                }
                cv::imshow("Pose estimation", poseEstimation);
            }

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

TEST(DISABLED_ArUcoMarkers, buildArUcoBoard) {

    // Based on the tutorial on https://docs.opencv.org/4.5.1/d5/dae/tutorial_aruco_detection.html
    // and the tutorial on https://docs.opencv.org/4.5.1/db/da9/tutorial_aruco_board_detection.html
    const int nMarkers = 4;
    const int markerSize = 6;
    cv::Ptr<cv::aruco::Dictionary> dictionary = cv::aruco::generateCustomDictionary(nMarkers, markerSize);

    const float side = 50; // length of marker in millimeters in the real world

    auto cornerPositions = [&side](const cv::Point3f& bottomLeft) -> std::vector<cv::Point3f> {
        return {
                bottomLeft + cv::Point3f{ 0, side, 0 },
                bottomLeft + cv::Point3f{ side, side, 0 },
                bottomLeft + cv::Point3f{ side, 0, 0 },
                bottomLeft + cv::Point3f{ 0, 0, 0 },
        };
    };

    const float separatorX = 50 + 60; // Horizontal distance between bottom-left points of each marker in millimeters
    const float separatorY = 50 + 180; // Vertical distance between bottom-left points of each marker in millimeters
    std::vector<std::vector<cv::Point3f>> objPoints = {
            cornerPositions(cv::Point3f{0, 0, 0}), // Marker 0
            cornerPositions(cv::Point3f{0, separatorY, 0}), // Marker 1
            cornerPositions(cv::Point3f{separatorX, separatorY, 0}), // Marker 2
            cornerPositions(cv::Point3f{separatorX, 0, 0}), // Marker 3
    };
    std::vector<int> ids;
    for (int i = 0; i < nMarkers; i++) {
        ids.push_back(i);
    }
    cv::Ptr<cv::aruco::Board> board = cv::aruco::Board::create(objPoints, dictionary, ids);

    cv::Mat test(1080/2, 1920/2, CV_32F);
    auto origin = cv::Point{(int) ((float)test.cols/2), (int) ((float)test.rows - (float)test.rows/8)};
    for (auto& marker : objPoints) {
        for(auto& corner: marker) {
            auto center = origin + cv::Point{(int) (corner.x), (int) (-corner.y)};
            std::cout << "draw circle at " << center.x << "," << center.y << std::endl;
            cv::circle(test, center, 5, cv::Scalar{0.8}, 2);
        }
        auto center = origin;
        std::cout << "draw circle at " << center.x << "," << center.y << std::endl;
        cv::circle(test, center, 5, cv::Scalar{1.0}, 2);
    }
    cv::imshow("test", test);
    cv::waitKey();
}

cv::Mat buildBoardToCameraMatrix(cv::Mat rotationMatrix, cv::Vec3d& tvec) {

    cv::Mat boardToCamera { cv::Size {4, 4}, CV_64F, cv::Scalar(0.0f) };
    for (int row = 0; row <= 2; row++) {
        for (int col = 0; col <= 2; col++) {
            boardToCamera.at<double>(row, col) = rotationMatrix.at<double>(row, col);
        }
    }
    for (int row = 0; row <= 2; row++) {
        boardToCamera.at<double>(row, 3) = tvec[row];
    }
    boardToCamera.at<double>(3, 3) = 1.0;

    return boardToCamera;
}

cv::Ptr<cv::aruco::Board> createArucoBoard(float markerLengthMilimeters) {
    // Based on the tutorial on https://docs.opencv.org/4.5.1/d5/dae/tutorial_aruco_detection.html
    const int nMarkers = 4;
    const int markerSize = 3;
    cv::Ptr<cv::aruco::Dictionary> dictionary = cv::aruco::generateCustomDictionary(nMarkers, markerSize);
    std::vector<int> ids = {0, 1, 2, 3};

    const float side = markerLengthMilimeters;

    auto cornerPositions = [&side](const cv::Point3f& bottomLeft) -> std::vector<cv::Point3f> {
        return {
                bottomLeft + cv::Point3f{ 0, side, 0 },
                bottomLeft + cv::Point3f{ side, side, 0 },
                bottomLeft + cv::Point3f{ side, 0, 0 },
                bottomLeft + cv::Point3f{ 0, 0, 0 },
        };
    };

    const float separatorX = 1725; // Horizontal distance between bottom-left points of each marker in millimeters
    const float separatorY = 1084; // Vertical distance between bottom-left points of each marker in millimeters
    cv::Point3f centerOffset{markerLengthMilimeters/2, markerLengthMilimeters/2, 0};
    std::vector<std::vector<cv::Point3f>> objPoints = {
            cornerPositions(cv::Point3f{0, 0, 0} - centerOffset),            // Marker 0
            cornerPositions(cv::Point3f{separatorX, 0, 0} - centerOffset),       // Marker 1
            cornerPositions(cv::Point3f{separatorX, separatorY, 0} - centerOffset), // Marker 2
            cornerPositions(cv::Point3f{0, separatorY, 0} - centerOffset),       // Marker 3
    };

    cv::Ptr<cv::aruco::Board> board = cv::aruco::Board::create(objPoints, dictionary, ids);
    return board;
}

int mouseX = -1;
int mouseY = -1;
bool pointChanged = true;

static void onMouse( int event, int x, int y, int, void* ) {
    if (event != cv::EVENT_LBUTTONDOWN) {
        return;
    }
    mouseX = x;
    mouseY = y;
    pointChanged = true;
}

/**
 * RealSense D435
 */
billiard::detection::CameraIntrinsics getIntrinsics() {
    billiard::detection::CameraIntrinsics intrinsics;
    intrinsics.cameraMatrix = getCameraMatrix();
    intrinsics.distCoeffs = getDistCoeffs();
    intrinsics.sensorSize = {0.0014, 0.0014 };
    return intrinsics;
}

TEST(ArUcoMarkers, realsense_tryout_refactored) {

    using namespace billiard::detection;

    // Based on the tutorial on https://docs.opencv.org/4.5.1/d5/dae/tutorial_aruco_detection.html
    // and the tutorial on https://docs.opencv.org/4.5.1/db/da9/tutorial_aruco_board_detection.html
    const float markerLength = 50; // length of marker in millimeters in the real world

    const float separatorX = 1725; // Horizontal distance between bottom-left points of each marker in millimeters
    const float separatorY = 1084; // Vertical distance between bottom-left points of each marker in millimeters
    cv::Point3f center {separatorX/2, separatorY/2, 0};

    cv::Ptr<cv::aruco::Board> board = createArucoBoard(markerLength);
    CameraIntrinsics intrinsics = getIntrinsics();

    double scale = 0.5;

    billiard::capture::CameraCapture capture {};

    if (capture.open()) {

        cv::namedWindow("Color", cv::WINDOW_AUTOSIZE);
        cv::namedWindow("Depth", cv::WINDOW_AUTOSIZE);

        while (true) {
            billiard::capture::CameraFrames frames = capture.read();

            if (!frames.depth.empty()) {
                cv::imshow("Depth", frames.colorizedDepth);
            }
            cv::Mat resizedColor;
            cv::resize(frames.color, resizedColor, cv::Size(0,0), scale, scale);
            cv::imshow("Color", resizedColor);

            cv::Mat frame;
            frames.color.copyTo(frame);

            imshow("Live", frame);

            CameraToWorldCoordinateSystemConfig config = billiard::detection::configure(frame, board, intrinsics);
            if (config.valid) {

                // Input
                cv::Point2d imagePoint;

                // Display world-point in image
                cv::Mat boardPoseEstimation = frame.clone();
                {
                    drawAxis(boardPoseEstimation, config.pose, config.intrinsics);
                    std::vector<cv::Point2d> imagePoints = worldPointsToImagePoints(config, { center });
                    cv::circle(boardPoseEstimation, imagePoints[0], 5, cv::Scalar{255, 0, 0}, 1);
                    cv::imshow("Board pose estimation", boardPoseEstimation);

                    imagePoint = imagePoints[0];
                }

                // Image-point to world-point
                Plane plane {{0, 0, 13.3}, {0, 0, 1}};
                std::vector<cv::Point3d> worldPoints = imagePointsToWorldPoints(config, plane, {imagePoint});

                // Display world-point in image
                {
                    std::vector<cv::Point2d> imagePoints = worldPointsToImagePoints(config, { worldPoints[0] });
                    cv::circle(boardPoseEstimation, imagePoints[0], 5, cv::Scalar{0, 255, 0}, 1);
                    cv::Mat boardPoseEstimationOutput;
                    cv::resize(boardPoseEstimation, boardPoseEstimationOutput, cv::Size(), 0.5, 0.5);
                    cv::imshow("Board pose estimation", boardPoseEstimationOutput);

                    std::cout << "world point: " << worldPoints[0] << " should be at " << center << std::endl;
                }

                int key = cv::waitKey(5);
                if (key == 32 /* SPACE */) {
                    cv::waitKey();
                    cv::destroyAllWindows();

                    cv::namedWindow("Result");
                    cv::setMouseCallback("Result", onMouse, nullptr);

                    // Live
                    cv::Mat output;
                    while (true) {

                        int key = cv::waitKey(50);
                        if (key == 27 /* ESC */) break;
                        if (pointChanged) {
                            pointChanged = false;
                            cv::Point mousePoint{mouseX, mouseY};
                            cv::Point2d point{mouseX * 1 / scale, mouseY * 1 / scale};

                            cv::resize(boardPoseEstimation, output, cv::Size(), scale, scale);
                            if (point.x >= 0 && point.y >= 0) {
                                cv::circle(output, mousePoint, 2, cv::Scalar{0, 255, 0}, 1);
                                std::vector<cv::Point3d> worldPoints = imagePointsToWorldPoints(config, plane, {point});
                                std::vector<cv::Point2d> imagePoints = worldPointsToImagePoints(config,{worldPoints[0]});
                                cv::circle(output, imagePoints[0] * scale, 5, cv::Scalar{255, 0, 0}, 1);

                                std::cout << "image point: " << point << " world point: " << worldPoints[0]
                                          << " image point: " << imagePoints[0] << std::endl;
                            }
                            cv::imshow("Result", output);
                        }
                    }
                }
            }

            int key = cv::waitKey(5);
            if (key == 27 /* ESC */) {
                break;
            }
        }
        capture.close();
    }
}

TEST(ArUcoMarkers, tryout_refactored) {

    using namespace billiard::detection;

    // Based on the tutorial on https://docs.opencv.org/4.5.1/d5/dae/tutorial_aruco_detection.html
    // and the tutorial on https://docs.opencv.org/4.5.1/db/da9/tutorial_aruco_board_detection.html
    const float markerLength = 50; // length of marker in millimeters in the real world

    const float side = markerLength;
    const float separatorX = 1.725*1000; // Horizontal distance between bottom-left points of each marker in millimeters
    const float separatorY = 1.084*1000; // Vertical distance between bottom-left points of each marker in millimeters
    cv::Point3f center {side/2 + separatorX/2, side/2 + separatorY/2, 0};

    cv::Ptr<cv::aruco::Board> board = createArucoBoard(markerLength);

    cv::Mat frame = cv::imread("D:\\Dev\\billiard-ai\\cmake-build-debug\\test\\billiard_capture\\aruco_board.png");

    imshow("Live", frame);

    CameraIntrinsics intrinsics;
    intrinsics.cameraMatrix = getCameraMatrix();
    intrinsics.distCoeffs = getDistCoeffs();
    intrinsics.sensorSize = {0.0014, 0.0014 };

    CameraToWorldCoordinateSystemConfig config = billiard::detection::configure(frame, board, intrinsics);
    if (config.valid) {

        // Input
        cv::Point2d imagePoint;

        // Display world-point in image
        cv::Mat boardPoseEstimation = frame.clone();
        {
            std::vector<cv::Point2d> imagePoints = worldPointsToImagePoints(config, { center });
            cv::circle(boardPoseEstimation, imagePoints[0], 5, cv::Scalar{255, 0, 0}, 1);
            cv::imshow("Board pose estimation", boardPoseEstimation);

            imagePoint = imagePoints[0];
        }

        // Image-point to world-point
        Plane plane {{0, 0, 0}, {0, 0, 1}};
        std::vector<cv::Point3d> worldPoints = imagePointsToWorldPoints(config, plane, {imagePoint});

        // Display world-point in image
        {
            std::vector<cv::Point2d> imagePoints = worldPointsToImagePoints(config, { worldPoints[0] });
            cv::circle(boardPoseEstimation, imagePoints[0], 5, cv::Scalar{0, 255, 0}, 1);
            cv::Mat boardPoseEstimationOutput;
            cv::resize(boardPoseEstimation, boardPoseEstimationOutput, cv::Size(), 0.5, 0.5);
            cv::imshow("Board pose estimation", boardPoseEstimationOutput);

            std::cout << "world point: " << worldPoints[0] << " should be at " << center << std::endl;
        }

        // Convert world-point to different frame of reference


        cv::waitKey();
        cv::destroyAllWindows();

        cv::namedWindow("Result");
        cv::setMouseCallback("Result", onMouse, nullptr);
        double scale = 0.5;

        // Live
        cv::Mat output;
        while(true) {

            int key = cv::waitKey(50);
            if (key == 27 /* ESC */) break;
            if (pointChanged) {
                pointChanged = false;
                cv::Point mousePoint {mouseX, mouseY};
                cv::Point2d point {mouseX * 1/scale, mouseY * 1/scale};

                cv::resize(boardPoseEstimation, output, cv::Size(), scale, scale);
                if (point.x >= 0 && point.y >= 0) {
                    cv::circle(output, mousePoint, 2, cv::Scalar{0, 255, 0}, 1);
                    std::vector<cv::Point3d> worldPoints = imagePointsToWorldPoints(config, plane, {point});
                    std::vector<cv::Point2d> imagePoints = worldPointsToImagePoints(config, { worldPoints[0] });
                    cv::circle(output, imagePoints[0] * scale, 5, cv::Scalar{255, 0, 0}, 1);

                    std::cout << "image point: " << point << " world point: " << worldPoints[0] << " image point: " << imagePoints[0] << std::endl;
                }
                cv::imshow("Result", output);
            }
        }
    }
}

TEST(ArUcoMarkers, tryout) {

    // Based on the tutorial on https://docs.opencv.org/4.5.1/d5/dae/tutorial_aruco_detection.html
    // and the tutorial on https://docs.opencv.org/4.5.1/db/da9/tutorial_aruco_board_detection.html
    const float markerLength = 50; // length of marker in millimeters in the real world

    const float side = markerLength;
    const float separatorX = 140; // Horizontal distance between bottom-left points of each marker in millimeters
    const float separatorY = 227; // Vertical distance between bottom-left points of each marker in millimeters
    cv::Point3f center {side/2 + separatorX/2, side/2 + separatorY/2, 0};

    cv::Ptr<cv::aruco::Board> board = createArucoBoard(markerLength);

    cv::Mat frame = cv::imread("D:\\Dev\\billiard-ai\\cmake-build-debug\\test\\billiard_capture\\aruco_board.png");

    imshow("Live", frame);

    std::vector<int> markerIds;
    std::vector<std::vector<cv::Point2f>> markerCorners, rejectedCandidates;
    cv::Ptr<cv::aruco::DetectorParameters> parameters = cv::aruco::DetectorParameters::create();
    cv::aruco::detectMarkers(frame, board->dictionary, markerCorners, markerIds, parameters, rejectedCandidates);

    cv::Mat detection = frame.clone();
    if (!markerIds.empty()) {
        cv::aruco::drawDetectedMarkers(detection, markerCorners, markerIds);
    }
    cv::imshow("Detection", detection);

    cv::Mat cameraMatrix = getCameraMatrix();
    cv::Mat distCoeffs = getDistCoeffs();

    cv::Mat undistorted;
    cv::undistort(frame, undistorted, cameraMatrix, distCoeffs);
    cv::imshow("Undistorted", undistorted);

    if (!markerIds.empty()) {
//        std::vector<cv::Vec3d> rvecs, tvecs;
//        cv::aruco::estimatePoseSingleMarkers(markerCorners, markerLength, cameraMatrix, distCoeffs, rvecs, tvecs);
//
//        cv::Mat poseEstimation = frame.clone();
//        for (int i = 0; i < rvecs.size(); ++i) {
//            auto rvec = rvecs[i];
//            auto tvec = tvecs[i];
//            cv::aruco::drawAxis(poseEstimation, cameraMatrix, distCoeffs, rvec, tvec, 100);
//        }
//        cv::imshow("Markers pose estimation", poseEstimation);

        cv::Mat boardPoseEstimation = frame.clone();
        cv::Vec3d rvec, tvec;
        int valid = cv::aruco::estimatePoseBoard(markerCorners, markerIds, board, cameraMatrix, distCoeffs, rvec, tvec);
        if (valid > 0) {
            cv::aruco::drawAxis(boardPoseEstimation, cameraMatrix, distCoeffs, rvec, tvec, 100);
            cv::imshow("Board pose estimation", boardPoseEstimation);

            // Display world-point in image
            std::vector<cv::Point3f> worldPoints = { center };
            std::vector<cv::Point2f> imagePoints;
            cv::projectPoints(worldPoints, rvec, tvec, cameraMatrix, distCoeffs, imagePoints);

            cv::circle(boardPoseEstimation, imagePoints[0], 5, cv::Scalar{255, 0, 0}, 1);
            cv::imshow("Board pose estimation", boardPoseEstimation);

            // Calculate camera world position
            // See: https://stackoverflow.com/a/12977143
            double theta = cv::norm(rvec); // in radians
            double thetaDegrees = theta * 180.0/CV_PI;
            cv::Vec3d axis = rvec / theta;
            std::cout << "Board pose estimation: rotation: " << thetaDegrees << " degrees around " << axis << " translation: " << tvec << " rvec: " << rvec << std::endl;
            // According to documentation of estimatePoseBoard:
            // The returned transformation is the one that transforms points from the
            // board coordinate system to the camera coordinate system.
            cv::Mat rotationMatrix;
            cv::Rodrigues(rvec, rotationMatrix);
            std::cout << "Probably equivalent rotation matrix: " << rotationMatrix << std::endl;

            cv::Mat boardToCamera = buildBoardToCameraMatrix(rotationMatrix, tvec);
            cv::Mat cameraToBoard;
            cv::invert(boardToCamera, cameraToBoard);

            std::cout << "board 2 camera: " << boardToCamera << std::endl;
            std::cout << "camera 2 board: " << cameraToBoard << std::endl;

            cv::Vec4d cameraInCameraCoordinates {0, 0, 0, 1};
            cv::Mat cameraInWorldCoordinatesMatHomogeneous = cameraToBoard * cameraInCameraCoordinates;
            cv::Mat cameraInWorldCoordinatesMat = cameraInWorldCoordinatesMatHomogeneous(cv::Rect{0, 0, 1, 3});
            std::cout << "cameraInWorldCoordinatesMatHomogeneous: " << cameraInWorldCoordinatesMatHomogeneous << " cameraInWorldCoordinatesMat: " << cameraInWorldCoordinatesMat << std::endl;
            cv::Vec3d cameraPosInWorldCoordinates(cameraInWorldCoordinatesMat);
            std::cout << "Camera is at " << cameraPosInWorldCoordinates << " in the real world" << std::endl;

            // Convert image point to world-point
            cv::Point2d s {0.003, 0.003 }; // Horizontal/Vertical size of a sensor pixel in milimeters
            // Principal point c
            cv::Point2d c { cameraMatrix.at<double>(0,2), cameraMatrix.at<double>(1,2) };
            double f = cameraMatrix.at<double>(0,0) * s.x;

            cv::Point2d ip = imagePoints[0];

            cv::Point2d imagePointInImageCoordinates { ip.x, ip.y };
            cv::Vec3d imagePointInCameraCoordinates { (imagePointInImageCoordinates.x - c.x) * s.x,
                                                      (imagePointInImageCoordinates.y - c.y) * s.y,
                                                      f };
            cv::Vec4d imagePointInCameraCoordinatesHomogenous { (imagePointInImageCoordinates.x - c.x) * s.x,
                                                      (imagePointInImageCoordinates.y - c.y) * s.y,
                                                      f,
                                                      1};

            cv::Mat imagePointInWorldCoordinates = rotationMatrix.t() * (imagePointInCameraCoordinates - tvec);
            cv::Mat imagePointInWorldCoordinates2 = cameraToBoard * imagePointInCameraCoordinatesHomogenous;

            std::cout << "f: " << std::to_string(f) << " s: " << s << " c: " << c << std::endl;
            std::cout << "image: " << imagePointInImageCoordinates << " camera: " << imagePointInCameraCoordinates << " world: " << imagePointInWorldCoordinates << std::endl;
            std::cout << "imagePointInWorldCoordinates: " << imagePointInWorldCoordinates << " imagePointInWorldCoordinates2: " << imagePointInWorldCoordinates2 << std::endl;

            // Define Real world plane
            double zDelta = 0.0;// Difference of the plane in height to the height of the aruco markers, negative means lower, positive means higher
            cv::Vec3d planePoint {0, 0, 0};
            cv::Vec3d planeNormal {0, 0, 1};

            // Define Line
            cv::Vec3d linePoint = cameraPosInWorldCoordinates;
            cv::Mat lineDirection = imagePointInWorldCoordinates - cameraInWorldCoordinatesMat;

            std::cout << "dir = " << "ip: " << imagePointInWorldCoordinates << " - " << cameraInWorldCoordinatesMat << std::endl;

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

            cv::Mat worldPointMat = cv::Mat(linePoint) + (lambda * lineDirection);
            cv::Point3d worldPoint(worldPointMat);

            std::cout << "Line: " << linePoint << " + " << std::to_string(lambda) << " * " << lineDirection << std::endl;
            std::cout << "Plane: (p - " << planePoint << ") * " << planeNormal << " = 0" << std::endl;
            std::cout << "world point: " << worldPoint << " should be at " << center << std::endl;
            std::cout << "" << std::endl;

            // Display world-point in image
            std::vector<cv::Point3d> worldPoints2 = { worldPoint };
            std::vector<cv::Point2d> imagePoints2;
            cv::projectPoints(worldPoints2, rvec, tvec, cameraMatrix, distCoeffs, imagePoints2);

            cv::circle(boardPoseEstimation, imagePoints2[0], 5, cv::Scalar{0, 255, 0}, 1);
            cv::Mat boardPoseEstimationOutput;
            cv::resize(boardPoseEstimation, boardPoseEstimationOutput, cv::Size(), 0.5, 0.5);
            cv::imshow("Board pose estimation", boardPoseEstimationOutput);
        }

    }

    cv::waitKey();
}

TEST(ArUcoMarkers, tryout_live) {

    // Based on the tutorial on https://docs.opencv.org/4.5.1/d5/dae/tutorial_aruco_detection.html
    // and the tutorial on https://docs.opencv.org/4.5.1/db/da9/tutorial_aruco_board_detection.html
    const int nMarkers = 4;
    const int markerSize = 6;
    cv::Ptr<cv::aruco::Dictionary> dictionary = cv::aruco::generateCustomDictionary(nMarkers, markerSize);

    const float side = 50; // length of marker in millimeters in the real world
    const float markerLength = side;

    auto cornerPositions = [&side](const cv::Point3f& bottomLeft) -> std::vector<cv::Point3f> {
        return {
                bottomLeft + cv::Point3f{ 0, side, 0 },
                bottomLeft + cv::Point3f{ side, side, 0 },
                bottomLeft + cv::Point3f{ side, 0, 0 },
                bottomLeft + cv::Point3f{ 0, 0, 0 },
        };
    };

    const float separatorX = 140; // Horizontal distance between bottom-left points of each marker in millimeters
    const float separatorY = 227; // Vertical distance between bottom-left points of each marker in millimeters
    std::vector<std::vector<cv::Point3f>> objPoints = {
            cornerPositions(cv::Point3f{0, 0, 0}),            // Marker 0
            cornerPositions(cv::Point3f{0, separatorY, 0}),       // Marker 1
            cornerPositions(cv::Point3f{separatorX, separatorY, 0}), // Marker 2
            cornerPositions(cv::Point3f{separatorX, 0, 0}),       // Marker 3
    };

    cv::Point3f center {side/2 + separatorX/2, side/2 + separatorY/2, 0};

    std::vector<int> ids;
    for (int i = 0; i < nMarkers; i++) {
        ids.push_back(i);
    }
    cv::Ptr<cv::aruco::Board> board = cv::aruco::Board::create(objPoints, dictionary, ids);

    cv::VideoCapture capture(0);
    capture.set(cv::CAP_PROP_FRAME_WIDTH, 1280);
    capture.set(cv::CAP_PROP_FRAME_HEIGHT, 720);
    capture.set(cv::CAP_PROP_FPS, 30);
    if (!capture.isOpened()) {
        std::cerr << "ERROR! Unable to open camera\n";
    }

    cv::Mat frame;
    while (capture.read(frame)) {
        if (frame.empty()) {
            std::cerr << "ERROR! blank frame grabbed\n";
            break;
        }

        imshow("Live", frame);

        std::vector<int> markerIds;
        std::vector<std::vector<cv::Point2f>> markerCorners, rejectedCandidates;
        cv::Ptr<cv::aruco::DetectorParameters> parameters = cv::aruco::DetectorParameters::create();
        cv::aruco::detectMarkers(frame, dictionary, markerCorners, markerIds, parameters, rejectedCandidates);

        cv::Mat detection = frame.clone();
        if (!markerIds.empty()) {
            cv::aruco::drawDetectedMarkers(detection, markerCorners, markerIds);
        }
        cv::imshow("Detection", detection);

        cv::Mat cameraMatrix = getCameraMatrix();
        cv::Mat distCoeffs = getDistCoeffs();

        cv::Mat undistorted;
        cv::undistort(frame, undistorted, cameraMatrix, distCoeffs);
        cv::imshow("Undistorted", undistorted);

        if (!markerIds.empty()) {
            std::vector<cv::Vec3d> rvecs, tvecs;
            cv::aruco::estimatePoseSingleMarkers(markerCorners, markerLength, cameraMatrix, distCoeffs, rvecs, tvecs);

            cv::Mat poseEstimation = frame.clone();
            for (int i = 0; i < rvecs.size(); ++i) {
                auto rvec = rvecs[i];
                auto tvec = tvecs[i];
                cv::aruco::drawAxis(poseEstimation, cameraMatrix, distCoeffs, rvec, tvec, 100);
            }
            cv::imshow("Markers pose estimation", poseEstimation);

            cv::Mat boardPoseEstimation = frame.clone();
            cv::Vec3d rvec, tvec;
            int valid = cv::aruco::estimatePoseBoard(markerCorners, markerIds, board, cameraMatrix, distCoeffs, rvec, tvec);
            if (valid > 0) {
                cv::aruco::drawAxis(boardPoseEstimation, cameraMatrix, distCoeffs, rvec, tvec, 100);
                cv::imshow("Board pose estimation", boardPoseEstimation);

                // Display world-point in image
                std::vector<cv::Point3f> worldPoints = { center };
                std::vector<cv::Point2f> imagePoints;
                cv::projectPoints(worldPoints, rvec, tvec, cameraMatrix, distCoeffs, imagePoints);

                cv::circle(boardPoseEstimation, imagePoints[0], 5, cv::Scalar{255, 0, 0}, 1);
                cv::imshow("Board pose estimation", boardPoseEstimation);

                // Calculate camera world position
                // See: https://stackoverflow.com/a/12977143
                double theta = cv::norm(rvec); // in radians
                double thetaDegrees = theta * 180.0/CV_PI;
                cv::Vec3d axis = rvec / theta;
                std::cout << "Board pose estimation: rotation: " << thetaDegrees << " degrees around " << axis << " translation: " << tvec << " rvec: " << rvec << std::endl;
                // According to documentation of estimatePoseBoard:
                // The returned transformation is the one that transforms points from the
                // board coordinate system to the camera coordinate system.
                cv::Mat rotationMatrix;
                cv::Rodrigues(rvec, rotationMatrix);
                std::cout << "Probably equivalent rotation matrix: " << rotationMatrix << std::endl;

                cv::Mat boardToCamera = buildBoardToCameraMatrix(rotationMatrix, tvec);
                cv::Mat cameraToBoard;// = buildCameraToBoardMatrix(rotationMatrix, tvec);
                cv::invert(boardToCamera, cameraToBoard);

                std::cout << "board 2 camera: " << boardToCamera << std::endl;
                std::cout << "camera 2 board: " << cameraToBoard << std::endl;

                cv::Vec4d cameraInCameraCoordinates {0, 0, 0, 1};
                cv::Mat cameraInWorldCoordinatesMatHomogeneous = cameraToBoard * cameraInCameraCoordinates;
                cv::Mat cameraInWorldCoordinatesMat = cameraInWorldCoordinatesMatHomogeneous(cv::Rect{0, 0, 1, 3});
                std::cout << "cameraInWorldCoordinatesMat: " << cameraInWorldCoordinatesMat << std::endl;
                cv::Vec3d cameraPosInWorldCoordinates { cameraInWorldCoordinatesMatHomogeneous.at<double>(0), cameraInWorldCoordinatesMatHomogeneous.at<double>(1), cameraInWorldCoordinatesMatHomogeneous.at<double>(2) };
                std::cout << "Camera is at " << cameraPosInWorldCoordinates << " in the real world" << std::endl;

                // Convert image point to world-point
                cv::Point2d s {0.003, 0.003 }; // Horizontal/Vertical size of a sensor pixel in milimeters
                // Principal point c
                cv::Point2d c { cameraMatrix.at<double>(0,2), cameraMatrix.at<double>(1,2) };
                double f = cameraMatrix.at<double>(0,0) * s.x;

                cv::Point2d ip = imagePoints[0];

                cv::Point2d imagePointInImageCoordinates { ip.x, ip.y };
                cv::Vec3d imagePointInCameraCoordinates { (imagePointInImageCoordinates.x - c.x) * s.x,
                                                          (imagePointInImageCoordinates.y - c.y) * s.y,
                                                          f };

                cv::Mat imagePointInWorldCoordinates = rotationMatrix.t() * (imagePointInCameraCoordinates - tvec);

                std::cout << "f: " << std::to_string(f) << " s: " << s << " c: " << c << std::endl;
                std::cout << "image: " << imagePointInImageCoordinates << " camera: " << imagePointInCameraCoordinates << " world: " << imagePointInWorldCoordinates << std::endl;

                // Define Real world plane
                double zDelta = 0.0;// Difference of the plane in height to the height of the aruco markers, negative means lower, positive means higher
                cv::Vec3d planePoint {0, 0, 0};
                cv::Vec3d planeNormal {0, 0, 1};

                // Define Line
                cv::Vec3d linePoint = cameraPosInWorldCoordinates;
                cv::Mat lineDirection = imagePointInWorldCoordinates - cameraInWorldCoordinatesMat;

                std::cout << "dir = " << "ip: " << imagePointInWorldCoordinates << " - " << cameraInWorldCoordinatesMat << std::endl;

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

                cv::Mat worldPointMat = cv::Mat(linePoint) + (lambda * lineDirection);
                cv::Point3f worldPoint { (float)worldPointMat.at<double>(0), (float)worldPointMat.at<double>(1), (float)worldPointMat.at<double>(2) };

                std::cout << "Line: " << linePoint << " + " << std::to_string(lambda) << " * " << lineDirection << std::endl;
                std::cout << "Plane: (p - " << planePoint << ") * " << planeNormal << " = 0" << std::endl;
                std::cout << "world point: " << worldPoint << " should be at " << center << std::endl;
                std::cout << "" << std::endl;

                // Display world-point in image
                std::vector<cv::Point3f> worldPoints2 = { worldPoint };
                std::vector<cv::Point2f> imagePoints2;
                cv::projectPoints(worldPoints2, rvec, tvec, cameraMatrix, distCoeffs, imagePoints2);

                cv::circle(boardPoseEstimation, imagePoints2[0], 5, cv::Scalar{0, 255, 0}, 1);
                cv::Mat boardPoseEstimationOutput;
                cv::resize(boardPoseEstimation, boardPoseEstimationOutput, cv::Size(), 0.5, 0.5);
                cv::imshow("Board pose estimation", boardPoseEstimationOutput);

            }

        }

        if (cv::waitKey(5) >= 0) break;
//    cv::waitKey();
    }
}
