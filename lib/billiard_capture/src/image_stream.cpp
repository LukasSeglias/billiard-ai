#include "image_stream.hpp"
#include <iostream>
#include <iomanip>

void configureStream(PvGenParameterArray* deviceParams) {
    // TODO: ist das notwendig?
    PvGenBoolean* doubleRateEnable = dynamic_cast<PvGenBoolean*>(deviceParams->Get("DoubleRate_Enable"));
    doubleRateEnable->SetValue(false);

    // TODO: hier setzen oder config auf Gerät ändern?
    PvGenInteger* width = dynamic_cast<PvGenInteger*>(deviceParams->Get("Width"));
    width->SetValue(2048);

    PvGenFloat* exposureTime = dynamic_cast<PvGenFloat*>(deviceParams->Get("ExposureTime"));
    exposureTime->SetValue(10000.0);
}

bool billiard::capture::ImageStream::start() {
    if (!opened) {
        PvGenParameterArray* deviceParams = device.device->GetParameters();

        startCommand = dynamic_cast<PvGenCommand*>(deviceParams->Get("AcquisitionStart"));
        stopCommand = dynamic_cast<PvGenCommand*>(deviceParams->Get("AcquisitionStop"));

        configureStream(deviceParams);

        // Get stream parameters
        PvGenParameterArray* streamParams = stream.stream->GetParameters();

        // Map a few GenICam stream stats counters
        frameRate = dynamic_cast<PvGenFloat*>(streamParams->Get("AcquisitionRate"));
        bandwidth = dynamic_cast<PvGenFloat*>(streamParams->Get("Bandwidth"));

        std::cout << "Enable streaming on the controller." << std::endl;
        device.enableStream();

        std::cout << "Sending AcquisitionStart command to the device" << std::endl;
        startCommand->Execute();

        opened = true;
        // TODO: should we handle errors in the calls above?
        return true;
    }
    return true;
}

void pvImageToMat(PvImage* pvImage, cv::Mat& image) {
    uint32_t width = pvImage->GetWidth();
    uint32_t height = pvImage->GetHeight();

    // @Source: Conversion from PVImage to Mat based on https://stackoverflow.com/questions/34218482/pvbuffer-pleora-sdk-to-opencv-buffer
    unsigned char* data = pvImage->GetDataPointer();
    cv::Mat frame(height, width, CV_8UC1, data, cv::Mat::AUTO_STEP);
    cv::cvtColor(frame, image, cv::COLOR_BayerGB2RGB);
}

bool billiard::capture::ImageStream::read(cv::Mat& image) {

    if (!opened) {
        // TODO: should we implicitly start instead?
        return false;
    }

    double frameRateVal = 0.0;
    double bandwidthVal = 0.0;

    PvBuffer* buffer = nullptr;
    PvResult operationResult;

    bool imageFound = false;

    while (true) {
        PvResult result = stream.stream->RetrieveBuffer(&buffer, &operationResult, 1000);
        if (result.IsOK()) {
            if (operationResult.IsOK()) {

                frameRate->GetValue(frameRateVal);
                bandwidth->GetValue(bandwidthVal);

                std::cout << std::fixed << std::setprecision(1);
                std::cout << " BlockID: " << std::uppercase << std::hex << std::setfill('0') << std::setw(16)
                          << buffer->GetBlockID();

                PvPayloadType type = buffer->GetPayloadType();
                if (type == PvPayloadTypeImage) {
                    PvImage* pvImage = buffer->GetImage();
                    uint32_t width = pvImage->GetWidth();
                    uint32_t height = pvImage->GetHeight();

                    pvImageToMat(pvImage, image);

                    imageFound = true;
                    std::cout << "  W: " << std::dec << width << " H: " << height;
                } else {
                    std::cout << " (buffer does not contain image)";
                }

                std::cout << "  " << frameRateVal << " FPS  " << (bandwidthVal / 1000000.0) << " Mb/s   \r";
            } else {
                // Non OK operational result
                std::cout << "ERROR: " << operationResult.GetCodeString().GetAscii() << "\r";
            }

            // Re-queue the buffer in the stream object
            stream.stream->QueueBuffer(buffer);

            if (imageFound) {
                return true;
            }
        } else {
            // Retrieve buffer failure
            std::cerr << "ERROR: " << result.GetCodeString().GetAscii() << "\r";
            return false;
        }
    }
}

void billiard::capture::ImageStream::stop() {
    if (opened) {
        std::cout << "Sending AcquisitionStop command to the device" << std::endl;
        stopCommand->Execute();

        std::cout << "Disable streaming on the controller." << std::endl;
        device.disableStream();

        std::cout << "Aborting buffers still in stream" << std::endl;
        stream.abortQueuedBuffers();

        opened = false;
    }
}
