// *****************************************************************************
//
// Copyright (c) 2017, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#include <PvSampleUtils.h>

#include "DataGenerator.h"

#include <PvBuffer.h>


#define BUFFERID_IMAGE ( 0x1234 )
#define BUFFERID_CHUNK ( 0x5678 )

#define BUFFER_COUNT ( 8 )

#define CHUNK_COUNT ( 1 )
#define CHUNK_OVERHEAD ( 64 )
#define CHUNK_ID ( 0xABCD )


///
/// \brief Constructor
///

DataGenerator::DataGenerator()
    : mPixelType( PvPixelUndefined )
    , mWidth( 0 )
    , mHeight( 0 )
    , mPaddingX( 0 )
    , mPaddingY( 0 )
    , mChunkDataSize( 0 )
    , mChunkData( NULL )
    , mTestPattern( TestPatternDiagonalMoving )
    , mSeed( 0 )
{
}


///
/// \brief Destructor
///

DataGenerator::~DataGenerator()
{
    FreeBuffers();
    PVDELETE( mChunkData );
}


///
/// \brief Frees all owned buffers
///

void DataGenerator::FreeBuffers()
{
    BufferList::iterator lIt = mBuffers.begin();
    while ( lIt != mBuffers.end() )
    {
        PVDELETE( *lIt );
        lIt++;
    }

    mBuffers.clear();
    mImages.clear();
    mChunks.clear();
}


///
/// \brief Allocates image buffers
///

void DataGenerator::AllocImages( PvPixelType aPixelType, uint32_t aWidth, uint32_t aHeight, uint16_t aPaddingX, uint16_t aPaddingY )
{
    if ( !mImages.empty() )
    {
        return;
    }

    mPixelType = aPixelType;
    mWidth = aWidth;
    mHeight = aHeight;
    mPaddingX = aPaddingX;
    mPaddingY = aPaddingY;

    for ( int i = 0; i < BUFFER_COUNT; i++ )
    {
        PvBuffer *lBuffer = new PvBuffer( PvPayloadTypeImage );
        lBuffer->SetID( BUFFERID_IMAGE );

        PvImage *lImage = lBuffer->GetImage();
        lImage->Alloc( mWidth, mHeight, mPixelType, mPaddingX, mPaddingY );

        mBuffers.push_back( lBuffer );
        mImages.push_back( lBuffer );
    }
}


///
/// \brief Allocates chunk data buffers
///

void DataGenerator::AllocChunkData( uint32_t aSize )
{
    if ( !mChunks.empty() )
    {
        return;
    }

    mChunkDataSize = aSize;
    mChunkData = new uint8_t[ mChunkDataSize ];

    for ( int i = 0; i < BUFFER_COUNT; i++ )
    {
        PvBuffer *lBuffer = new PvBuffer( PvPayloadTypeChunkData );
        lBuffer->SetID( BUFFERID_CHUNK );

        PvChunkData *lChunkData  = lBuffer->GetChunkData();
        lChunkData->Alloc( mChunkDataSize + CHUNK_COUNT * CHUNK_OVERHEAD );

        mBuffers.push_back( lBuffer );
        mChunks.push_back( lBuffer );
    }
}


///
/// \brief Returns the next test pattern image and chunk data buffers
///

bool DataGenerator::GetNext( PvBuffer **aImage, PvBuffer **aChunkData )
{
    if ( !mImages.empty() && !mChunks.empty() )
    {
        // Get iamge buffer
        *aImage = mImages.front();
        mImages.pop_front();

        // Generate test pattern
        GenerateTestPattern( ( *aImage )->GetImage() );

        // Get chunk buffer
        *aChunkData = mChunks.front();
        mChunks.pop_front();

        // Generate LFSR
        GenerateLFSR( ( *aChunkData )->GetChunkData() );

        FrameComplete();
        return true;
    }

    return false;
}


///
/// \brief Releases a buffer: queues it back for re-use
///

void DataGenerator::Release( PvBuffer *aBuffer )
{
    if ( aBuffer != NULL )
    {
        // We use buffer ID to tell image and chunk buffers apart
        switch ( aBuffer->GetID() )
        {
        case BUFFERID_IMAGE:
            // Put buffer back in images list
            mImages.push_back( aBuffer );
            break;

        case BUFFERID_CHUNK:
            // Put buffer back in chunks list
            mChunks.push_back( aBuffer );
            break;

        default:
            break;
        }
    }
}


///
/// \brief Notification from the owner that the current frame is complete.
///
/// Increments seed if needed.
///

void DataGenerator::FrameComplete()
{
    if ( ( mTestPattern == TestPatternDiagonalMoving ) ||
         ( mTestPattern == TestPatternHorizontalMoving ) ||
         ( mTestPattern == TestPatternVerticalMoving) )
    {
        mSeed++;
    }
}


///
/// \brief Generates the next test pattern
///

void DataGenerator::GenerateTestPattern( PvImage *aImage )
{
    switch ( mTestPattern )
    {
    case TestPatternDiagonalMoving:
    case TestPatternDiagonalStatic:
        GenerateTestPatternDiagonal( aImage );
        break;

    case TestPatternHorizontalMoving:
    case TestPatternHorizontalStatic:
        GenerateTestPatternHorizontal( aImage );
        break;

    case TestPatternVerticalMoving:
    case TestPatternVerticalStatic:
        GenerateTestPatternVertical( aImage );
        break;

    default:
        break;
    }
}


///
/// \brief Generates a diagonal test pattern
///

void DataGenerator::GenerateTestPatternDiagonal( PvImage *aImage )
{
    for ( uint32_t y = 0; y < mHeight; y++ )
    {
        uint8_t lIndex = static_cast<uint8_t>( ( mSeed + y ) & 0xFF );
        uint8_t *lPtr = aImage->GetDataPointer() + ( y * mWidth * aImage->GetBitsPerPixel() ) / 8 + mPaddingX;

        for ( uint32_t x = 0; x < mWidth; x++ )
        {
            *( lPtr++ ) = lIndex++;
        }
    }
}


///
/// \brief Generates an horizontal test pattern
///

void DataGenerator::GenerateTestPatternHorizontal( PvImage *aImage )
{
    for ( uint32_t y = 0; y < mHeight; y++ )
    {
        uint8_t lIndex = static_cast<uint8_t>( ( mSeed + y ) & 0xFF );
        uint8_t *lPtr = aImage->GetDataPointer() + ( y * mWidth * aImage->GetBitsPerPixel() ) / 8 + mPaddingX;

        for ( uint32_t x = 0; x < mWidth; x++ )
        {
            *( lPtr++ ) = lIndex;
        }
    }
}


///
/// \brief Generates a vertical test pattern
///

void DataGenerator::GenerateTestPatternVertical( PvImage *aImage )
{
    for ( uint32_t y = 0; y < mHeight; y++ )
    {
        uint8_t lIndex = static_cast<uint8_t>( mSeed & 0xFF );
        uint8_t *lPtr = aImage->GetDataPointer() + ( y * mWidth * aImage->GetBitsPerPixel() ) / 8 + mPaddingX;

        for ( uint32_t x = 0; x < mWidth; x++ )
        {
            *( lPtr++ ) = lIndex++;
        }
    }
}


///
/// \brief Generates the next LFSR
///

void DataGenerator::GenerateLFSR( PvChunkData *aChunkData )
{
    const uint16_t cPolynomial = 0x8016;

    // Generate LFSR chunk data
    uint8_t *lPtr = mChunkData;
    uint16_t lLFSR = static_cast<uint16_t>( 0xFF00 | ( mSeed & 0xFF ) );
    for ( uint32_t i = 0; i < mChunkDataSize; i++ )
    {
        // Write LSB byte of LFSR
        *( lPtr++ ) = static_cast<uint8_t>( lLFSR & 0xFF );

        // Calculate next LFSR state
        lLFSR = ( ( lLFSR >> 1 ) ^ ( -( lLFSR & 1 ) & cPolynomial ) );
    }

    // Add chunk to buffer
    aChunkData->AddChunk( CHUNK_ID, mChunkData, mChunkDataSize );
}

