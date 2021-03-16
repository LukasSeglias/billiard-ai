// *****************************************************************************
//
//     Copyright (c) 2015, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#ifndef __PVMP4WRITER_H__
#define __PVMP4WRITER_H__

#include "PvAppUtilsLib.h"
#include "PvBuffer.h"


namespace PvAppUtilsLib
{
    class IMp4Writer;
}


class PV_APPUTILS_API PvMp4Writer
{
public:

    PvMp4Writer();
	~PvMp4Writer();

    bool IsAvailable() const;
    bool IsOpened();

    PvResult Open( const PvString &aFilename, PvImage *aImage );
    PvResult WriteFrame( PvImage *aImage, uint32_t *aFileSizeDelta );
    PvResult Close();

    uint32_t GetAvgBitrate() const;
    PvResult SetAvgBitrate( uint32_t aValue );

    void GetLastError( PvString &aString ) const;
    void ResetLastError();

private:

    PvAppUtilsLib::IMp4Writer *mThis;

	 // Not implemented
	PvMp4Writer( const PvMp4Writer & );
	const PvMp4Writer &operator=( const PvMp4Writer & );

};

#endif
