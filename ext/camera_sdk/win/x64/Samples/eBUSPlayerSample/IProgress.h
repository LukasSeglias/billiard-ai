// *****************************************************************************
//
//     Copyright (c) 2013, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#ifndef __IPROGRESS_H__
#define __IPROGRESS_H__

#include <PvResult.h>


#define SETSTATUS( p, a ) \
    if ( p != NULL ) \
    { \
        PV_LOGINFO( mLogger, "Status: " << a ); \
        p->SetStatus( a ); \
    }


class Task;


class IProgress
{
public:

    virtual ~IProgress() {}

    virtual int RunTask( Task *aTask ) = 0;

    virtual void SetTitle( const PvString &aTitle ) = 0;

    virtual void SetStatus( const std::string &aStatus ) = 0;
    virtual void ShowWarning( const std::string &aStatus ) = 0;

};


class IProgressFactory
{
public:

    virtual IProgress *CreateProgressDialog() = 0;

};


#endif // __IPROGRESS_H__

