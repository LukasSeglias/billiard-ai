// *****************************************************************************
//
//     Copyright (c) 2012, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#pragma once

#include "afxmt.h"

#include "Constants.h"
#include "SmartBuffer.h"

#include "PvBuffer.h"

///
/// \class CurrentBuffersTable
///
/// \brief Manage the buffers provided by the stream thread to only
/// keep the last ( current ) one and return the other to the onwer. 
/// On the other side, a snapshop can be taken by the transmitter thread
/// at a given moment to be converted and transmitted.
///
class CurrentBuffersTable
{
public: 
    ///
    /// \brief Constructor
    ///
    CurrentBuffersTable();

    ///
    /// \brief Destructor
    ///
    virtual ~CurrentBuffersTable();

    ///
    /// \brief Set a new buffer in the internal table and manage the existing one
    ///
    /// \param aBuffer New buffer to add
    /// \param aRow The tiling row index of the steam
    /// \param aColumn The tiling column index of the steam
    ///
    void Set( SmartBuffer* aBuffer, int aRow, int aColumn );

    ///
    /// \brief Lock the current buffers in memory for usage by the application
    ///
    /// \note the function GetBufferFromSnapshot can only be used when the snapshot is locked
    ///
    void LockSnapshot();

    ///
    /// \brief Get the last buffer from the snapshot
    ///
    /// \note the function GetBufferFromSnapshot can only be used when the snapshot is locked
    ///
    /// \param aRow The tiling row index of the steam
    /// \param aColumn The tiling column index of the steam
    ///
    /// \return The buffer or NULL is nothing set yet
    ///
    inline PvBuffer* GetBufferFromSnapshot( int aRow, int aColumn )
    {
        return mSnapshot[ aRow ][aColumn ];
    }

    ///
    /// \brief Unlock the current buffers in memory for usage by the application
    ///
    /// \note the function GetBufferFromSnapshot can only be used when the snapshot is locked
    ///
    void UnlockSnapshot();

    ///
    /// \brief Reset the current buffer at a given location
    ///
    /// \param aRow The tiling row index of the steam
    /// \param aColumn The tiling column index of the steam
    ///
    void Reset( int aRow, int aColumn );

private:

    // Protect the access to the use count
    CCriticalSection mCriticalSection;

    // This is the storage for the buffer. We keep the most recent one from each stream
    // to be ready to provide a snapshot.
    SmartBuffer* mCurrentTable[ MAX_TILES_ROW ][ MAX_TILES_COLUMN ];

    // This memory is used when making a snapshot of the current block table
    SmartBuffer* mSnapshot[ MAX_TILES_ROW ][ MAX_TILES_COLUMN ];
};