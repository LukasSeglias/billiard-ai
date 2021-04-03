#pragma once

#include <opencv2/opencv.hpp>
#include <utility>
#include "macro_definition.hpp"

namespace billiard::capture {

    class Device;
    class Stream;
    class ImageStream;

    class EXPORT_BILLIARD_CAPTURE_LIB ImageCapture {
    public:
        ImageCapture() = default;
        ImageCapture(const ImageCapture& other) = delete;
        ImageCapture(ImageCapture&& other) = delete;
        bool open(std::string macAddress);
        void close();
        [[nodiscard]] cv::Mat read() const;
        ~ImageCapture();
    private:
        Device* device;
        Stream* stream;
        ImageStream* imageStream;
    };

    struct EXPORT_BILLIARD_CAPTURE_LIB CameraFrames {
        cv::Mat color;
        cv::Mat depth;
        cv::Mat colorizedDepth;
        CameraFrames(const cv::Mat& color, const cv::Mat& depth, const cv::Mat& colorizedDepth): color(color), depth(depth), colorizedDepth(colorizedDepth) {}
        CameraFrames(const CameraFrames& other) = default;
        CameraFrames(CameraFrames&& other) = default;
        ~CameraFrames() = default;
    };

    class Camera;

    class EXPORT_BILLIARD_CAPTURE_LIB CameraCapture {
    public:
        CameraCapture() = default;
        CameraCapture(const CameraCapture& other) = delete;
        CameraCapture(CameraCapture&& other) = delete;
        bool open();
        void close();
        [[nodiscard]] CameraFrames read() const;
        ~CameraCapture();
    private:
        Camera* camera;
    };

}