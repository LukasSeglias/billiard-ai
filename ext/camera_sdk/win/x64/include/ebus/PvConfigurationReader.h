// *****************************************************************************
//
//     Copyright (c) 2008, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#ifndef __PVCONFIGURATIONREADER_H__
#define __PVCONFIGURATIONREADER_H__

#include "PvPersistenceLib.h"
#include "PvDevice.h"
#include "PvStream.h"
#include "PvStringList.h"
#include "PvPropertyList.h"


namespace PvPersistenceLib
{
    class ConfigurationReader;
}


class PvConfigurationReader
{
public:
    
    PV_PERSISTENCE_API PvConfigurationReader();
    PV_PERSISTENCE_API ~PvConfigurationReader();
    
    PV_PERSISTENCE_API PvResult Load( const PvString &aFilename );
	PV_PERSISTENCE_API PvResult LoadFromString( const PvString &aString );
    
    PV_PERSISTENCE_API uint32_t GetDeviceCount();
    PV_PERSISTENCE_API PvResult GetDeviceName( uint32_t aIndex, PvString &aName );
    PV_PERSISTENCE_API PvResult Restore( const PvString &aName, PvDevice *aDevice );
    PV_PERSISTENCE_API PvResult Restore( uint32_t aIndex, PvDevice *aDevice );
      
    PV_PERSISTENCE_API uint32_t GetStreamCount();
    PV_PERSISTENCE_API PvResult GetStreamName( uint32_t aIndex, PvString &aName );
    PV_PERSISTENCE_API PvResult Restore( const PvString &aName, PvStream *aStream );
    PV_PERSISTENCE_API PvResult Restore( uint32_t aIndex, PvStream *Stream );
    
    PV_PERSISTENCE_API uint32_t GetStringCount();
    PV_PERSISTENCE_API PvResult GetStringName( uint32_t aIndex, PvString &aName );
    PV_PERSISTENCE_API PvResult Restore( const PvString &aKey, PvString &aValue );
    PV_PERSISTENCE_API PvResult Restore( uint32_t aIndex, PvString &aValue );

    PV_PERSISTENCE_API uint32_t GetGenParameterArrayCount();
    PV_PERSISTENCE_API PvResult GetGenParameterArrayName( uint32_t aIndex, PvString &aName );
    PV_PERSISTENCE_API PvResult Restore( const PvString &aKey, PvGenParameterArray *aParameterArray );
    PV_PERSISTENCE_API PvResult Restore( uint32_t aIndex, PvGenParameterArray *aParameterArray );

    PV_PERSISTENCE_API uint32_t GetPropertyListCount();
    PV_PERSISTENCE_API PvResult GetPropertyListName( uint32_t aIndex, PvString &aName );
    PV_PERSISTENCE_API PvResult Restore( const PvString &aKey, PvPropertyList *aPropertyList );
    PV_PERSISTENCE_API PvResult Restore( uint32_t aIndex, PvPropertyList *aPropertyList );

    PV_PERSISTENCE_API void SetErrorList( PvStringList *aList, const PvString &aPrefix );

private:

    PvPersistenceLib::ConfigurationReader *mThis;
    
    PvStringList *mErrorList;
    PvString mErrorPrefix;

	 // Not implemented
	PvConfigurationReader( const PvConfigurationReader& );
	const PvConfigurationReader &operator=( const PvConfigurationReader & );

};

#endif
