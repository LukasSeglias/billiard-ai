// *****************************************************************************
//
//     Copyright (c) 2007, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#ifndef __PVBUFFER_H__
#define __PVBUFFER_H__

#include "PvBufferLib.h"
#include "PvPixelType.h"
#include "PvPayloadType.h"
#include "PvImage.h"
#include "PvRawData.h"
#include "PvChunkData.h"


namespace PvBufferLib
{
    class Buffer;
}

namespace PvStreamLib
{
	class Pipeline;
}

class PvPipeline;
class PvStream;
class PvBufferConverter;
class PvBufferConverterRGBFilter;
class PvDeInterlacer;
class PvImage;
class PvTransmitterGEV;


class PV_BUFFER_API PvBuffer
{
public:

    PvBuffer( PvPayloadType aPayloadType = PvPayloadTypeImage );
    virtual ~PvBuffer();

	PvPayloadType GetPayloadType() const;
	
    PvImage *GetImage();
	const PvImage *GetImage() const;
    PvRawData *GetRawData();
	const PvRawData *GetRawData() const;
    PvChunkData *GetChunkData();
	const PvChunkData *GetChunkData() const;

    const uint8_t * GetDataPointer() const;
    uint8_t * GetDataPointer();

    uint64_t GetID() const;
    void SetID( uint64_t aValue );

    bool IsExtendedID() const;
    bool IsAttached() const;
    bool IsAllocated() const;

    uint32_t GetAcquiredSize() const;
    uint32_t GetRequiredSize() const;
	uint32_t GetSize() const;

    PvResult Reset( PvPayloadType aPayloadType = PvPayloadTypeImage );

    PvResult Alloc( uint32_t aSize );
    void Free();

    PvResult Attach( void * aBuffer, uint32_t aSize );
    uint8_t *Detach();

    uint64_t GetBlockID() const;
    PvResult GetOperationResult() const;
    uint64_t GetTimestamp() const;
    uint64_t GetReceptionTime() const;

    PvResult SetTimestamp( uint64_t aTimestamp );
    PvResult SetBlockID( uint64_t aBlockID );
    PvResult SetReceptionTime( uint64_t aReceptionTime );

    uint32_t GetPacketsRecoveredCount() const;
    uint32_t GetPacketsRecoveredSingleResendCount() const;
    uint32_t GetResendGroupRequestedCount() const;
    uint32_t GetResendPacketRequestedCount() const;
    uint32_t GetLostPacketCount() const;
    uint32_t GetIgnoredPacketCount() const;
    uint32_t GetRedundantPacketCount() const;
    uint32_t GetPacketOutOfOrderCount() const;

    PvResult GetMissingPacketIdsCount( uint32_t& aCount );
    PvResult GetMissingPacketIds( uint32_t aIndex, uint32_t& aPacketIdLow, uint32_t& aPacketIdHigh );

    void SetChunkLayoutID( uint32_t aChunkLayoutID );
    uint32_t GetChunkLayoutID();
    PvResult AddChunk( uint32_t aID, const uint8_t *aData, uint32_t aLength );

    bool HasChunks() const;
    uint32_t GetChunkCount();
    PvResult GetChunkIDByIndex( uint32_t aIndex, uint32_t &aID );
    uint32_t GetChunkSizeByIndex( uint32_t aIndex );
    uint32_t GetChunkSizeByID( uint32_t aID );
    const uint8_t *GetChunkRawDataByIndex( uint32_t aIndex );
    const uint8_t *GetChunkRawDataByID( uint32_t aID );
    uint32_t GetPayloadSize() const;

    bool IsHeaderValid() const;
    bool IsTrailerValid() const;

private:

    // Not implemented
	PvBuffer( const PvBuffer & );
	const PvBuffer &operator=( const PvBuffer & );

	friend class PvStreamLib::Pipeline;
    friend class PvPipeline;
    friend class PvStream;
    friend class PvBufferConverter;
    friend class PvBufferConverterRGBFilter;
    friend class PvDeInterlacer;
    friend class PvTransmitterGEV;

    PvBufferLib::Buffer * mThis;
};


#endif
