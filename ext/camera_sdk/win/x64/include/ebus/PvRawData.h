// *****************************************************************************
//
//     Copyright (c) 2010, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#ifndef __PVRAWDATA_H__
#define __PVRAWDATA_H__

#include "PvBufferLib.h"
#include "PvPixelType.h"


namespace PvBufferLib
{
    class RawData;
    class Buffer;
}


class PV_BUFFER_API PvRawData
{
public:

    uint64_t GetPayloadLength() const;

    PvResult Alloc( uint64_t aPayloadLength, uint32_t aMaximumChunkLength = 0 );
	void Free();

    PvResult Attach( void * aRawBuffer, uint64_t aPayloadLength, uint32_t aMaximumChunkLength = 0 );
	uint8_t *Detach();

protected:

	PvRawData( PvBufferLib::RawData *aRawData );
    virtual ~PvRawData();

private:

	friend class PvBufferLib::Buffer;

	// Not implemented
	PvRawData( const PvRawData & );
	const PvRawData &operator=( const PvRawData & );

    PvBufferLib::RawData *mThis;
};

#endif
