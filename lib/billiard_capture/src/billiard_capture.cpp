#include <billiard_capture/billiard_capture.hpp>

#ifdef BILLIARD_CAPTURE_WITH_EBUS_SDK
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
#endif

#include <librealsense2/rs.hpp>
#include <librealsense2/hpp/rs_internal.hpp>
#include <list>
#include "image_stream.hpp"
#include "camera.hpp"

namespace billiard::capture {

// @Source: This code is heavily based on the eBUS SDK Example 'PVStreamSample'
#ifdef BILLIARD_CAPTURE_WITH_EBUS_SDK

    #define BUFFER_COUNT ( 16 )

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
#endif

    const cv::Size colorFrameSize { 1280, 720 };
    const cv::Size depthFrameSize { 1280, 720 };
    // TODO: cleanup
//    const int colorFps = 30;
    const int colorFps = 15;
    const int depthFps = 30;
    const int fourcc = cv::VideoWriter::fourcc('M', 'J', 'P', 'G');
    const std::string fileExtension = "avi";

    bool CameraCapture::open() {
        rs2::context ctx;
        std::cout << "librealsense " << RS2_API_VERSION_STR << std::endl;
        uint32_t numberOfDevices = ctx.query_devices().size();
        std::cout << "RealSense devices connected: " << numberOfDevices << std::endl;

        if (numberOfDevices == 0) {
            return false;
        }

        // TODO: cleanup
        rs2::config cfg;
//        cfg.enable_stream(RS2_STREAM_COLOR, 1920, 1080, RS2_FORMAT_BGR8, 30);
        cfg.enable_stream(RS2_STREAM_COLOR, colorFrameSize.width, colorFrameSize.height, RS2_FORMAT_BGR8, colorFps);
//        cfg.enable_stream(RS2_STREAM_DEPTH, depthFrameSize.width, depthFrameSize.height, RS2_FORMAT_Z16, depthFps);

        camera = new Camera();
        if (camera->pipe.start(cfg)) {
            return true;
        } else {
            return false;
        }
    }

    void CameraCapture::close() {
        recording = false;
        if (worker.joinable()) {
            worker.join();
        }
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

    CameraFrames CameraCapture::readFrames() {

        rs2::frameset data = camera->pipe.wait_for_frames();

        // TODO: cleanup
//        rs2::depth_frame depthFrame = data.get_depth_frame();
//        cv::Mat depth = toMat(depthFrame, CV_16UC1);
        cv::Mat depth;
//        auto depthFrame = data.get_depth_frame();
//        cv::Mat depthInt = toMat(depthFrame, CV_16SC1);
//        depthInt.convertTo(depth, CV_32FC1);
//        depth *= depthFrame.get_units();
//        depth = depthInt; // TODO: remove this

        cv::Mat colorizedDepth;
//        cv::Mat colorizedDepth;
//        if (depthFrame && depthFrame.get_data() != nullptr) {
//            rs2::colorizer color_map;
//            colorizedDepth = toMat(depthFrame.apply_filter(color_map),CV_8UC3);
//        }

        cv::Mat color = toMat(data.get_color_frame(), CV_8UC3);

//        if (capture && imageNumber < 1000) {
//            std::string nr = std::to_string(imageNumber++);
//            std::string colorOutputFileName = "image_" + nr + "_color.png";
//            std::string depthOutputFileName = "image_" + nr + "_depth.png";
//
//            cv::Mat depthScaledToCentimeters = 100.0 * depth;
//            cv::Mat depthScaledThresholded;
//            cv::threshold(depthScaledToCentimeters, depthScaledThresholded, 255, 255, cv::THRESH_BINARY);
//            cv::Mat depthNormalized;
//            cv::normalize(depthScaledThresholded, depthNormalized, 0, 255, cv::NORM_MINMAX, CV_8UC1);
//
//            cv::imshow("depth - original", depth);
//            cv::imshow("depth - scaled", depthScaledToCentimeters);
//            cv::imshow("depth - scaled & thresholded", depthScaledThresholded);
//            cv::imshow("depth - normalized", depthNormalized);
//            cv::waitKey();
//
//            cv::imwrite(colorOutputFileName, color);
//            cv::imwrite(depthOutputFileName, depthNormalized);
//        }

        return CameraFrames(color, depth, colorizedDepth);
    }

    CameraFrames CameraCapture::read() {
        if (recording) {
            std::lock_guard<std::mutex> guard(latestRecordedFramesLock);
            return latestRecordedFrames;
        } else {
            return readFrames();
        }
    }

    void CameraCapture::toggleRecording() {

        if (recording) {

            // Stop recording
            recording = false;
            worker.join();

        } else {

            // Start recording
            recording = true;
            worker = std::thread(CameraCapture::record, this);
        }
    }

    void CameraCapture::record (CameraCapture* capture) {

        std::string videoNr = std::to_string(capture->videoNumber++);
        std::string colorOutputFileName = "video_" + videoNr + "_color." + fileExtension;
        std::string depthOutputFileName = "video_" + videoNr + "_depth." + fileExtension;
        cv::VideoWriter colorVideo { colorOutputFileName, fourcc, (double) colorFps, colorFrameSize, true };
        cv::VideoWriter depthVideo { depthOutputFileName, fourcc, (double) depthFps, depthFrameSize, false };

        if (!colorVideo.isOpened() || !depthVideo.isOpened()) {
            std::cout << "CameraCapture: Unable to open VideoWriter for recording" << std::endl;
            return;
        }

        while (capture->recording) {
            billiard::capture::CameraFrames frames = capture->readFrames();

            capture->latestRecordedFramesLock.lock();
            capture->latestRecordedFrames = frames;
            capture->latestRecordedFramesLock.unlock();

            // TODO: cleanup
            colorVideo.write(frames.color);
//            depthVideo.write(frames.depth);

#ifdef BILLIARD_CAPTURE_DEBUG_VISUAL
            cv::imshow("Color", frames.color);
            cv::imshow("Depth", frames.depth);
#endif
        }
    }

    CameraCapture::~CameraCapture() {
        close();
        delete camera;
    }
}
