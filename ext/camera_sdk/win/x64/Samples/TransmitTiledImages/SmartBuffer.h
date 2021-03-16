// *****************************************************************************
//
//     Copyright (c) 2012, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#pragma once

#include <PvBuffer.h>

#include "ProtectedQueue.h"

///
/// \class SmartBuffer
///
/// \brief PvBuffer that can be used as a smart object with a reference count
/// The memory manipulation is self contains.
///
class SmartBuffer
    : public PvBuffer
{
public:
    ///
    /// \brief Constructor
    ///
    /// \brief aReturnQueue Queue used to return the buffer to the stream thread
    ///
    SmartBuffer( ProtectedQueue<SmartBuffer>* aReturnQueue )
        : PvBuffer( PvPayloadTypeImage )
        , mReturnQueue( aReturnQueue )
        , mUseCount( 0 )
    {
    }

    ///
    /// \brief Destructor
    ///
    virtual ~SmartBuffer()
    {
    }

    ///
    /// \brief Increase the internal use count of the buffer
    ///
    inline void IncreaseCount()
    {
        mCriticalSection.Lock();

        mUseCount++;
    
        mCriticalSection.Unlock();
    }

    ///
    /// \brief Decrease the internal use count of the buffer
    /// If no more user, the buffer is returned to the stream via the return queue
    ///
    inline void DecreaseCount()
    {
        mCriticalSection.Lock();

        if( ! ( --mUseCount ) )
        {
            mReturnQueue->Push( this );
        }
    
        mCriticalSection.Unlock();
    }

private:
    // Queue to return the buffer to the stream without blocking the 
    // acquisition path
    ProtectedQueue<SmartBuffer>* mReturnQueue;

    // Keep track of the number of user to be able to release
    // the buffer to the stream only when needed
    int mUseCount; 

    // Protection for the use count
    CCriticalSection mCriticalSection;
};