// *****************************************************************************
//
//     Copyright (c) 2012, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#pragma once

#include "Thread.h"
#include "Constants.h"
#include "CurrentBuffersTable.h"
#include "ImageBuffer.h"
#include "TransmitterPeriodStabilizer.h"
#include "TransmitterConfig.h"
#include "Queue.h"

#include <PvBufferConverter.h>
#include <PvTransmitterGEV.h>
#include <PvDisplayWnd.h>
#include <PvFPSStabilizer.h>

// Stats information
typedef struct
{
    unsigned int BlocksTransmitted;
    float InstantaneousTransmissionRate;
    float InstantaneousPayloadThroughput;
    unsigned int DisplayFrameRate;
} TransmitterStats;

///
/// \class TransmitterThread
///
/// \brief Thread managing the transmission
///
class TransmitterThread 
    : public Thread
{
public:
    ///
    /// \brief Constructor
    ///
    TransmitterThread();

    /// 
    /// \brief Destructor
    ///
    virtual ~TransmitterThread();

    ///
    /// \brief Initialize the transmitter memory class.
    /// Must be called once at the beginning
    ///
    /// \param aMainWnd Parent GUI window
    /// \param aDisplay Display windows 
    /// \param aCurrentBuffersTable Current Buffers Table used as data source
    ///
    /// \return false on error
    ///
    bool Initialize( CWnd* aMainWnd, PvDisplayWnd* aDisplay, CurrentBuffersTable* aCurrentBuffersTable );

    ///
    /// \brief Allocate / Re-allocate the memory and configure the thread
    ///
    /// \param aMaxInputWidth Maximun source image width. Used to initialize internal memory.
    /// \param aMaxInputHeight Maximun source image height. Used to initialize internal memory. 
    /// \param aConfig The transmitter config
    ///
    /// \return false on error
    ///
    bool Configure( unsigned int aMaxInputWidth, unsigned int aMaxInputHeight, 
        TransmitterConfig& aConfig );
    
    ///
    /// \brief Start the transmitter thread
    ///
    /// \return false on error
    ///
    virtual bool Start();
    
    ///
    /// \brief Stop the transmitter thread
    ///
    /// \return false on error
    ///
    virtual bool Stop();

    ///
    /// \brief Retrieve the statistic 
    ///
    void GetStats( TransmitterStats& aStats );

protected:

    ///
    /// \brief Main thread function processing
    /// Will get the images from the current buffers table, format them into tiles and
    /// transmit the image
    ///
    /// \return Always 0
    ///
    virtual DWORD Function();

private:

    // Object that keep track of the last buffer for each stream
    CurrentBuffersTable* mCurrentBuffersTable;

    // Buffer used to display and transmit the reformatted images
    ImageBuffer mBuffers[ TX_POOL_SIZE ];

    // Buffer for the conversion in RGB32
    ImageBuffer mBufferConvertedImage;

    // Internal storage for buffer
    Queue<ImageBuffer> mBufferFree;

    // Current configurated mode.
    TilingMode mMode;

    // Transmitter engine
    PvTransmitterGEV mTransmitter;

    // Used to keep track of the display frame rate
    PvFPSStabilizer mDisplayRate;

    // Manage the output frequency
    TransmitterPeriodStabilizer mThrottleOutput;
    
    // Reference to the display to output the PvBuffer to the user
    PvDisplayWnd* mDisplay;

    // Parent window
    CWnd* mMainWnd;

    // Converter for the buffer ( shared in all of them to minimize the resource usage )
    PvBufferConverter mConverter;

private:
    ///
    /// \brief Convert the snapshot images into a tiled image
    ///
    /// \param aBuffer Output destination buffer for the formatted image
    ///
    void Convert( ImageBuffer* aBuffer );
};

