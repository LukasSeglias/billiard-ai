// *****************************************************************************
//
//     Copyright (c) 2013, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#ifndef __BUFFER_OPTIONS_H__
#define __BUFFER_OPTIONS_H__

#include "Persistent.h"

#include <PvPipeline.h>


class Player;


class BufferOptions : public Persistent
#ifdef _AFXDLL
    , public CObject
#endif // _AFXDLL
{
#ifdef _AFXDLL
    DECLARE_DYNAMIC( BufferOptions )
#endif // _AFXDLL

public:

    BufferOptions();
    ~BufferOptions();

    // Persistent
    PvResult Save( PvConfigurationWriter *aWriter );
    PvResult Load( PvConfigurationReader *aReader );

    PvString GetHelpBufferCount() const;
    PvString GetHelpBufferSize() const;
    PvString GetHelpAutoResizing() const;

    bool HasChanged( PvPipeline *aPipeline );
    bool ShowWarning();

    void Init( PvPipeline *aPipeline );
    PvResult Apply( PvPipeline *aPipeline );

    double GetRequiredMemoryInGB() const { return static_cast<double>( mBufferCount * mBufferSize ) / ( 1024.0 * 1024.0 * 1024.0 ); }

    uint32_t GetBufferCount() const { return mBufferCount; }
    uint32_t GetBufferSize() const { return mBufferSize; }
    bool GetAutoResize() const { return mAutoResize; }

    void SetBufferCount( uint32_t aValue ) { mBufferCount = aValue; }
    void SetBufferSize( uint32_t aValue ) { mBufferSize = aValue; }
    void SetAutoResize( bool aValue ) { mAutoResize = aValue; }

    void operator=( const BufferOptions& aObjectSource );

protected:

private:

    uint32_t mBufferCount;
    uint32_t mBufferSize;

    bool mAutoResize;

};


#endif // __BUFFER_OPTIONS_H__

