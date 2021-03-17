#pragma once

#include <ebus/PvDevice.h>
#include <ebus/PvStream.h>
#include <ebus/PvBuffer.h>
#include <opencv2/opencv.hpp>
#include <list>
#include <iostream>

typedef std::list<PvBuffer*> BufferList;

namespace billiard::capture {

    class Device {
    public:
        PvDevice* device = nullptr;

        explicit Device(PvDevice *device): device(device) {}
        virtual ~Device() {
            device->Disconnect();
            PvDevice::Free(device);
        }

        void enableStream() {
            device->StreamEnable();
        }

        void disableStream() {
            device->StreamDisable();
        }
    };

    class Stream {
    public:
        PvStream* stream = nullptr;
        BufferList buffers;

        explicit Stream(PvStream* stream): stream(stream) {}
        virtual ~Stream() {
            freeStreamBuffers();
            stream->Close();
            PvStream::Free(stream);
        }

        void freeStreamBuffers() {
            for(PvBuffer* buffer : buffers) {
                delete buffer;
            }
            buffers.clear();
        }
        void abortQueuedBuffers() {
            // Abort all buffers from the stream and dequeue
            stream->AbortQueuedBuffers();
            while (stream->GetQueuedBufferCount() > 0) {
                PvBuffer* buffer = nullptr;
                PvResult lOperationResult;
                stream->RetrieveBuffer(&buffer, &lOperationResult);
            }
        }
    };

    class ImageStream {
    public:
        Device& device;
        Stream& stream;
        bool opened = false;

        // Commands
        PvGenCommand* startCommand = nullptr;
        PvGenCommand* stopCommand = nullptr;

        // Stats
        PvGenFloat* frameRate = nullptr;
        PvGenFloat* bandwidth = nullptr;

        ImageStream(Device& device, Stream& stream): device(device), stream(stream) {}

        virtual ~ImageStream() {
            stop();
        }

        bool start();
        bool read(cv::Mat& result);
        void stop();
    };
}
