// *****************************************************************************
//
//     Copyright (c) 2010, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#ifndef __PVIMAGE_H__
#define __PVIMAGE_H__

#include "PvBufferLib.h"
#include "PvPixelType.h"


class PvBuffer;
class PvTruesenseConverter;

namespace PvBufferLib
{
    class Image;
    class Buffer;
}


class PV_BUFFER_API PvImage
{
public:

    const uint8_t *GetDataPointer() const;
    uint8_t *GetDataPointer();

    uint32_t GetWidth() const;
    uint32_t GetHeight() const;
    PvPixelType GetPixelType() const;
    uint32_t GetBitsPerPixel() const;

    static uint32_t GetPixelSize( PvPixelType aPixelType );
    static bool IsPixelColor( PvPixelType aPixelType );
    static bool IsPixelHighRes( PvPixelType aPixelType );
    static uint32_t GetBitsPerComponent( PvPixelType aPixelType );
    static PvString PixelTypeToString( PvPixelType aPixelType );

    uint32_t GetRequiredSize() const;
    uint32_t GetImageSize() const;
    uint32_t GetEffectiveImageSize() const;

    uint32_t GetOffsetX() const;
    uint32_t GetOffsetY() const;
    uint16_t GetPaddingX() const;
    uint16_t GetPaddingY() const;

    void SetOffsetX( uint32_t aValue );
    void SetOffsetY( uint32_t aValue );

    PvResult Alloc( uint32_t aSizeX, uint32_t aSizeY, PvPixelType aPixelType, uint16_t aPaddingX = 0, uint16_t aPaddingY = 0, 
        uint32_t aMaximumChunkLength = 0 );
	void Free();

    PvResult Attach( void * aRawBuffer, uint32_t aSizeX, uint32_t aSizeY, PvPixelType aPixelType, uint16_t aPaddingX = 0, 
        uint16_t aPaddingY = 0, uint32_t aMaximumChunkLength = 0 );
	uint8_t *Detach();

    bool IsPartialLineMissing() const;
    bool IsFullLineMissing() const;
    void SetEOFByLineCount( bool aValue = true ); 
    bool IsEOFByLineCount() const;
    bool IsInterlacedEven() const;
    bool IsInterlacedOdd() const;
    bool IsImageDropped() const;
    bool IsDataOverrun() const;

	PvBuffer *GetBuffer();

protected:

    PvImage( PvBufferLib::Image *aImage );
    virtual ~PvImage();

private:

	friend class PvBufferLib::Buffer;
	friend class PvTruesenseConverter;

    PvBufferLib::Image * mThis;

	// Not implemented
	PvImage( const PvImage & );
	const PvImage &operator=( const PvImage & );
};

#endif
