// *****************************************************************************
//
//     Copyright (c) 2013, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#ifndef __PVSTREAMGEV_H__
#define __PVSTREAMGEV_H__

#include "PvStream.h"



class PV_STREAM_API PvStreamGEV : public PvStream
{
public:
	
	PvStreamGEV();
	virtual ~PvStreamGEV();

    PvResult Open( const PvDeviceInfo *aDeviceInfo );
    PvResult Open( const PvDeviceInfo *aDeviceInfo, uint16_t aLocalPort, uint16_t aChannel = 0, const PvString & aLocalIpAddress = PvString(), uint32_t aBuffersCapacity = 64 );
    PvResult Open( const PvDeviceInfo *aDeviceInfo, const PvString & aMulticastAddr, uint16_t aDataPort, uint16_t aChannel = 0, const PvString & aLocalIPAddress = PvString(), uint32_t aBuffersCapacity = 64 );

    PvResult Open( const PvString &aInfo );
    PvResult Open( const PvString &aInfo, uint16_t aLocalPort, uint16_t aChannel = 0, const PvString & aLocalIpAddress = PvString(), uint32_t aBuffersCapacity = 64 );
    PvResult Open( const PvString &aInfo, const PvString & aMulticastAddr, uint16_t aDataPort, uint16_t aChannel = 0, const PvString & aLocalIPAddress = PvString(), uint32_t aBuffersCapacity = 64 );

    virtual PvStreamType GetType() const;

    PvResult FlushPacketQueue();
    bool GetWaitForFirstPacketOfBlockToStart() const;
    PvResult SetWaitForFirstPacketOfBlockToStart( bool aWaitForFirstPacketOfBlockToStart );

    uint16_t GetLocalPort() const;
    PvString GetLocalIPAddress() const;
    PvString GetMulticastIPAddress() const;

    PvString GetDeviceIPAddress() const;
    uint16_t GetSpecificLocalPort() const;

    uint32_t GetUserModeDataReceiverThreadPriority() const;
    PvResult SetUserModeDataReceiverThreadPriority( uint32_t aPriority );

    static PvResult IsDriverInstalled( PvString &aIPAddress, bool &aInstalled, const PvString & aLocalIPAddress = PvString() );

protected:

private:

	 // Not implemented
	PvStreamGEV( const PvStreamGEV & );
    const PvStreamGEV &operator=( const PvStreamGEV & );
   

};

#endif
