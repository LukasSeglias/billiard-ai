// *****************************************************************************
//
//     Copyright (c) 2017, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#ifndef __PVCHUNKDATA_H__
#define __PVCHUNKDATA_H__

#include "PvBufferLib.h"
#include "PvPixelType.h"


namespace PvBufferLib
{
    class ChunkData;
    class Buffer;
}


class PV_BUFFER_API PvChunkData
{
public:

    uint64_t GetChunkDataPayloadLength() const;

    PvResult Alloc( uint32_t aMaximumChunkLength );
	void Free();

    PvResult Attach( void * aRawBuffer, uint32_t aMaximumChunkLength );
	uint8_t *Detach();

    // Writing chunk data
    PvResult AddChunk( uint32_t aID, const uint8_t *aData, uint32_t aLength );

    // Reading chunk data
    uint32_t GetChunkCount();
    PvResult GetChunkIDByIndex( uint32_t aIndex, uint32_t &aID );
    uint32_t GetChunkSizeByIndex( uint32_t aIndex );
    uint32_t GetChunkSizeByID( uint32_t aID );
    const uint8_t *GetChunkRawDataByIndex( uint32_t aIndex );
    const uint8_t *GetChunkRawDataByID( uint32_t aID );

public:

	PvChunkData( PvBufferLib::ChunkData *aChunkData );
    virtual ~PvChunkData();

private:

	friend class PvBufferLib::Buffer;

	// Not implemented
	PvChunkData( const PvChunkData & );
	const PvChunkData &operator=( const PvChunkData & );

    PvBufferLib::ChunkData *mThis;

};

#endif
