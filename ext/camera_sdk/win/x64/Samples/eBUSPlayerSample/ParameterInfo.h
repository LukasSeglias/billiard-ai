// *****************************************************************************
//
//     Copyright (c) 2007, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#ifndef __PARAMETER_INFO_H__
#define __PARAMETER_INFO_H__

#include <PvGenParameterArray.h>


class LogBuffer;


class ParameterInfo : public PvGenEventSink
{
#ifdef _AFXDLL
    DECLARE_DYNAMIC( ParameterInfo )
#endif // _AFXDLL

public:

    ParameterInfo();
    ~ParameterInfo();

    void Initialize( PvGenParameter *aParameter, LogBuffer *aLogBuffer );
    void Reset();

    // Used to temporarily disabling node map access
    bool IsEnabled() const { return mEnabled; }
    void Enable( bool aEnabled ) { mEnabled = aEnabled; }

private:

    void OnParameterUpdate( PvGenParameter *aParameter );

    void SetLastValue();
    void LogToBuffer();

    PvGenParameter *mParameter;
    PvString mLastValue;

    LogBuffer *mLogBuffer;

    bool mEnabled;
};

#endif // __PARAMETER_INFO_H__
