// *****************************************************************************
//
//     Copyright (c) 2012, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#ifndef __PTRESULT_H__
#define __PTRESULT_H__

#include "PtUtilsLib.h"
#include "PtTypes.h"
#include "PtString.h"


class PT_UTILS_LIB_API PtResult
{

public:

    PtResult();
    PtResult( uint32_t aCode );
    PtResult( uint32_t aCode, const PtString & aDescription );

    // copy constructor
    PtResult( const PtResult& aResult );

    // The destructor is not virtual to make as much efficient as possible using
    // the object as return value.
	~PtResult();

    operator const char  * () const;
    PtResult& operator = (const PtResult & aB);
    bool operator == ( const PtResult & aB ) const;
    bool operator == ( const uint32_t aCode ) const;
    bool operator != ( const PtResult & aB ) const;
    bool operator != ( const uint32_t aCode ) const;
	const PtResult & operator |= ( const PtResult & aB );

	void SetCode( uint32_t aIn );
    uint32_t GetCode() const;
    PtString GetCodeString() const;
    PtString GetDescription() const;
    void SetDescription( const PtString & aDescription );

    bool IsFailure() const;
    bool IsOK() const;
	bool IsPending() const;
    bool IsSuccess() const;

	struct PT_UTILS_LIB_API Code
	{
		static const uint32_t OK;
		static const uint32_t NOT_INITIALIZED;       
		static const uint32_t NOT_FOUND;      
        static const uint32_t BUSY;
		static const uint32_t CANNOT_OPEN_FILE;         
		static const uint32_t NOT_CONNECTED;            
		static const uint32_t STATE_ERROR;
		static const uint32_t INVALID_DATA_FORMAT;   
		static const uint32_t ABORTED;
		static const uint32_t NOT_ENOUGH_MEMORY;
		static const uint32_t GENERIC_ERROR;
		static const uint32_t INVALID_PARAMETER;
		static const uint32_t CANCEL;
		static const uint32_t PENDING;
        static const uint32_t TIMEOUT;
        static const uint32_t NO_DRIVER;
        static const uint32_t NO_LICENSE;
        static const uint32_t GENICAM_XML_ERROR;
        static const uint32_t CANT_READ_MANIFEST;
        static const uint32_t NOT_IMPLEMENTED;
        static const uint32_t NOT_SUPPORTED;
        static const uint32_t FILE_ERROR;
        static const uint32_t ERR_OVERFLOW;
        static const uint32_t IMAGE_ERROR;
        static const uint32_t MISSING_PACKETS;
        static const uint32_t BUFFER_TOO_SMALL;
        static const uint32_t TOO_MANY_RESENDS;
        static const uint32_t RESENDS_FAILURE;
        static const uint32_t TOO_MANY_CONSECUTIVE_RESENDS;
        static const uint32_t AUTO_ABORTED;
        static const uint32_t BAD_VERSION;
        static const uint32_t NO_MORE_ENTRY;
        static const uint32_t NO_AVAILABLE_DATA;
        static const uint32_t NETWORK_ERROR;
        static const uint32_t REBOOT_NEEDED;
        static const uint32_t REBOOT_AND_RECALL;
        static const uint32_t CORRUPTED_DATA;
        static const uint32_t UNEXPECTED_ERROR;
    };

private:

	uint32_t mCode;
    PtString* mDescription;

};

#endif
