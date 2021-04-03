#include <billiard_capture/billiard_capture.hpp>

#include <ebus/PvBufferConverter.h>

#include <ebus/PvSystem.h>
#include <ebus/PvDevice.h>
#include <ebus/PvDeviceGEV.h>
#include <ebus/PvDeviceU3V.h>
#include <ebus/PvDeviceInfoGEV.h>
#include <ebus/PvStream.h>
#include <ebus/PvStreamGEV.h>
#include <ebus/PvStreamU3V.h>
#include <ebus/PvBuffer.h>

#include <librealsense2/rs.hpp>
#include <librealsense2/hpp/rs_internal.hpp>

#include <list>

#include "image_stream.hpp"
#include "camera.hpp"

// @Source: This code is heavily based on the eBUS SDK Example 'PVStreamSample'

#define BUFFER_COUNT ( 16 )

namespace billiard::capture {

    // @Source: Heavily based on function PvSelectDevice from PVSampleUtils.h
    bool findGigEDeviceWithMAC(std::string& macAddress, PvString* aConnectionID) {

        PvSystem system;
        system.Find();

        for (uint32_t i = 0; i < system.GetInterfaceCount(); i++) {
            auto* iface = dynamic_cast<const PvInterface*>(system.GetInterface(i));

            if (iface != nullptr && iface->GetType() == PvInterfaceTypeNetworkAdapter) {

                for (uint32_t deviceIndex = 0; deviceIndex < iface->GetDeviceCount(); deviceIndex++) {

                    const PvDeviceInfo* deviceInfo = iface->GetDeviceInfo(deviceIndex);
                    if (deviceInfo->GetType() == PvDeviceInfoTypeGEV) {

                        auto* deviceInfoGEV = dynamic_cast<const PvDeviceInfoGEV*>(deviceInfo);
                        if (deviceInfoGEV != nullptr) {

                            if (macAddress == deviceInfoGEV->GetMACAddress().GetAscii()) {

                                if (!deviceInfo->IsConfigurationValid()) {
                                    std::cerr << "Device with MAC " << macAddress << " has an invalid configuration"
                                              << std::endl;
                                    return false;
                                }

                                *aConnectionID = deviceInfoGEV->GetConnectionID();
                                return true;
                            }
                        }
                    }
                }
            }
        }

        std::cerr << "Device with MAC " << macAddress << " not found" << std::endl;
        return false;
    }

    Device* ConnectToDevice(const PvString& connectionId) {

        // Connect to the GigE Vision or USB3 Vision device
        std::cout << "Connecting to device." << std::endl;
        PvResult result;
        PvDevice* device = PvDevice::CreateAndConnect(connectionId, &result);
        if (device == nullptr) {
            std::cerr << "Unable to connect to device." << std::endl;
            return nullptr;
        }
        return new Device(device);
    }

    Stream* OpenStream(const PvString& connectionId) {

        // Open stream to the GigE Vision or USB3 Vision device
        std::cout << "Opening stream from device." << std::endl;
        PvResult result;
        PvStream* stream = PvStream::CreateAndOpen(connectionId, &result);
        if (stream == nullptr) {
            std::cerr << "Unable to stream from device." << std::endl;
            return nullptr;
        }
        return new Stream(stream);
    }

    void ConfigureStream(PvDevice* device, PvStream* stream) {
        // If this is a GigE Vision device, configure GigE Vision specific streaming parameters
        auto* deviceGEV = dynamic_cast<PvDeviceGEV*>(device);
        if (deviceGEV != nullptr) {
            auto* streamGEV = static_cast<PvStreamGEV*>(stream);

            deviceGEV->NegotiatePacketSize();
            deviceGEV->SetStreamDestination(streamGEV->GetLocalIPAddress(), streamGEV->GetLocalPort());
        }
    }

    void CreateStreamBuffers(PvDevice* device, PvStream* stream, BufferList& buffers) {

        uint32_t size = device->GetPayloadSize();

        uint32_t bufferCount = (stream->GetQueuedBufferMaximum() < BUFFER_COUNT) ?
                               stream->GetQueuedBufferMaximum() :
                               BUFFER_COUNT;

        for (uint32_t i = 0; i < bufferCount; i++) {

            auto* buffer = new PvBuffer;
            buffer->Alloc(static_cast<uint32_t>(size));

            buffers.push_back(buffer);
        }

        for (PvBuffer* buffer : buffers) {
            stream->QueueBuffer(buffer);
        }
    }

    ImageCapture::~ImageCapture() {
        // Delete in reverse order so that everything is shut down nicely
        delete imageStream;
        delete stream;
        delete device;
    }

    bool ImageCapture::open(std::string macAddress) {

        PvString connectionId;
        if (findGigEDeviceWithMAC(macAddress, &connectionId)) {

            device = ConnectToDevice(connectionId);
            if (device != nullptr) {

                stream = OpenStream(connectionId);
                if (stream != nullptr) {

                    ConfigureStream(device->device, stream->stream);
                    CreateStreamBuffers(device->device, stream->stream, stream->buffers);

                    imageStream = new ImageStream(*device, *stream);
                    imageStream->start();

                    std::cout << "open succeeded" << std::endl;

                    return true;
                }
            }
        }

        return false;
    }

    cv::Mat ImageCapture::read() const {
        if (imageStream != nullptr) {
            cv::Mat image;
            if (imageStream->read(image)) {
                return image;
            }
        }
        return cv::Mat();
    }

    void ImageCapture::close() {
        // Delete in reverse order so that everything is shut down nicely
        delete imageStream;
        imageStream = nullptr;
        delete stream;
        stream = nullptr;
        delete device;
        device = nullptr;
    }

    bool CameraCapture::open() {
        rs2::context ctx;
        std::cout << "librealsense " << RS2_API_VERSION_STR << std::endl;
        std::cout << "RealSense devices connected: " << ctx.query_devices().size() << std::endl;

        rs2::config cfg;
        cfg.enable_stream(RS2_STREAM_COLOR, 1920, 1080, RS2_FORMAT_BGR8, 30);
        cfg.enable_stream(RS2_STREAM_DEPTH, 848, 480, RS2_FORMAT_Z16, 30);

        camera = new Camera();
        camera->pipe.start(cfg);

        return true;
    }

    void CameraCapture::close() {
        if (camera != nullptr) {
            camera->pipe.stop();
        }
    }

    cv::Mat toMat(const rs2::frame& frame, int type) {
        if (!frame || frame.get_data() == nullptr) {
            return cv::Mat {};
        }
        const int width = frame.as<rs2::video_frame>().get_width();
        const int height = frame.as<rs2::video_frame>().get_height();
        cv::Mat image(cv::Size(width, height), type, (void*)frame.get_data(), cv::Mat::AUTO_STEP);
        // Clone in order to copy image content so that original frame can be freed
        return image.clone();
    }

    CameraFrames CameraCapture::read() const {

        rs2::frameset data = camera->pipe.wait_for_frames();

        rs2::depth_frame depthFrame = data.get_depth_frame();
        cv::Mat depth = toMat(depthFrame, CV_8UC1);

        cv::Mat colorizedDepth;
        if (depthFrame && depthFrame.get_data() != nullptr) {
            rs2::colorizer color_map;
            colorizedDepth = toMat(depthFrame.apply_filter(color_map),CV_8UC3);
        }

        cv::Mat color = toMat(data.get_color_frame(), CV_8UC3);

        return CameraFrames(color, depth, colorizedDepth);
    }

    CameraCapture::~CameraCapture() {
        delete camera;
    }
}
