// *****************************************************************************
//
//     Copyright (c) 2012, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#pragma once

#include <PvBuffer.h>
#include <PvBufferConverter.h>


///
/// \class ImageBuffer
///
/// \brief Enhanced PvBuffer with the functionalities of CImage for image manipulations
///

class ImageBuffer
{
public:
    ///
    /// \brief Constructor
    ///
    ImageBuffer();

    ///
    /// \brief Destructor
    ///
    virtual ~ImageBuffer();

    ///
    /// \brief Allocate: re-allocate the buffer size to be able to received 
    /// images without been resized all the time. The pixel type is fixed
    ///
    /// \param aWidth Width of the image in pixel
    /// \param aHeight Height of the image in pixel
    ///
    /// \return false on error
    ///
    bool Alloc( int aWidth, int aHeight );

    ///
    /// \brief Free any allocated buffer. 
    /// Internally called automatically but can used can for it to 
    /// free some memory on the system
    ///
    void Free();

    /// 
    /// \brief Copy the provided buffer to the current one
    /// 
    /// \param aBuffer buffer source
    ///
    /// \return false on error
    ///
    bool Copy( PvBuffer* aBuffer );

    /// 
    /// \brief Stretch the current buffer to the given buffer respecting
    /// the ROI information provided
    /// 
    /// \param aBuffer Output buffer
    /// \param aX Output buffer ROI offset X in pixel
    /// \param aY Output buffer ROI offset Y in pixel
    /// \param aSizeX Output buffer ROI size x
    /// \param aSizeY Output buffer ROI size x
    ///
    void StretchTo( ImageBuffer* aBuffer, int aX, int aY, int aSizeX, int aSizeY );
    
    /// 
    /// \brief Crop the current buffer to the given buffer respecting
    /// the ROI information provided
    /// 
    /// \param aBuffer Output buffer
    /// \param aX Output buffer ROI offset X in pixel
    /// \param aY Output buffer ROI offset Y in pixel
    /// \param aSizeX Output buffer ROI size x
    /// \param aSizeY Output buffer ROI size x
    ///
    void CropTo( ImageBuffer* aBuffer, int aX, int aY, int aSizeX, int aSizeY );
    
    /// 
    /// \brief Resize with ratio preserved the current buffer to the given buffer respecting
    /// the ROI information provided
    /// 
    /// \param aBuffer Output buffer
    /// \param aX Output buffer ROI offset X in pixel
    /// \param aY Output buffer ROI offset Y in pixel
    /// \param aSizeX Output buffer ROI size x
    /// \param aSizeY Output buffer ROI size x
    ///
    void FitToWithRatio( ImageBuffer* aBuffer, int aX, int aY, int aSizeX, int aSizeY );
    
    ///
    /// \brief Set the zero value to the section of the buffer
    ///
    void SetZero();

    ///
    /// \brief Set the converter to the buffer
    ///
    /// \param aConverter The converter to be set
    ///
    inline void SetConverter( PvBufferConverter* aConverter )
    {
        mConverter = aConverter;
    }

    /// 
    /// \brief Get a PvBuffer for transmission
    /// 
    /// \return The PvBuffer type object pointing to this image
    ///
    inline PvBuffer* GetPvBuffer() 
    { 
        return &mBuffer; 
    }

    ///
    /// From a PvBuffer, retrieve a ImageBuffer
    ///
    /// \param aBuffer PvBuffer we have get with the function GetPvBuffer
    ///
    /// \return Source ImageBuffer
    ///
    inline static ImageBuffer* GetImageBufferFromPvBuffer( PvBuffer* aBuffer )
    {
        return ( ImageBuffer* ) aBuffer->GetID();
    }

    ///
    /// \brief Width of the buffer
    ///
    inline int GetWidth() 
    {
        return mWidth;
    }

    ///
    /// \brief Height of the buffer
    ///
    inline int GetHeight()
    {
        return mHeight;
    }

protected:

    ///
    /// \brief Stretches a source CImage to a destination CImage
    ///
    /// \param aSource Source CImage
    /// \param aSourceRect Source rectangle
    /// \param aDestination Destination CImage
    /// \param aDestinationRect Destination rectangle
    ///
    void Stretch( CImage *aSource, CRect &aSourceRect, CImage *aDestination, CRect &aDestinationRect );

private:

    // Internal buffer
    CImage* mImage;

    // Internal buffer for pixel conversion and eBUS SDK compatibility
    PvBuffer  mBuffer;

    // Converter 
    PvBufferConverter* mConverter;

    // Portion of the image used
    int  mWidth;
    int  mHeight;
};

