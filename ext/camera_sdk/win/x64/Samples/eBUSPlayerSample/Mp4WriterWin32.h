// *****************************************************************************
//
//     Copyright (c) 2015, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#ifndef __MP4_WRITER_WIN32_H__
#define __MP4_WRITER_WIN32_H__

#include <IMp4Writer.h>
#include <PvMp4Writer.h>


class Mp4WriterWin32 : public IMp4Writer
{
public:

    Mp4WriterWin32() {}
    virtual ~Mp4WriterWin32() {}

    bool IsAvailable() const { return mThis.IsAvailable(); }
    bool Open( const std::string &aFilename, PvImage *aImage ) { return mThis.Open( aFilename.c_str(), aImage ).IsOK(); }
    bool IsOpened() { return mThis.IsOpened(); }
    void Close() { mThis.Close(); }

    bool WriteFrame( PvImage *aImage, uint32_t *aFileSizeDelta = NULL ) { return mThis.WriteFrame( aImage, aFileSizeDelta ).IsOK(); }

    uint32_t GetAvgBitrate() const { return mThis.GetAvgBitrate(); }
    void SetAvgBitrate( uint32_t aValue ) { mThis.SetAvgBitrate( aValue ); }

    void GetLastError( PvString &aError ) { mThis.GetLastError( aError ); }
    void ResetLastError() { mThis.ResetLastError(); }

private:

    PvMp4Writer mThis;

};


#endif // __MP4_WRITER_WIN32_H__

