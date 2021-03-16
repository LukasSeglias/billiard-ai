// *****************************************************************************
//
//     Copyright (c) 2012, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#pragma once

#include <PvConfigurationWriter.h>
#include <PvConfigurationReader.h>

// Tiling mode supported
enum TilingMode
{
    TILING_MODE_MIN = 0,
    TILING_MODE_RATIO = TILING_MODE_MIN,
    TILING_MODE_CROP,
    TILING_MODE_STRETCH,
    TILING_MODE_MAX
};

class TransmitterConfig
{
public:

    ///
    /// \brief Constructor
    ///
    TransmitterConfig()
    {
        Reset();
    }

    ///
    /// \brief Destructor
    ///
    virtual ~TransmitterConfig() 
    {
    }

    ///
    /// \brief Reset all the field of the object
    ///
    void Reset()
    {
        mWidth = 640;
        mHeight = 480;
        mMode = TILING_MODE_RATIO;
        mFps = 30;
        mPacketSize = 1476;
        mDestinationIPAddress = _T( "239.192.1.1" );
        mDestinationPort = 1042;
        mSourceIPAddress = _T( "" );
    }

    ///
    /// \brief Get the width of the image
    ///
    /// \return Image width
    ///
    inline unsigned int GetWidth()
    {
        return mWidth;
    }
    
    ///
    /// \brief Set the width of the image
    ///
    /// \param aWidth The new width
    ///
    inline void SetWidth( unsigned int aWidth )
    {
        mWidth = aWidth;
    }
    
    ///
    /// \brief Get the height of the image
    ///
    /// \return Image height
    ///
    inline unsigned int GetHeight()
    {
        return mHeight;
    }
    
    ///
    /// \brief Set the height of the image
    ///
    /// \param aHeight The new height
    ///
    inline void SetHeight( unsigned int aHeight )
    {
        mHeight = aHeight;
    }
    
    ///
    /// \brief Get the tiling mode
    ///
    /// \return tiling mode
    ///
    inline TilingMode GetTilingMode()
    {
        return mMode;
    }
    
    ///
    /// \brief Set the tiling mode
    ///
    /// \param aMode The new tiling mode
    ///
    inline void SetTilingMode( TilingMode aMode )
    {
        mMode = aMode;
    }

    ///
    /// \brief Get the frame rate
    ///
    /// \return Frame rate in fps
    ///
    inline unsigned int GetFrameRate()
    {
        return mFps;
    }
    
    ///
    /// \brief Set the frame rate
    ///
    /// \param aFps The new frame rate in fps
    ///
    inline void SetFrameRate( unsigned int aFps )
    {
        mFps = aFps;
    }

    ///
    /// \brief Get the packet size
    ///
    /// \return The packet size
    ///
    inline unsigned int GetPacketSize()
    {
        return mPacketSize;
    }
    
    ///
    /// \brief Set the packet size
    ///
    /// \param aPacketSize The new packet size
    ///
    inline void SetPacketSize( unsigned int aPacketSize )
    {
        mPacketSize = aPacketSize;
    }

    ///
    /// \brief Get the destination address
    ///
    /// \return The destination address
    ///
    inline CString& GetDestinationIPAddress()
    {
        return mDestinationIPAddress;
    }
    
    ///
    /// \brief Set the destination address
    ///
    /// \param aDestinationIPAddress The destination address
    ///
    inline void SetDestinationIPAddress( CString& aDestinationIPAddress )
    {
        mDestinationIPAddress = aDestinationIPAddress;
    }

    ///
    /// \brief Get the UDP port to send to
    ///
    /// \return UDP port to send to
    ///
    inline unsigned short GetDestinationPort()
    {
        return mDestinationPort;
    }
    
    ///
    /// \brief Set the UDP port to send to
    ///
    /// \param aPacketSize The new UDP port to send to
    ///
    inline void SetDestinationPort( unsigned short aDestinationPort )
    {
        mDestinationPort = aDestinationPort;
    }
    
    ///
    /// \brief Get the source address
    ///
    /// \return The source address
    ///
    inline CString& GetSourceIPAddress()
    {
        return mSourceIPAddress;
    }
    
    ///
    /// \brief Set the source address
    ///
    /// \param aSourceIPAddress The source address
    ///
    inline void SetSourceIPAddress( CString& aSourceIPAddress )
    {
        mSourceIPAddress = aSourceIPAddress;
    }

    ///
    /// \brief Store the class into a persistence object
    ///
    /// \param aWriter The persistence object
    ///
    /// \return Error State
    ///
    PvResult Store( PvConfigurationWriter& aWriter );

    ///
    /// \brief Load the class from a persistence object
    ///
    /// \param aReader The persistence object
    ///
    /// \return Error State
    ///
    PvResult Restore( PvConfigurationReader& aReader );
private:
    // Width of the image
    unsigned int mWidth;
    
    // Height of the image
    unsigned int mHeight;
    
    // Tiling mode
    TilingMode mMode;
    
    // Transmitter frame rate
    unsigned int mFps;
    
    // Transmitter packet size
    unsigned int mPacketSize;
    
    // IP address to send to
    CString mDestinationIPAddress;
    
    // UDP port to send to
    unsigned short mDestinationPort;
    
    // Source of the data
    CString mSourceIPAddress;
};