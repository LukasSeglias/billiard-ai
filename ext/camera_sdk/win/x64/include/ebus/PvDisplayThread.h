// *****************************************************************************
//
//     Copyright (c) 2010, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#ifndef __PVDISPLAYTHREAD_H__
#define __PVDISPLAYTHREAD_H__

#include "PvAppUtilsLib.h"
#include "PvPropertyList.h"
#include "PvDeinterlacingType.h"
#include "PvTapGeometry.h"


namespace PvAppUtilsLib
{
    class DisplayThread;
    class DisplayThreadProxy;

}; // namespace PvAppUtilsLib

class PvBuffer;
class PvPipeline;
class PvGenParameterArray;


class PV_APPUTILS_API PvDisplayThread
{
public:

    PvDisplayThread();
    virtual ~PvDisplayThread();

    PvResult Start( PvPipeline *aPipeline, PvGenParameterArray *aParameters );
    PvResult Stop( bool aWait );
    PvResult WaitComplete();
    bool IsRunning() const;

    uint32_t GetPriority() const;
    PvResult SetPriority( uint32_t aPriority );

    PvBuffer *RetrieveLatestBuffer();
    void ReleaseLatestBuffer();

    bool GetKeepPartialImagesEnabled() const;
    void SetKeepPartialImagesEnabled( bool aEnabled );

    bool GetBufferLogErrorEnabled() const;
    void SetBufferLogErrorEnabled( bool aValue );

    bool GetBufferLogAllEnabled() const;
    void SetBufferLogAllEnabled( bool aValue );

    bool GetChunkLogEnabled() const;
    void SetChunkLogEnabled( bool aValue );

    PvDeinterlacingType GetDeinterlacing() const;
    void SetDeinterlacing( PvDeinterlacingType aValue );

    uint32_t GetFPS() const;
    uint32_t GetTargetFPS() const;
    void SetTargetFPS( uint32_t aValue );

    bool GetVSyncEnabled() const;
    void SetVSyncEnabled( bool aEnabled );

    bool GetDisplayChunkDataEnabled() const;
    void SetDisplayChunkDataEnabled( bool aEnabled );

    PvTapGeometryEnum GetTapGeometry() const;
    void SetTapGeometry( PvTapGeometryEnum aGeometry );

    void ResetStatistics();

    virtual PvResult Save( PvPropertyList &aPropertyList );
    virtual PvResult Load( PvPropertyList &aPropertyList );

protected:

    virtual void OnBufferRetrieved( PvBuffer *aBuffer );
    virtual void OnBufferDisplay( PvBuffer *aBuffer );
    virtual void OnBufferDone( PvBuffer *aBuffer );
    virtual void OnBufferLog( const PvString &aLog );
    virtual void OnBufferTextOverlay( const PvString &aText );

private:

    PvAppUtilsLib::DisplayThread *mThis;
    friend class PvAppUtilsLib::DisplayThreadProxy;

	 // Not implemented
	PvDisplayThread( const PvDisplayThread & );
	const PvDisplayThread &operator=( const PvDisplayThread & );

};

#endif
