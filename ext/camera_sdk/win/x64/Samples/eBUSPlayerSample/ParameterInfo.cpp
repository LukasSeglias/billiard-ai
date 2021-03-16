// *****************************************************************************
//
//     Copyright (c) 2007, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#include "eBUSPlayerShared.h"
#include "ParameterInfo.h"
#include "LogBuffer.h"

#include <assert.h>
#include <sstream>


#ifdef _AFXDLL
    IMPLEMENT_DYNAMIC( ParameterInfo, CObject )
#endif // _AFXDLL

        
///
/// \brief Constructor.
/// 

ParameterInfo::ParameterInfo()
    : mParameter( NULL )
    , mLogBuffer( NULL )
    , mEnabled( true )
{
}


///
/// \brief Destructor.
/// 

ParameterInfo::~ParameterInfo()
{
    if ( mParameter != NULL )
    {
        mParameter->UnregisterEventSink( this );
    }
}


///
/// \brief Initializes the object.
/// 

void ParameterInfo::Initialize( PvGenParameter *aParameter, LogBuffer *aLogBuffer )
{
    assert( aParameter != NULL );
    assert( aLogBuffer != NULL );
    
    // We do not support double-init (yet)
    assert( mParameter == NULL );
    assert( mLogBuffer == NULL );

    mParameter = aParameter;
    mLogBuffer = aLogBuffer;

    mParameter->RegisterEventSink( this );
}


///
/// \brief Reset the last known value.
/// 

void ParameterInfo::Reset()
{
    mLastValue = "";
}


///
/// \brief Callback of GenICam parameter invalidator.
/// 

void ParameterInfo::OnParameterUpdate( PvGenParameter *aParameter )
{
    if ( mEnabled )
    {
        SetLastValue();
        LogToBuffer();
    }
}


///
/// \brief Refreshes the last value of a parameter.
/// 

void ParameterInfo::SetLastValue()
{
    mLastValue = "";
    if ( !mParameter->IsReadable() || !mParameter->IsAvailable() )
    {
        return;
    }
    
    PvGenType lType;
    mParameter->GetType( lType );
    if ( lType == PvGenTypeRegister )
    {
        mLastValue = "{Register}";
        return;
    }
    
    PvString lString;
    if ( mParameter->ToString( lString ).IsOK() )
    {
        mLastValue = lString;
    }
    else
    {
        mLastValue = "NA";
    }
}


///
/// \brief Logs a parameter event to the log buffer.
/// 

void ParameterInfo::LogToBuffer()
{
    if ( !mLogBuffer->IsGenICamEnabled() )
    {
        return;
    }

    PvString lName;
    if ( !mParameter->GetName( lName ).IsOK() )
    {
        assert( 0 ); // Totally unexpected...
    }

    std::stringstream lSS;
    lSS << lName.GetAscii();
    lSS << ": ";
    lSS << mLastValue.GetAscii();

    mLogBuffer->Log( lSS.str().c_str() );
}

