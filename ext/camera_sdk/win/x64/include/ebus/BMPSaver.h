// *****************************************************************************
//
//     Copyright (c) 2013, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#ifndef __BMPSAVER_H__
#define __BMPSAVER_H__

#include "SimpleImagingLib.h"

#include "ImagingBuffer.h"
#include "SimpleImagingLib.h"


namespace SimpleImagingLib 
{


#pragma pack( push, 1 )

	struct BITMAPFILEHEADER
	{
		uint16_t bfType;
		uint32_t bfSize;
		uint16_t bfReserved1;
		uint16_t bfReserved2;
		uint32_t bfOffBits;
	};

	struct BITMAPINFOHEADER
	{
		uint32_t biSize;
		int32_t	 biWidth;
		int32_t	 biHeight;
		uint16_t biPlanes;
		uint16_t biBitCount;
		uint32_t biCompression;
		uint32_t biSizeImage;
		int32_t	 biXPelsPerMeter;
		int32_t	 biYPelsPerMeter;
		uint32_t biColorsUsed;
		uint32_t biClrImportant;
	};

#pragma pack( pop )


class SIMPLEIMAGING_API BMPFile
{
public:

    bool Save( const std::string &aFileName, ImagingBuffer* aImage );
    bool Open( const std::string &aFileName, ImagingBuffer* aImage );

private:

	bool Save( const std::string &aFileName, const void* aData, int32_t aWidth, int32_t aHeight, uint16_t aBitDepth );
	bool ReadHeader( std::string FilePath, BITMAPINFOHEADER* header );
    void WriteToDisk( const std::string &aFileName, BITMAPFILEHEADER aFileHeader, BITMAPINFOHEADER &aInfoHeader, const void* aData );
	void InitializeHeader( BITMAPFILEHEADER &fileHeader, BITMAPINFOHEADER &infoHeader, int32_t aWidth, int32_t aHeight, uint16_t aBitDepth );
	bool ReadData( uint8_t* mBuffer, uint32_t mBufferSize );

};


}

#endif
