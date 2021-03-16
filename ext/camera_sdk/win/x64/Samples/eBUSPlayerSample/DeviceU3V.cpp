// *****************************************************************************
//
//     Copyright (c) 2013, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#include "eBUSPlayerShared.h"
#include "DeviceU3V.h"

#include <PvDeviceU3V.h>
#include <PvDeviceInfoU3V.h>


#ifdef _AFXDLL
    IMPLEMENT_DYNAMIC( DeviceU3V, Device )
#endif // _AFXDLL


///
/// \brief Constructor
///

DeviceU3V::DeviceU3V( IMessageSink *aSink, LogBuffer *aLogBuffer )
    : Device( aSink, aLogBuffer )
{
    SetDevice( new PvDeviceU3V );
}


///
/// \brief Destructor
///

DeviceU3V::~DeviceU3V()
{

}


///
/// \brief Connects a USB3 Vision device
///

PvResult DeviceU3V::Connect( IProgress *aProgress, Setup *aSetup, const PvDeviceInfo *aDeviceInfo, uint16_t &aChannel, const PvString &aCommunicationParameters ) 
{ 
    PV_LOGINFO( mLogger, "Connecting USB3 Vision device: " << aDeviceInfo->GetConnectionID().GetAscii() );

    // Make sure we have the right type of device info
    const PvDeviceInfoU3V *lDeviceInfo = dynamic_cast<const PvDeviceInfoU3V *>( aDeviceInfo );
    if ( lDeviceInfo == NULL )
    {
        return PvResult::Code::NOT_SUPPORTED;
    }

    LoadCommunicationParameters( aCommunicationParameters );

    // Connect device
    PvResult lResult = GetDevice()->Connect( aDeviceInfo );

    return lResult; 
}


///
/// \brief Returns the U3V device GUID
///

std::string DeviceU3V::GetGUID()
{
    if ( !IsConnected() )
    {
        return "";
    }

    PvString lGUID;
    GetDevice()->GetCommunicationParameters()->GetStringValue( "DeviceGUID", lGUID );

    return lGUID.GetAscii();
}


///
/// \brief Sets the streaming destination
///

PvResult DeviceU3V::SetStreamDestination( Setup *aSetup, Stream *aStream, uint16_t aChannel ) 
{ 
    return PvResult::Code::OK; 
}

