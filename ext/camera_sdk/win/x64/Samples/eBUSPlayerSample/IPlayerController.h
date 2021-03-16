// *****************************************************************************
//
//     Copyright (c) 2013, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#ifndef __IPLAYER_CONTROLLER_H__
#define __IPLAYER_CONTROLLER_H__

#include <IMessageSink.h>
#include <IProgress.h>

#include <PvGenParameterArray.h>


class IPlayerController : public IMessageSink
{
public:

    virtual IProgress *CreateProgressDialog() = 0;

    virtual void StartTimer() = 0;
    virtual void StopTimer() = 0;

};


#endif // __IPLAYER_CONTROLLER_H__

