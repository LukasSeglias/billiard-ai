// *****************************************************************************
//
//     Copyright (c) 2015, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#ifndef __MP4_WRITER_H__
#define __MP4_WRITER_H__

#include <PvTypes.h>
#include <PvString.h>


class PvImage;


class IMp4Writer
{
public:

    IMp4Writer() {}
    virtual ~IMp4Writer() {}

    virtual bool IsAvailable() const = 0;
    virtual bool Open( const std::string &aFilename, PvImage *aImage ) = 0;
    virtual bool IsOpened() = 0;
    virtual void Close() = 0;

    virtual bool WriteFrame( PvImage *aImage, uint32_t *aFileSizeDelta ) = 0;

    virtual uint32_t GetAvgBitrate() const = 0;
    virtual void SetAvgBitrate( uint32_t aValue ) = 0;

    virtual void GetLastError( PvString &aString ) = 0;
    virtual void ResetLastError() = 0;

};


#endif // __MP4_WRITER_H__

