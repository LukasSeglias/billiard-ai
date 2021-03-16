// *****************************************************************************
//
//     Copyright (c) 2013, Pleora Technologies Inc., All rights reserved.
//
// *****************************************************************************

#ifndef __IMAGINGBUFFERALLOCATOR_H__
#define __IMAGINGBUFFERALLOCATOR_H__

#include <stdint.h>
#ifndef __MACH__
    #include <malloc.h>
#endif
#include <stdlib.h>


namespace SimpleImagingLib 
{

class ImagingBufferAllocator
{
public:
	ImagingBufferAllocator(){}

	virtual ~ImagingBufferAllocator(){}
	virtual uint8_t* Allocate( size_t aSize ) = 0;
	virtual void Free( uint8_t* aBuffer )	= 0;
};

#ifdef WIN32
class WindowsAlignedAllocator
{
public:
    uint8_t* Alloc( size_t aSize, size_t aByteAlignment ) 
	{
		return static_cast<uint8_t*>( _aligned_malloc( aSize, aByteAlignment ) );
	}

    void Free( uint8_t* aBuffer ) 
	{
		_aligned_free( aBuffer );
	}
};
#else
class StandardAllocator
{
public:
	uint8_t* Alloc( size_t size, size_t ByteAlignment ) 
	{
		return static_cast<uint8_t*>( malloc( size ) );
	}

	void Free( uint8_t* buffer ) 
	{
		::free( buffer );
	}
};
#endif

template<typename AllocatorPolicy>
class AlignedImageBufferAllocatorT : public ImagingBufferAllocator
{
public:

	AlignedImageBufferAllocatorT()
		: mByteAlignment( 16 )
	{
	}

    virtual uint8_t* Allocate( size_t aSize ) 
	{
		return static_cast<uint8_t*>( AllocatorPolicy().Alloc( aSize, mByteAlignment ) );
	}

    virtual void Free( uint8_t* aBuffer ) 
	{
		AllocatorPolicy().Free( aBuffer );
	}

private: 
	uint32_t mByteAlignment;

};

#ifdef WIN32
typedef AlignedImageBufferAllocatorT<WindowsAlignedAllocator> AlignedImageBufferAllocator;
#else
typedef AlignedImageBufferAllocatorT<StandardAllocator> AlignedImageBufferAllocator;
#endif

}

#endif
