// *****************************************************************************
//
//     Copyright (c) 2013, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#ifndef __PVDFWPAYLOAD_H__
#define __PVDFWPAYLOAD_H__

#include "PvBaseLib.h"
#include "PvString.h"
#include "PvResult.h"


namespace PvBaseLib
{
    class DfwPayload;
}


class PV_BASE_API PvDfwPayload
{
public:

    PvDfwPayload( const PvString &aFilename );
    PvDfwPayload( uint8_t *aBuffer, uint32_t aBufferLength );
    virtual ~PvDfwPayload();

    static PvResult GetResetRecord( const PvString &aFirmwareFilename, uint8_t *aBuffer, uint32_t aBufferLength, uint32_t &aBytesWritten );
    static PvResult GetDeviceIdRecord( const PvString &aFirmwareFilename, const PvString &aNewDeviceId, uint8_t *aBuffer, uint32_t aBufferLength, uint32_t &aBytesWritten );

protected:

private:

#ifndef PV_GENERATING_DOXYGEN_DOC

    PvBaseLib::DfwPayload *mThis;

#endif // PV_GENERATING_DOXYGEN_DOC

    // Not implemented
    PvDfwPayload();
    PvDfwPayload( const PvDfwPayload & );
    const PvDfwPayload &operator=( const PvDfwPayload & );

};

#endif
