// *****************************************************************************
//
//     Copyright (c) 2012, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#ifndef __PVTRANSMITTERGEV_H__
#define __PVTRANSMITTERGEV_H__

#include "PvTransmitterLib.h"
#include "PvBuffer.h"
#include "IPvTransmitterGEV.h"


namespace PvTransmitterLib
{
    class TransmitterGEV;
};


class PV_TRANSMITTER_API PvTransmitterGEV 
    : public IPvTransmitterGEV
{
public:

    PvTransmitterGEV();
    virtual ~PvTransmitterGEV();

    PvResult Open( PvString aDestinationIPAddress, uint16_t aDestinationPort, 
        PvString aSourceIPAddress = "", uint16_t aSourcePort = 0, bool aDontFrag = true,
        bool aExtendedIDs = false, uint32_t aBuffersCapacity = 64, bool aTimestampWhenSending = false, 
        uint32_t aNumberOfPendingResendRequests = 32, uint32_t aNumberOfBuffersHoldForResends = 0 );
    PvResult Close();
    PvResult LoadBufferPool( PvBuffer** aBuffers, uint32_t aBufferCount );
    PvResult QueueBuffer( PvBuffer* aBuffer );
    PvResult RetrieveFreeBuffer( PvBuffer ** aBuffer, uint32_t aTimeout = 0xFFFFFFFF );
    PvResult AbortQueuedBuffers( uint32_t aTimeout = 0xFFFFFFFF, bool* aPartialTransmission = NULL );

    uint32_t GetQueuedBufferCount();
    uint32_t GetPacketSize();
    PvResult SetPacketSize( uint32_t aPacketSize );

    float GetMaxPayloadThroughput();
    PvResult SetMaxPayloadThroughput( float aMaxPayloadThroughput );
    uint16_t GetSourcePort();
    uint16_t GetDestinationPort();

    PvString GetDestinationIPAddress();
    PvString GetSourceIPAddress();
    
    void ResetStats();
    uint64_t GetBlocksTransmitted() const;
    uint64_t GetSamplingTime() const;
    uint64_t GetPayloadBytesTransmitted() const;
    float GetInstantaneousPayloadThroughput() const;
    float GetAveragePayloadThroughput() const;
    float GetInstantaneousTransmissionRate() const;
    float GetAverageTransmissionRate() const;
    uint64_t GetNumberOfResendRequests() const;

    PvResult SetSentBuffersTimeout( uint32_t aTimeout );
    uint32_t GetSentBuffersTimeout() const;

    virtual bool IsOpen() const;
    virtual bool IsTransmitting() const;
    virtual bool GetExtendedIDs() const;	
    virtual PvResult SetExtendedIDs( bool aExtendedID );
    virtual void QueuePacketResend( uint64_t aBlockID, uint32_t aFirstPacketID, uint32_t aLastPacketID );

    uint32_t GetUserModeTransmitterThreadPriority() const;
    PvResult SetUserModeTransmitterThreadPriority( uint32_t aPriority );
    uint32_t GetBufferPoolThreadPriority() const;
    PvResult SetBufferPoolThreadPriority( uint32_t aPriority );

private:

    PvTransmitterLib::TransmitterGEV *mThis;

};

#endif
