// *****************************************************************************
//
//     Copyright (c) 2016, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#ifndef __PVCAMERABRIDGE_H__
#define __PVCAMERABRIDGE_H__

#include "PvCameraBridgeLib.h"
#include "PvDeviceSerialEnums.h"
#include "PvStringList.h"
#include "PvConfigurationReader.h"
#include "PvConfigurationWriter.h"


class PvDevice;
class PvStream;
class PvGenParameterArray;

namespace PvCameraBridgeLib
{
    class CameraBridge;
}


typedef enum 
{
    PvCameraBridgeUnknown = -1,
    PvCameraBridgePCF = 0,
    PvCameraBridgeCLP,
    PvCameraBridgeGenCP

} PvCameraBridgeType;


class PV_CAMERABRIDGE_API PvCameraBridgeEventSink
{
public:

	PvCameraBridgeEventSink();
    virtual ~PvCameraBridgeEventSink();

    virtual PvResult OnSetParameter( const PvString &aName, const PvString &aValue );
    virtual void OnError( const PvString &aMessage );

};


class PV_CAMERABRIDGE_API PvCameraBridge
{
public:

    PvCameraBridge( PvDevice *aDevice, PvDeviceSerial aPort, PvStream *aStream = NULL );
    virtual ~PvCameraBridge();

    PvGenParameterArray *GetParameters();
    bool IsConnected();
    PvResult Disconnect();
    PvResult Recover();

    PvDeviceSerial GetPort() const;
    PvDevice *GetDevice();
    PvStream *GetStream();

    PvResult StartAcquisition();
    PvResult StopAcquisition();

    bool GetParametersSyncEnabled() const;
    void SetParametersSyncEnabled( bool aEnabled );

    // Source (for parameters sync, acquistion start/stop, etc.)
    PvString GetSource() const;
    void SetSource( const PvString &aSource );

    PvResult RegisterEventSink( PvCameraBridgeEventSink* aEventSink );
    PvResult UnregisterEventSink( PvCameraBridgeEventSink* aEventSink );

    // Pleora Camera File
    PvResult ConnectPleoraCameraFile( const PvString &aPath );
    PvResult GetPleoraCameraFileParserErrors( PvStringList &aErrors );
    PvResult GetPleoraCameraFilePath( PvString &aPath ) const;

    // CLProtocol
    static PvResult GetCLProtocolTemplates( PvStringList &aTemplates );
    PvResult ProbeCLProtocol( const PvString &aTemplate, PvString &aCameraID, PvStringList &aXMLIDs );
    PvResult ConnectCLProtocol( const PvString &aTemplate, const PvString &aXMLID = "" );
    PvResult GetCLProtocolTemplate( PvString &aTemplate ) const;
    PvResult GetCLProtocolCameraID( PvString &aCameraID ) const;
    PvResult GetCLProtocolXMLID( PvString &aXMLID ) const;

    // GenCP
    PvResult GetGenCPXMLIDs( PvStringList &aXMLIDs );
    PvResult ConnectGenCP( const PvString &aXMLID = "" );
    PvResult GetGenCPXMLID( PvString &aXMLID ) const;

    // Persistence
    PvResult Save( const PvString &aPrefix, PvConfigurationWriter &aWriter );
    PvResult Load( const PvString &aPrefix, PvConfigurationReader &aReader, PvStringList &aErrors );

    // Statistics
    uint64_t GetBytesSentToCamera() const;
    uint64_t GetBytesReceivedFromCamera() const;
    void ResetStatistics();

    // Static PoCL helpers
    static bool IsPoCLSupported( PvDevice *aDevice );
    static bool IsPoCLEnabled( PvDevice *aDevice );
    static PvResult SetPoCLEnabled( PvDevice *aDevice, bool aEnabled );

protected:

private:

    PvCameraBridgeLib::CameraBridge *mThis;

	 // Not implemented
	PvCameraBridge( const PvCameraBridge & );
	const PvCameraBridge&operator=( const PvCameraBridge & );

};

#endif
