// *****************************************************************************
//
//     Copyright (c) 2008, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#ifndef __PVBUFFERWRITTER_H__
#define __PVBUFFERWRITTER_H__

#include "PvBuffer.h"
#include "PvBufferConverter.h"
#include "PvBufferFormatType.h"


namespace PvBufferLib
{
    class BufferWriter;
}


class PV_BUFFER_API PvBufferWriter
{

public:

    PvBufferWriter();
    virtual ~PvBufferWriter();

    PvResult Store( PvBuffer* aBuffer, const PvString& aFilename, PvBufferFormatType aType = PvBufferFormatBMP, uint32_t *aBytesWritten = NULL );

    PvBufferConverter &GetConverter();

protected:

private:

    // Not implemented
	PvBufferWriter( const PvBufferWriter & );
	const PvBufferWriter &operator=( const PvBufferWriter & );

    PvBufferLib::BufferWriter *mThis;
};

#endif
