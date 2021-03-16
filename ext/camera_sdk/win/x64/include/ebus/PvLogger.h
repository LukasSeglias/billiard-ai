// *****************************************************************************
//
//     Copyright (c) 2016, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#ifndef __PVLOGGER_H__
#define __PVLOGGER_H__

#include "PvResult.h"

#include <sstream>


namespace PtUtilsLib
{
    class Logger;
}

namespace PvBaseLib
{
    class LogSink;
}


enum PvLogLevelEnum
{
    PvLogLevelUnknown,
    PvLogLevelInfo,
    PvLogLevelError,
    PvLogLevelWarning,
    PvLogLevelCritical,
    PvLogLevelDebug
};


class PV_BASE_API PvLogger
{
public:

    PvLogger( const PvString &aCategory );
    virtual ~PvLogger();

    void LogInfo( const char *aFile, int aLine, const char *aFunction, const char *aMessage );
    void LogError( const char *aFile, int aLine, const char *aFunction, const char *aMessage );
    void LogWarning( const char *aFile, int aLine, const char *aFunction, const char *aMessage );
    void LogCritical( const char *aFile, int aLine, const char *aFunction, const char *aMessage );
    void LogDebug( const char *aFile, int aLine, const char *aFunction, const char *aMessage );

private:

    // Not implemented
    PvLogger();
    PvLogger( const PvLogger & );
    const PvLogger &operator=( const PvLogger & );

    PtUtilsLib::Logger *mThis;

};


#define PV_LOGINFO( logger, entry ) { \
    std::stringstream lStr; lStr << entry; \
    ( logger ).LogInfo( __FILE__, __LINE__, __FUNCTION__, lStr.str().c_str() ); }

#define PV_LOGERROR( logger, entry ) { \
    std::stringstream lStr; lStr << entry; \
    ( logger ).LogError( __FILE__, __LINE__, __FUNCTION__, lStr.str().c_str() ); }

#define PV_LOGANDRETURN( logger, result, entry ) { \
    std::stringstream lStr; lStr << entry; \
    ( logger ).LogError( __FILE__, __LINE__, __FUNCTION__, lStr.str().c_str() ); \
    return PvResult( result, lStr.str().c_str() ); }

#define PV_LOGWARNING( logger, entry ) { \
    std::stringstream lStr; lStr << entry; \
    ( logger ).LogWarning( __FILE__, __LINE__, __FUNCTION__, lStr.str().c_str() ); }

#define PV_LOGCRITICAL( logger, entry ) { \
    std::stringstream lStr; lStr << entry; \
    ( logger ).LogCritical( __FILE__, __LINE__, __FUNCTION__, lStr.str().c_str() ); }

#define PV_LOGDEBUG( logger, entry ) { \
    std::stringstream lStr; lStr << entry; \
    ( logger ).LogDebug( __FILE__, __LINE__, __FUNCTION__, lStr.str().c_str() ); }


class PV_BASE_API PvLogSink
{
public:

    PvLogSink();
    virtual ~PvLogSink();

    virtual void Log( PvLogLevelEnum aLevel, const char *aFile, uint32_t aLine, const char *aFunction, const char *aCategory, const char *aMessage ) = 0;

private:

    // Not implemented
    PvLogSink( const PvLogSink & );
    const PvLogSink &operator=( const PvLogSink & );

    PvBaseLib::LogSink *mThis;

};

#endif
