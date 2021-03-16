// *****************************************************************************
//
// Copyright (c) 2017, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#pragma once

#include <list>
#include <PvPixelType.h>


class PvBuffer;
class PvImage;
class PvChunkData;

typedef std::list<PvBuffer *> BufferList;


typedef enum
{
    TestPatternDiagonalMoving = 0,
    TestPatternDiagonalStatic,
    TestPatternHorizontalMoving,
    TestPatternHorizontalStatic,
    TestPatternVerticalMoving,
    TestPatternVerticalStatic

} TestPattern;


class DataGenerator
{
public:

    DataGenerator();
    virtual ~DataGenerator();

    void AllocImages( PvPixelType aPixelType, uint32_t aWidth, uint32_t aHeight, uint16_t aPaddingX = 0, uint16_t aPaddingY = 0 );
    void AllocChunkData( uint32_t aSize );

    TestPattern GetTestPattern() const { return mTestPattern; }
    void SetTestPattern( TestPattern aPattern ) { mTestPattern = aPattern; }

    bool GetNext( PvBuffer **aImage, PvBuffer **aChunkData );
    void Release( PvBuffer *aBuffer );

protected:

    void FreeBuffers();
    void FrameComplete();
    void GenerateTestPattern( PvImage *aImage );
    void GenerateTestPatternDiagonal( PvImage *aImage );
    void GenerateTestPatternHorizontal( PvImage *aImage );
    void GenerateTestPatternVertical( PvImage *aImage );
    void GenerateLFSR( PvChunkData *aChunkData );

private:

    BufferList mBuffers;
    BufferList mImages;
    BufferList mChunks;

    PvPixelType mPixelType;
    uint32_t mWidth;
    uint32_t mHeight;
    uint16_t mPaddingX;
    uint16_t mPaddingY;

    uint32_t mChunkDataSize;
    uint8_t *mChunkData;

    TestPattern mTestPattern;

    uint64_t mSeed;

};

