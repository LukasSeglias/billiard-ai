// *****************************************************************************
//
//     Copyright (c) 2012, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#include "stdafx.h"

#include "CurrentBuffersTable.h"

CurrentBuffersTable::CurrentBuffersTable()
{
    for( int i = 0; i < MAX_TILES_ROW; i++ )
    {
        for( int j = 0; j < MAX_TILES_COLUMN; j++ )
        {
            mCurrentTable[ i ][ j ] = NULL;
            mSnapshot[ i ][ j ] = NULL;
        }
    }
}

CurrentBuffersTable::~CurrentBuffersTable()
{
}

void CurrentBuffersTable::Set( SmartBuffer* aBuffer, int aRow, int aColumn )
{
    mCriticalSection.Lock();

    // If the element was already in the list, we do not need it anymore
    if( mCurrentTable[ aRow ][ aColumn ] )
    {
        mCurrentTable[ aRow ][ aColumn ]->DecreaseCount();
    }

    // Hold the new buffer and lock it into the list
    aBuffer->IncreaseCount();
    mCurrentTable[ aRow ][ aColumn ] = aBuffer;

    mCriticalSection.Unlock();
}

void CurrentBuffersTable::LockSnapshot()
{
    mCriticalSection.Lock();
    for( int i = 0; i < MAX_TILES_ROW; i++ )
    {
        for( int j = 0; j < MAX_TILES_COLUMN; j++ )
        {
            // Increase the use count of the element
            if( mCurrentTable[ i ][ j ] )
            {
                mCurrentTable[ i ][ j ]->IncreaseCount();
            }
            mSnapshot[ i ][ j ] = mCurrentTable[ i ][ j ];
        }
    }
    mCriticalSection.Unlock();
}

void CurrentBuffersTable::UnlockSnapshot()
{
    mCriticalSection.Lock();
    for( int i = 0; i < MAX_TILES_ROW; i++ )
    {
        for( int j = 0; j < MAX_TILES_COLUMN; j++ )
        {
            // we only want to return the buffer if it has been replace in 
            // the current table. Otherwise, we keep it for potential later usage
            if( mSnapshot[ i ][ j ] )
            {
                mSnapshot[ i ][ j ]->DecreaseCount();                
                mSnapshot[ i ][ j ] = NULL;
            }

        }
    }
    mCriticalSection.Unlock();
}

void CurrentBuffersTable::Reset( int32_t aRow, int32_t aColumn )
{
    mCriticalSection.Lock();

    if( mCurrentTable[ aRow ][ aColumn ] )
    {
        mCurrentTable[ aRow ][ aColumn ]->DecreaseCount();
        mCurrentTable[ aRow ][ aColumn ] = NULL;
    }

    mCriticalSection.Unlock();
}
