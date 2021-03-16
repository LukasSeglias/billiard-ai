// *****************************************************************************
//
//     Copyright (c) 2012, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#pragma once

#include "malloc.h"

///
/// \class Queue
///
/// \brief Queue template
///
template <class T>
class Queue
{
public:
    ///
    /// \brief Constructor
    ///
    Queue()
        : mSize( 0 )
        , mHead( 0 )
        , mTail( 0 )
        , mItems( NULL )
    {
    }

    ///
    /// \brief Destructor
    ///
    virtual ~Queue() 
    {
        if ( mItems )
        {
            free( mItems );
            mItems = NULL;
        }
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
        mItems = ( T** ) malloc( sizeof( T* ) * aSize );
        if( !mItems )
        {
            return false;
        }
        
        mSize = aSize;
        return true;
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
        if( ( ( mTail + 1 ) % mSize ) == mHead )
        {
            return false;
        }

        mItems[ mTail ] = aItem;
        mTail++;
        mTail %= mSize;
        return true;
    }

    ///
    /// \brief Pop the first element from the queue
    /// The element is REMOVED from the queue by this call
    /// 
    /// \return The element
    ///
    inline T* Pop()
    {
        T* lItem = NULL;
        
        if ( mTail != mHead )
        {
            lItem = mItems[ mHead ];
            mHead++;
            mHead %= mSize;
        }
        return lItem;
    }

private:
    // Index of the head
    int mHead;

    // Index of the tail
    int mTail;

    // Size of the queue
    int mSize;

    // Storage for the pointers
    T** mItems;
};