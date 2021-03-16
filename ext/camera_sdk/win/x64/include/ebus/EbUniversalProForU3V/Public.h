// *****************************************************************************
//
//     Copyright (c) 2012, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#pragma once

//
// This is the version of the protocol used to communicate between the user mode
// and the kernel mode of the driver. If this version is not the same, the connection of the
// driver should be refused
//
#define U3V_PROTOCOL_DRIVER_NUMBER  ( 4 )

//
// Define an Interface Guid so that app can find the device and talk to it.
// This is used by the SetupAPI library in user mode for driver enumeration
// {BCEEE14F-417C-44C5-B5D0-1E145C90B831}
DEFINE_GUID( GUID_DEVINTERFACE_U3V, 0xbceee14f, 0x417c, 0x44c5, 0xb5, 0xd0, 0x1e, 0x14, 0x5c, 0x90, 0xb8, 0x31);

// This is the compatible ID for the usb class driver
#define U3V_COMPATIBLE_ID      L"USB\\Class_ef&SubClass_05&Prot_00"

// The following value is "arbitrarily" chosen from the space defined by Microsoft
// as being "for non-Microsoft use"
//
#define U3V_FILE_DEVICE_OFFSET ( 45067 )

//
// Device control codes - values between 2048 and 4095 arbitrarily chosen
//
#define IOCTL_U3V_OFFSET           ( 2050 )

#define IOCTL_U3V_OFFSET_GENERAL   (    0 )
#define IOCTL_U3V_OFFSET_DSI       (  100 )
#define IOCTL_U3V_OFFSET_DEI       (  200 )
#define IOCTL_U3V_OFFSET_DCI       (  300 )
#define IOCTL_U3V_OFFSET_DDI       (  400 )

// **************************************************************************
//      Configuration structures 
// **************************************************************************

// DSI Configuration registers
typedef struct 
{
    ULONG Control;
    ULONG RequiredPayloadSize;        // Check for buffer size
    ULONG InfoPayload_SizeAlignment;  // Check for SI xxx Size value alignment
    ULONG MaximumLeaderSize;
    ULONG PayloadTransferSize;
    ULONG PayloadTransferCount;
    ULONG PayloadFinalSize1;
    ULONG PayloadFinalSize2;
    ULONG MaximumTrailerSize;
    ULONG Control_BandwidthControl;
    ULONG BytesPerSecond;             // Only read if SIControlBandwidthControl set
}
U3V_DEVICE_STREAM_INTERFACE_CONFIGURATION;

// DEI Configuration registers
typedef struct 
{
    // here we will use the minimum of the event configuration because all the 
    // event management is handled by the same file object and the SDK will make 
    // the management properly from a single process...
    ULONG MaximumEventTransferLength;
}
U3V_DEVICE_EVENT_INTERFACE_SBRM;

// DCI Configuration registers
typedef struct 
{
    // here we will use the minimum of the available configuration
    ULONG MaximumDeviceResponseTime;        // ms
}
U3V_DEVICE_CONTROL_INTERFACE_ABRM;
typedef struct 
{
    // here we will use the minimum of the available configuration
    ULONG MaximumCommandTransferLength;      
    ULONG MaximumAcknowledgeTransferLength;      
}
U3V_DEVICE_CONTROL_INTERFACE_SBRM;

// Structure to store the statistic on the processing of the Device Control Interface
typedef struct
{
    // Number of request queued
    volatile LONG Queued;
    // Number of request processed
    volatile LONG Processed;
    // Number of request processed with success
    volatile LONG ProcessedOK;
    // Number of pending ACK received
    volatile LONG PendingACKReceived;
    // Number of command cancelled during the transport
    volatile LONG Cancelled;
    // Number of timeout when waiting for the ACK
    volatile LONG Timeout;
    // Number of link error when interecting with the USB transport layer
    volatile LONG LinkError;
    // Number of request cancelled because of formatting request errors
    volatile LONG FormattingRequestError;
    // Incorrect ACK packet
    volatile LONG MalformedACK;
    // Incorrect late ACK packet
    volatile LONG DiscardedLateACK;
}
U3V_DEVICE_CONTROL_INTERFACE_STATISTIC;

// **************************************************************************
//      GENERAL
// **************************************************************************

#define IOCTL_U3V_DEVICE_REBOOT   \
    CTL_CODE( U3V_FILE_DEVICE_OFFSET, IOCTL_U3V_OFFSET + IOCTL_U3V_OFFSET_GENERAL + 0, METHOD_BUFFERED, FILE_ANY_ACCESS )

// **************************************************************************

#define IOCTL_U3V_INFO_READ   \
    CTL_CODE( U3V_FILE_DEVICE_OFFSET, IOCTL_U3V_OFFSET + IOCTL_U3V_OFFSET_GENERAL + 1, METHOD_BUFFERED, FILE_ANY_ACCESS )

typedef struct
{
    // Protocol used to communicate with the user mode
    ULONG DriverProtocolVersion;

    // Version number of the driver
    struct 
    {
        ULONG MajorNumber;
        ULONG MinorNumber;
        ULONG SubMinorNumber;
        ULONG BuildNumber;
    } Version;
} 
U3V_INFO_READ_OUT;

// **************************************************************************

#define IOCTL_U3V_DEVICE_STREAM_INTERFACE_CONFIGURATION_READ   \
    CTL_CODE( U3V_FILE_DEVICE_OFFSET, IOCTL_U3V_OFFSET + IOCTL_U3V_OFFSET_GENERAL + 2, METHOD_BUFFERED, FILE_ANY_ACCESS )

typedef struct
{
    ULONG Index;
    ULONG Timeout;  // in ms
} 
U3V_DEVICE_STREAM_INTERFACE_CONFIGURATION_READ_IN;
typedef struct
{
    U3V_DEVICE_STREAM_INTERFACE_CONFIGURATION Configuration;
} 
U3V_DEVICE_STREAM_INTERFACE_CONFIGURATION_READ_OUT;

// **************************************************************************

#define IOCTL_U3V_DEVICE_CONTROL_INTERFACE_CONFIGURATION_READ   \
    CTL_CODE( U3V_FILE_DEVICE_OFFSET, IOCTL_U3V_OFFSET + IOCTL_U3V_OFFSET_GENERAL + 3, METHOD_BUFFERED, FILE_ANY_ACCESS )

typedef struct
{
    U3V_DEVICE_CONTROL_INTERFACE_ABRM ConfigurationABRM;
    U3V_DEVICE_CONTROL_INTERFACE_SBRM ConfigurationSBRM;
} 
U3V_DEVICE_CONTROL_INTERFACE_CONFIGURATION_READ_OUT;

// **************************************************************************

#define IOCTL_U3V_STATISTIC_READ   \
    CTL_CODE( U3V_FILE_DEVICE_OFFSET, IOCTL_U3V_OFFSET + IOCTL_U3V_OFFSET_GENERAL + 4, METHOD_BUFFERED, FILE_ANY_ACCESS )

typedef struct
{
    union
    {
        U3V_DEVICE_CONTROL_INTERFACE_STATISTIC DeviceControlInterface;
    }
    DeviceInterface;
} 
U3V_DEVICE_STATISTIC_READ_OUT;

// **************************************************************************
//      DEVICE STREAM INTERFACE
// **************************************************************************

// **************************************************************************

#define IOCTL_U3V_DEVICE_STREAM_INTERFACE_SET_MASTER   \
    CTL_CODE( U3V_FILE_DEVICE_OFFSET, IOCTL_U3V_OFFSET + IOCTL_U3V_OFFSET_DSI + 0, METHOD_BUFFERED, FILE_ANY_ACCESS )

typedef struct
{
    ULONG Index;
	ULONG MaximumNumberOfPendingURB;
} 
U3V_DEVICE_STREAM_INTERFACE_SET_MASTER_IN;

#define IOCTL_U3V_DEVICE_STREAM_INTERFACE_BUFFER_QUEUE   \
    CTL_CODE( U3V_FILE_DEVICE_OFFSET, IOCTL_U3V_OFFSET + IOCTL_U3V_OFFSET_DSI + 1, METHOD_BUFFERED, FILE_ANY_ACCESS )

typedef struct
{
    ULONG Index;
    ULONG PayloadSize;
    ULONGLONG Payload;      // This contains a pointer so we can use a 32 bits application with a 64 bits driver
    ULONG Timeout;
} 
U3V_DEVICE_STREAM_INTERFACE_BUFFER_QUEUE_IN;
typedef struct
{
    LONG PayloadSize;
    ULONG PayloadOffset;        
    LONG LeaderSize;
    ULONG LeaderOffset;
    LONG TrailerSize;
    ULONG TrailerOffset;
    ULONGLONG TrailerTimestamp;
    BOOLEAN FirstBlock;
    // Store the leader packet right after, start at offset LeaderOffset 
    //  from lBuffer + sizeof( U3V_DEVICE_STREAM_INTERFACE_BUFFER_QUEUE_OUT )
    // Store the trailer at the offset TrailerOffset after the leader start, 
    //  start at offset TrailerOffset from lBuffer + sizeof( U3V_DEVICE_STREAM_INTERFACE_BUFFER_QUEUE_OUT )
    // Store the payload at the offset PayloadOffset after the trailer start,
    //  start at offset PayloadOffset from lBuffer + sizeof( U3V_DEVICE_STREAM_INTERFACE_BUFFER_QUEUE_OUT )
    //  if the U3V_DEVICE_STREAM_INTERFACE_BUFFER_QUEUE_IN::Payload pointer was NULL
} 
U3V_DEVICE_STREAM_INTERFACE_BUFFER_QUEUE_OUT;

// **************************************************************************
//      DEVICE EVENT INTERFACE
// **************************************************************************

#define IOCTL_U3V_DEVICE_EVENT_INTERFACE_SET_MASTER   \
    CTL_CODE( U3V_FILE_DEVICE_OFFSET, IOCTL_U3V_OFFSET + IOCTL_U3V_OFFSET_DEI + 0, METHOD_BUFFERED, FILE_ANY_ACCESS )

typedef struct
{
	ULONG MaximumNumberOfPendingURB;
    U3V_DEVICE_EVENT_INTERFACE_SBRM SBRM;
} 
U3V_DEVICE_EVENT_INTERFACE_SET_MASTER_IN;

// **************************************************************************

#define IOCTL_U3V_DEVICE_EVENT_INTERFACE_BUFFER_QUEUE   \
    CTL_CODE( U3V_FILE_DEVICE_OFFSET, IOCTL_U3V_OFFSET + IOCTL_U3V_OFFSET_DEI + 1, METHOD_BUFFERED, FILE_ANY_ACCESS )

// The out structure will be used to store the whole event buffer

// **************************************************************************
//      DEVICE CONTROL INTERFACE
// **************************************************************************

#define IOCTL_U3V_DEVICE_CONTROL_INTERFACE_SET_MASTER   \
    CTL_CODE( U3V_FILE_DEVICE_OFFSET, IOCTL_U3V_OFFSET + IOCTL_U3V_OFFSET_DCI + 0, METHOD_BUFFERED, FILE_ANY_ACCESS )

// **************************************************************************

#define IOCTL_U3V_DEVICE_CONTROL_INTERFACE_SEND   \
    CTL_CODE( U3V_FILE_DEVICE_OFFSET, IOCTL_U3V_OFFSET + IOCTL_U3V_OFFSET_DCI + 1, METHOD_BUFFERED, FILE_ANY_ACCESS )

// The In will contains a formatted packet with the command id missing.

// If the ACK is asked, the out will contain it...

// **************************************************************************

#define IOCTL_U3V_DEVICE_CONTROL_INTERFACE_CONFIGURATION_WRITE   \
    CTL_CODE( U3V_FILE_DEVICE_OFFSET, IOCTL_U3V_OFFSET + IOCTL_U3V_OFFSET_DCI + 2, METHOD_BUFFERED, FILE_ANY_ACCESS )

typedef struct
{
    U3V_DEVICE_CONTROL_INTERFACE_ABRM ConfigurationABRM;
    U3V_DEVICE_CONTROL_INTERFACE_SBRM ConfigurationSBRM;
} 
U3V_DEVICE_CONTROL_INTERFACE_CONFIGURATION_WRITE_IN;

// **************************************************************************

#define IOCTL_U3V_DEVICE_STREAM_INTERFACE_CONFIGURATION_WRITE   \
    CTL_CODE( U3V_FILE_DEVICE_OFFSET, IOCTL_U3V_OFFSET + IOCTL_U3V_OFFSET_DCI + 3, METHOD_BUFFERED, FILE_ANY_ACCESS )

#define U3V_DEVICE_STREAM_INTERFACE_CONTROL_ENABLE_BIT  ( 0x01 )

typedef struct
{
    ULONG Index;
    U3V_DEVICE_STREAM_INTERFACE_CONFIGURATION Configuration;
} 
U3V_DEVICE_STREAM_INTERFACE_CONFIGURATION_WRITE_IN;

// **************************************************************************

#define IOCTL_U3V_DEVICE_STREAM_INTERFACE_RESET_ENDPOINT   \
    CTL_CODE( U3V_FILE_DEVICE_OFFSET, IOCTL_U3V_OFFSET + IOCTL_U3V_OFFSET_DCI + 4, METHOD_BUFFERED, FILE_ANY_ACCESS )

typedef struct
{
    ULONG Index;
} 
U3V_DEVICE_STREAM_INTERFACE_RESET_ENDPOINT_IN;

// **************************************************************************
//      DEVICE DEBUG INTERFACE
// **************************************************************************

#define IOCTL_U3V_DEVICE_DEBUG_INTERFACE_BUFFER_QUEUE   \
    CTL_CODE( U3V_FILE_DEVICE_OFFSET, IOCTL_U3V_OFFSET + IOCTL_U3V_OFFSET_DDI + 0, METHOD_BUFFERED, FILE_ANY_ACCESS )

// The out structure will be used to store the whole data

// **************************************************************************

