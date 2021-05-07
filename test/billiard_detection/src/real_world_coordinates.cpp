#include <gtest/gtest.h>
#include <billiard_detection/billiard_detection.hpp>

cv::Ptr<cv::aruco::Board> createArucoBoard2(float markerLengthMilimeters) {
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

billiard::detection::CameraIntrinsics getIntrinsics2() {
    return billiard::detection::CameraIntrinsics {
            cv::Point2d { 1375.69, 1375.85 },
            cv::Point2d { 974.842, 539.363 },
            0.0,
            cv::Point3d { 0.0, 0.0, 0.0 },
            cv::Point2d { 0.0, 0.0 },
            cv::Point2d { 0.0014, 0.0014 }
    };
}


TEST(Detection, image_coordinates_to_world_coordinates) {

    using namespace billiard::detection;

    // Config
    double innerTableLength = 1889.0; // millimeters
    double innerTableWidth  =  951;   // millimeters
    double ballDiameter     = 52.3;   // millimeters
    double ballRadius       = ballDiameter/2; // millimeters
    cv::Point2d innerTableCenter { innerTableLength/2, innerTableWidth/2 };

    const float markerLength = 50; // length of marker in millimeters in the real world
    cv::Ptr<cv::aruco::Board> board = createArucoBoard2(markerLength);
    CameraIntrinsics intrinsics = getIntrinsics2();
    Plane plane {{0, 0, 13.3 - ballRadius}, {0, 0, 1}};

    cv::Vec3d worldToRail { 78.5, -71, 0.0 };
    cv::Vec3d railToModel { innerTableCenter.x, innerTableCenter.y, 0.0 };

    WorldToModelCoordinates worldToModel;
    worldToModel.translation = worldToRail - railToModel;

    int balls = 7;
    std::vector<cv::Point2d> expectedImagePoints = {
            cv::Point2d { 166, 957 }, // Ball 1
            cv::Point2d { 929, 926 }, // Ball 2
            cv::Point2d { 163, 150 }, // Ball 3
            cv::Point2d { 931, 170 }, // Ball 4
            cv::Point2d { 1743, 226 }, // Ball 5
            cv::Point2d { 1738, 864 }, // Ball 6
            cv::Point2d { 637, 524 }  // Ball 7
    };
    std::vector<cv::Point2d> expectedModelPoints = {
            // Ball 1
            cv::Point2d { innerTableLength - (1730+49+ballRadius),innerTableWidth - (850+49+ballRadius) } - innerTableCenter,
            // Ball 2
            cv::Point2d { innerTableLength - (880+49+ballRadius),innerTableWidth - (830+49+ballRadius) } - innerTableCenter,
            // Ball 3
            cv::Point2d { innerTableLength - (1721+49+ballRadius),850+49+ballRadius } - innerTableCenter,
            // Ball 4
            cv::Point2d { innerTableLength - (877+49+ballRadius),833+49+ballRadius } - innerTableCenter,
            // Ball 5
            cv::Point2d { 1785+49+ballRadius,766+49+ballRadius } - innerTableCenter,
            // Ball 6
            cv::Point2d { 1786+49+ballRadius,innerTableWidth - (763+49+ballRadius) } - innerTableCenter,
            // Ball 7
            cv::Point2d { (540+49+ballRadius),innerTableWidth - (373+49+ballRadius) } - innerTableCenter,
    };

    double totalAbsoluteDist = 0.0;

    for(int ball = 1; ball <= balls; ball++) {
        std::stringstream filePath;
        filePath << "./resources/test_real_world_coordinates/real_world_" << std::to_string(ball) << ".png";
        cv::Mat frame = cv::imread(filePath.str());
        cv::Point2d imagePoint = expectedImagePoints[ball-1];
        cv::Point2d expectedModelPoint = expectedModelPoints[ball-1];

        double scale = 0.5;
        cv::Mat resizedColor;
        cv::resize(frame, resizedColor, cv::Size(), scale, scale);
        cv::imshow("Color", resizedColor);

        int pixelRadius = 30;
        cv::Mat frameCopy;
        frame.copyTo(frameCopy);
        cv::circle(frameCopy, imagePoint, 1, cv::Scalar{0, 255, 0}, 1);
        cv::Mat ballImage = frameCopy(cv::Rect{(int)imagePoint.x - pixelRadius, (int)imagePoint.y - pixelRadius, 2*pixelRadius, 2*pixelRadius});
        cv::Mat ballImageScaled;
        cv::resize(ballImage, ballImageScaled, cv::Size(), 4.0, 4.0);
        cv::imshow("Ball truth", ballImageScaled);

        CameraToWorldCoordinateSystemConfig config = configure(frame, board, intrinsics);
        if (config.valid) {

            std::vector<cv::Point3d> worldPoints = imagePointsToWorldPoints(config, plane, {imagePoint});
            std::vector<cv::Point2d> imagePoints = worldPointsToImagePoints(config, { worldPoints[0] });
            std::vector<cv::Point2d> modelPoints = worldPointsToModelPoints(worldToModel, { worldPoints[0] });

            cv::Mat output = frame.clone();
            cv::circle(output, imagePoints[0], 5, cv::Scalar{0, 255, 0}, 1);
            cv::resize(output, output, cv::Size(), scale, scale);

            cv::imshow("Output", output);
            std::cout << "image point: " << imagePoints[0] << " should be at: " << imagePoint << std::endl;
            std::cout << "world point: " << worldPoints[0] << std::endl;
            std::cout << "model point: " << modelPoints[0] << " should be at " << expectedModelPoint << " dist: " << (modelPoints[0] - expectedModelPoint) << " dist: " << cv::norm(modelPoints[0] - expectedModelPoint) << std::endl;

            totalAbsoluteDist += cv::norm(modelPoints[0] - expectedModelPoint);

        }

        cv::waitKey();
    }
    std::cout << "total absolute distance: " << std::to_string(totalAbsoluteDist) << std::endl;
}
