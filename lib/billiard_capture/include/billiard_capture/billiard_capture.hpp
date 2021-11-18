#pragma once

#include <opencv2/opencv.hpp>
#include <utility>
#include "macro_definition.hpp"
#include <atomic>
#include <thread>

namespace billiard::capture {

#ifdef BILLIARD_CAPTURE_WITH_EBUS_SDK
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
        [[nodiscard]] virtual cv::Mat read() const;
        ~ImageCapture();
    private:
        Device* device;
        Stream* stream;
        ImageStream* imageStream;
    };
#endif

    struct EXPORT_BILLIARD_CAPTURE_LIB CameraFrames {
        cv::Mat color;
        cv::Mat depth;
        cv::Mat colorizedDepth;
        CameraFrames(const cv::Mat& color, const cv::Mat& depth, const cv::Mat& colorizedDepth): color(color), depth(depth), colorizedDepth(colorizedDepth) {}
        CameraFrames() = default;
        CameraFrames(const CameraFrames& other) = default;
        CameraFrames(CameraFrames&& other) = default;
        ~CameraFrames() = default;
        CameraFrames& operator=(const CameraFrames& other) = default;
    };

    class Camera;

    class EXPORT_BILLIARD_CAPTURE_LIB CameraCapture {
    public:
        CameraCapture() = default;
        CameraCapture(const CameraCapture& other) = delete;
        CameraCapture(CameraCapture&& other) = delete;
        bool open();
        void close();
        void toggleRecording();
        CameraFrames read();
        ~CameraCapture();
    private:
        Camera* camera;
        int videoNumber = 1;
        std::atomic_bool recording = false;
        std::thread worker;
        billiard::capture::CameraFrames latestRecordedFrames;
        std::mutex latestRecordedFramesLock;

        static void record(CameraCapture* capture);
        CameraFrames readFrames();
    };

}