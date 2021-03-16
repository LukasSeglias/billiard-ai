// *****************************************************************************
//
//     Copyright (c) 2013, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#ifndef __PVTFTPCLIENT_H__
#define __PVTFTPCLIENT_H__

#include "PvBaseLib.h"
#include "PvResult.h"
#include "PvString.h"


namespace PvBaseLib
{
    class TftpClient;
}


class PV_BASE_API PvTftpClient
{
public:

    PvTftpClient();
    ~PvTftpClient();

    PvResult Init( const PvString &aDeviceIPAddress, const PvString &aDeviceFilename );

    PvResult SendFile( const PvString &aHostFilename = "" );
    PvResult SendData( const uint8_t *aBuffer, uint32_t aBufferLength );
    PvResult GetFile( const PvString &aHostFilename = "" );
    PvResult GetData( uint8_t *aBuffer, uint32_t aBufferLength, uint32_t *aBytesRead );

    PvResult Abort();
    PvResult GetWarning( PvString &aWarning );
    PvResult GetTransferResult() const;
    PvResult GetProgress( int64_t &aCompleted, int64_t &aTotal );

protected:

private:

#ifndef PV_GENERATING_DOXYGEN_DOC

    PvBaseLib::TftpClient *mThis;

#endif // PV_GENERATING_DOXYGEN_DOC

    // Not implemented
    PvTftpClient( const PvTftpClient & );
    const PvTftpClient &operator=( const PvTftpClient & );

};


#endif
