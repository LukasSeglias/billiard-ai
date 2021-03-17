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

#include <list>

#include "image_stream.hpp"

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

}

billiard::capture::ImageCapture::~ImageCapture() {
    // Delete in reverse order so that everything is shut down nicely
    delete imageStream;
    delete stream;
    delete device;
}

bool billiard::capture::ImageCapture::open(std::string macAddress) {

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

cv::Mat billiard::capture::ImageCapture::read() const {
    if (imageStream != nullptr) {
        cv::Mat image;
        if (imageStream->read(image)) {
            return image;
        }
    }
    return cv::Mat();
}

void billiard::capture::ImageCapture::close() {
    // Delete in reverse order so that everything is shut down nicely
    delete imageStream;
    imageStream = nullptr;
    delete stream;
    stream = nullptr;
    delete device;
    device = nullptr;
}
