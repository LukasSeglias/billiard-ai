// *****************************************************************************
//
//     Copyright (c) 2012, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#pragma once

#include "PvDeviceLib.h"
#include "PvDeviceEnums.h"


namespace PvDeviceLib
{
    class ActionCommand;
};



class PV_DEVICE_API PvActionCommand
{
public:    

    PvActionCommand();
    virtual ~PvActionCommand();

    uint32_t GetInterfaceCount() const;
    PvResult GetInterfaceMACAddress( uint32_t aIndex, PvString& aInterfaceMACAddress ) const;
    PvResult GetInterfaceIPAddress( uint32_t aIndex, PvString& aInterfaceIPAddress ) const;
    PvResult GetInterfaceDescription( uint32_t aIndex, PvString& aInterfaceDescription ) const;
    PvResult GetInterfaceEnabled( uint32_t aIndex, bool& aEnabled ) const;
    PvResult SetInterfaceEnabled( uint32_t aIndex, bool aEnabled );

    uint32_t GetDeviceKey() const;
    void SetDeviceKey( uint32_t aDeviceKey );
    uint32_t GetGroupKey() const;
    void SetGroupKey( uint32_t aGroupKey );
    uint32_t GetGroupMask() const;
    void SetGroupMask( uint32_t aGroupMask );
    bool GetScheduledTimeEnable() const;
    void SetScheduledTimeEnable( bool aEnabled );
    uint64_t GetScheduledTime() const;
    void SetScheduledTime( uint64_t aScheduledTime );

    PvResult Send( uint32_t aTimeout, uint32_t aDeviceCount = 0, bool aRequestAcknowledgements = true );
    PvResult Resend( uint32_t aTimeout, uint32_t aDeviceCount = 0, bool aRequestAcknowledgements = true );

    uint32_t GetAcknowledgementCount() const;
    PvResult GetAcknowledgementIPAddress( uint32_t aIndex, PvString& aIPAddress ) const;
    PvResult GetAcknowledgementStatus( uint32_t aIndex, PvActionAckStatusEnum& aStatus ) const;

    uint32_t GetActionAckStatusOKCount() const;
    uint32_t GetActionAckStatusLateCount() const;
    uint32_t GetActionAckStatusOverflowCount() const;
    uint32_t GetActionAckStatusNoRefTimeCount() const;
    void ResetStatistics();

private:

    PvDeviceLib::ActionCommand *mThis;

};

