// *****************************************************************************
//
//     Copyright (c) 2007, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#ifndef __PVRESULT_H__
#define __PVRESULT_H__

#include "PvString.h"


class PV_BASE_API PvResult
{

public:

    PvResult();
    PvResult( uint32_t aCode );
    PvResult( uint32_t aCode, uint32_t aOSCode );
    PvResult( uint32_t aCode, const PvString & aDescription );
    PvResult( uint32_t aCode, uint32_t aOSCode, const PvString & aDescription );

    // copy constructor
    PvResult( const PvResult& aResult );

    // The destructor is not virtual to make as much efficient as possible using
    // the object as return value.
	~PvResult();

    operator const char  * () const;
    PvResult& operator = (const PvResult & aB);
    bool operator == ( const PvResult & aB ) const;
    bool operator == ( const uint32_t aCode ) const;
    bool operator != ( const PvResult & aB ) const;
    bool operator != ( const uint32_t aCode ) const;
	const PvResult & operator |= ( const PvResult & aB );

	void SetCode( uint32_t aIn );
    uint32_t GetCode() const;
    PvString GetCodeString() const;
    PvString GetDescription() const;
    void SetDescription( const PvString & aDescription );

    bool IsFailure() const;
    bool IsOK() const;
	bool IsPending() const;
    bool IsSuccess() const;

    // Can be used to retrieve internal diagnostic information
    uint32_t GetInternalCode() const;
    uint32_t GetOSCode() const;
    
    struct Code
    {
	enum CodeEnum
	{
        OK = 0,
        NOT_INITIALIZED = 0x0605,
        NOT_FOUND = 0x0019,
        CANNOT_OPEN_FILE = 0x0006,
        NO_MORE_ITEM = 0x0015,
        NOT_CONNECTED = 0x0017,         
        STATE_ERROR = 0x001c,
        THREAD_ERROR = 0x001d,
        INVALID_DATA_FORMAT = 0x0501,
        ABORTED = 0x0001,
        NOT_ENOUGH_MEMORY = 0x0018,
        GENERIC_ERROR = 0x4000,
        INVALID_PARAMETER = 0x4001,
        CANCEL = 0x4002,
        PENDING = 0xffff,
        TIMEOUT = 0x001e,
        NO_LICENSE = 0x0602,
        GENICAM_XML_ERROR = 0x0904,
        CANT_READ_MANIFEST = 0x0905,
        NOT_IMPLEMENTED = 0x0604,
        NOT_SUPPORTED = 0x001a,
        FILE_ERROR = 0x0010,
        ERR_OVERFLOW = 0x001b,
        IMAGE_ERROR = 0x0025,
        MISSING_PACKETS = 0x0027,
        BUFFER_TOO_SMALL = 0x0004,
        TOO_MANY_RESENDS = 0x0b00,
        RESENDS_FAILURE = 0x0b01,
        TOO_MANY_CONSECUTIVE_RESENDS = 0x0b03,
        AUTO_ABORTED = 0x0b02,
        BAD_VERSION = 0x0201,
        NO_MORE_ENTRY = 0x0603,
        NO_AVAILABLE_DATA = 0x0014,
        NETWORK_ERROR = 0x0013,
        RESYNC = 0x0028,
        BUSY = 0x0202,
        CORRUPTED_DATA = 0x0303
	};
    };

private:

	uint32_t mCode;
    uint32_t mInternalCode;
    uint32_t mOSCode;
    PvString* mDescription;

};


//
// Direct #defines or the PvResult::Code - typically used to solve
// delay-loading issues
//

#define PV_OK ( 0 )
#define PV_NOT_INITIALIZED ( 0x0605 )
#define PV_NOT_FOUND ( 0x0019 )
#define PV_CANNOT_OPEN_FILE (0x0006 )
#define PV_NO_MORE_ITEM ( 0x0015 )
#define PV_NOT_CONNECTED ( 0x0017 )         
#define PV_STATE_ERROR ( 0x001c )
#define PV_THREAD_ERROR ( 0x001d )
#define PV_INVALID_DATA_FORMAT ( 0x0501 )
#define PV_ABORTED ( 0x0001 )
#define PV_NOT_ENOUGH_MEMORY ( 0x0018 )
#define PV_GENERIC_ERROR ( 0x4000 )
#define PV_INVALID_PARAMETER ( 0x4001 )
#define PV_CANCEL ( 0x4002 )
#define PV_PENDING ( 0xffff )
#define PV_TIMEOUT ( 0x001e )
#define PV_NO_LICENSE ( 0x0602 )
#define PV_GENICAM_XML_ERROR ( 0x0904 )
#define PV_CANT_READ_MANIFEST ( 0x0905 )
#define PV_NOT_IMPLEMENTED ( 0x0604 )
#define PV_NOT_SUPPORTED ( 0x001a )
#define PV_FILE_ERROR ( 0x0010 )
#define PV_ERR_OVERFLOW ( 0x001b )
#define PV_IMAGE_ERROR ( 0x0025 )
#define PV_MISSING_PACKETS ( 0x0027 )
#define PV_BUFFER_TOO_SMALL ( 0x0004 )
#define PV_TOO_MANY_RESENDS ( 0x0b00 )
#define PV_RESENDS_FAILURE ( 0x0b01 )
#define PV_TOO_MANY_CONSECUTIVE_RESENDS ( 0x0b03 )
#define PV_AUTO_ABORTED ( 0x0b02 )
#define PV_BAD_VERSION ( 0x0201 )
#define PV_NO_MORE_ENTRY ( 0x0603 )
#define PV_NO_AVAILABLE_DATA ( 0x0014 )
#define PV_NETWORK_ERROR ( 0x0013 )
#define PV_RESYNC ( 0x0028 )
#define PV_BUSY ( 0x0202 )
#define PV_CORRUPTED_DATA ( 0x0303 )

#endif
