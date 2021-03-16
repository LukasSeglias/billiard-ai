// *****************************************************************************
//
//     Copyright (c) 2012, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#pragma once

#include "afxmt.h"

#include "Queue.h"

///
/// \class ProtectedQueue
///
/// \brief Thread-safe queue template
///
template <class T>
class ProtectedQueue
{
public:
    ///
    /// \brief Constructor
    ///
    ProtectedQueue()
    {
    }

    ///
    /// \brief Destructor
    ///
    virtual ~ProtectedQueue() 
    {
    }

    ///
    /// \brief Initialize the queue
    ///
    /// \param aSize Maximun size of the queue
    ///
    /// \return false on error
    ///
    inline bool Initialize( int aSize ) 
    {
        return mQueue.Initialize( aSize );
    }

    ///
    /// \brief Push a new element on the queue
    ///
    /// \param aItem New element
    ///
    /// \return false on error
    ///
    inline bool Push( T* aItem )
    {
        bool lReturn;

        mCriticalSection.Lock();

        lReturn = mQueue.Push( aItem );

        mCriticalSection.Unlock();

        return lReturn;
    }

    ///
    /// \brief Pop the first element from the queue
    /// The element is REMOVED from the queue by this call
    /// 
    /// \return The element
    ///
    inline T* Pop()
    {
        T* lItem;

        mCriticalSection.Lock();

        lItem = mQueue.Pop();

        mCriticalSection.Unlock();

        return lItem;
    }

private:
    // Protection for a multi-thread safe queue
    CCriticalSection mCriticalSection;

    // Internally used an un-protected queue
    Queue<T> mQueue;
};
