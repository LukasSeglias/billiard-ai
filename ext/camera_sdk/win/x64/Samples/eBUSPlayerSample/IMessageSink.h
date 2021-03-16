// *****************************************************************************
//
//     Copyright (c) 2013, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#ifndef __IMESSAGE_SINK_H__
#define __IMESSAGE_SINK_H__

#include <PvResult.h>


class IMessageSink
{
public:

    virtual intptr_t PostMsg( uint32_t aMessage, uintptr_t wParam = 0, intptr_t lParam = 0 ) = 0;
    virtual intptr_t SendMsg( uint32_t aMessage, uintptr_t wParam = 0, intptr_t lParam = 0 ) = 0;
    virtual intptr_t SendMsgIfPossible( uint32_t aMessage, uintptr_t wParam = 0, intptr_t lParam = 0 ) = 0;

};


#endif // __IMESSAGE_SINK_H__

