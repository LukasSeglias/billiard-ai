#include <gtest/gtest.h>
#include <billiard_detection/billiard_detection.hpp>
#include <billiard_snooker/billiard_snooker.hpp>
#include "config.hpp"

struct Capture {
    unsigned long long frameIndex;
    glm::vec2 pixelPosition;
    glm::vec2 modelPosition;
};

struct Context {
    unsigned long long frameIndex = 0;
    std::vector<Capture> captures;
    std::shared_ptr<billiard::detection::DetectionConfig> detectionConfig;
};

std::ostream& operator<<(std::ostream& os, const glm::vec2& vector) {
    os << "(" << vector.x << ", " << vector.y << ")";
    return os;
}

void onMouseClick(int event, int x, int y, int flags, void *userdata);

TEST(SimulationVsReality, video_analysis) {

    std::string videoPath = "./resources/simulation_vs_reality/13_0030_0034.avi";

    cv::Size imageSize = getImageSize();
    billiard::detection::Table table = getTable();
    billiard::detection::ArucoMarkers markers = getMarkers();
    billiard::detection::CameraIntrinsics intrinsics = getIntrinsics_realsense_hd();

    bool imageChanged = true;

    std::shared_ptr<billiard::detection::DetectionConfig> detectionConfig;

    cv::VideoCapture video {videoPath};
    if (!video.isOpened()) {
        std::cout << "Unable to open video" << std::endl;
        return;
    }

    std::vector<cv::Mat> frames {};

    while (true) {
        cv::Mat temp;
        if (!video.read(temp)) {
            break;
        }
        frames.push_back(temp);
    }

    std::cout << "Frames read: " << frames.size() << std::endl;

    if (frames.empty()) {
        std::cout << "No frames read" << std::endl;
        return;
    }

    detectionConfig = std::make_shared<billiard::detection::DetectionConfig>(billiard::detection::configure(frames.at(0), table, markers, intrinsics));
    if (!detectionConfig->valid) {
        std::cout << "Unable to configure detection" << std::endl;
        return;
    }

    if (!billiard::snooker::configure(*detectionConfig)) {
        std::cout << "Unable to configure snooker detection" << std::endl;
        return;
    }

    Context context {};
    context.detectionConfig = detectionConfig;
    context.frameIndex = 0;

    cv::namedWindow("Frame");
    cv::setMouseCallback("Frame", onMouseClick, &context);

    while(true) {
        if (imageChanged) {
            imageChanged = false;

            cv::Mat frame = frames.at(context.frameIndex);
            cv::resize(frame, frame, imageSize);
            cv::imshow("Frame", frame);

//            billiard::detection::State pixelState = billiard::snooker::detect(billiard::detection::State(), frame);
//            billiard::detection::State state = billiard::detection::pixelToModelCoordinates(*detectionConfig, pixelState);
//
//            for (auto& ball : state._balls) {
//
//                cv::Point2d modelPoint = cv::Point2d(ball._position.x, ball._position.y);
//                std::cout << "model point: " << modelPoint << std::endl;
//            }

        }

        char key = (char) cv::waitKey(1);
        if (key == 27 /* ESC */) {
            break;
        } else if (key == 97 /* A */) {
            context.frameIndex = context.frameIndex == 0 ? frames.size() - 1 : context.frameIndex - 1;
            imageChanged = true;
        } else if (key == 100 /* D */) {
            context.frameIndex = (context.frameIndex + 1) % (frames.size());
            imageChanged = true;
        } else if (key == 'q' /* Q */) {
            context.frameIndex = context.frameIndex >= 10 ? context.frameIndex - 10 : frames.size() - 1;
            imageChanged = true;
        } else if (key == 'e' /* E */) {
            context.frameIndex = (context.frameIndex + 10) % (frames.size());
            imageChanged = true;
        } else if (key == ' ') {
            if (!context.captures.empty()) {
                context.captures.erase(context.captures.end() - 1);
            }
        }
    }

}

void onMouseClick(int event, int x, int y, int flags, void *userdata) {

    if (event != cv::MouseEventTypes::EVENT_LBUTTONDOWN) {
        return;
    }

    auto* context = static_cast<Context*>(userdata);

    glm::vec2 pixelPosition {x, y};

    billiard::detection::Ball ball {};
    ball._position = pixelPosition;

    billiard::detection::State pixelState {};
    pixelState._balls.push_back(ball);

    billiard::detection::State state = billiard::detection::pixelToModelCoordinates(*(context->detectionConfig), pixelState);
    glm::vec2 modelPosition = state._balls.at(0)._position;

    Capture capture {};
    capture.frameIndex = context->frameIndex;
    capture.pixelPosition = pixelPosition;
    capture.modelPosition = modelPosition;

    context->captures.push_back(capture);

    std::cout << "cature: pixel=" << pixelPosition << " model=" << modelPosition << " frame=" << capture.frameIndex << std::endl;

    if (context->captures.size() == 2) {
        Capture first = context->captures.at(0);
        Capture second = context->captures.at(1);

        glm::vec2 modelVector = second.modelPosition - first.modelPosition;
        long long frameDuration = second.frameIndex - first.frameIndex;
        float duration = ((float) frameDuration) / 30.0f; // assuming 30 fps

        float s = glm::length(modelVector);

        std::cout << "frames=" << frameDuration
                  << " duration=" << duration
                  << " vector=" << modelVector
                  << " s=" << s
                  << std::endl;
    }
}
