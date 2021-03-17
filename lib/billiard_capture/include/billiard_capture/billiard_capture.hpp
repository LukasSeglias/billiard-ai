#pragma once

#include <opencv2/opencv.hpp>
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

}