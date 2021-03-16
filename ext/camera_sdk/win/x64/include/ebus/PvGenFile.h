// *****************************************************************************
//
//     Copyright (c) 2009, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#ifndef __PVGENFILE_H__
#define __PVGENFILE_H__

#include "PvGenParameterArray.h"
#include "PvStringList.h"


typedef enum 
{
	PvGenOpenModeWrite = 0,
	PvGenOpenModeRead = 1,
	PvGenOpenModeUndefined = 999,

} PvGenOpenMode;


namespace PvGenICamLib
{
    class GenFile;
}


class PV_GENICAM_API PvGenFile
{
public:

    PvGenFile();
    virtual ~PvGenFile();

    PvResult Open( PvGenParameterArray *aArray, const PvString &aFilename, PvGenOpenMode aMode );
    PvResult Close();

    bool IsOpened() const;

    PvResult WriteFrom( const PvString &aLocalFilename );
    PvResult ReadTo( const PvString &aLocalFilename );

    PvResult Write( const uint8_t *aBuffer, int64_t aLength, int64_t &aBytesWritten );
    PvResult Read( uint8_t *aBuffer, int64_t aLength, int64_t &aBytesRead );

    PvResult GetStatus( PvString &aStatus );
    PvString GetLastErrorMessage() const;

    PvResult GetProgress( int64_t &aCompleted, int64_t &aTotal );

    static bool IsSupported( PvGenParameterArray *aArray );
    static bool IsReadable( PvGenParameterArray *aArray, const PvString &aFilename );
    static bool IsWritable( PvGenParameterArray *aArray, const PvString &aFilename );
    static void GetFiles( PvGenParameterArray *aArray, PvStringList &aFiles );

private:

    // Not implemented
	PvGenFile( const PvGenFile & );
	const PvGenFile &operator=( const PvGenFile & );

    PvGenICamLib::GenFile *mThis;
};

#endif
