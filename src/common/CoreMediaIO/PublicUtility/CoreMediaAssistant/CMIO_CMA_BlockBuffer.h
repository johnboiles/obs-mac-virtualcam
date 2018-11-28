/*
	    File: CMIO_CMA_BlockBuffer.h
	Abstract: C++ wrapper for CMBlockBufferRef
	 Version: 1.2
	
	Disclaimer: IMPORTANT:  This Apple software is supplied to you by Apple
	Inc. ("Apple") in consideration of your agreement to the following
	terms, and your use, installation, modification or redistribution of
	this Apple software constitutes acceptance of these terms.  If you do
	not agree with these terms, please do not use, install, modify or
	redistribute this Apple software.
	
	In consideration of your agreement to abide by the following terms, and
	subject to these terms, Apple grants you a personal, non-exclusive
	license, under Apple's copyrights in this original Apple software (the
	"Apple Software"), to use, reproduce, modify and redistribute the Apple
	Software, with or without modifications, in source and/or binary forms;
	provided that if you redistribute the Apple Software in its entirety and
	without modifications, you must retain this notice and the following
	text and disclaimers in all such redistributions of the Apple Software.
	Neither the name, trademarks, service marks or logos of Apple Inc. may
	be used to endorse or promote products derived from the Apple Software
	without specific prior written permission from Apple.  Except as
	expressly stated in this notice, no other rights or licenses, express or
	implied, are granted by Apple herein, including but not limited to any
	patent rights that may be infringed by your derivative works or by other
	works in which the Apple Software may be incorporated.
	
	The Apple Software is provided by Apple on an "AS IS" basis.  APPLE
	MAKES NO WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION
	THE IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS
	FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND
	OPERATION ALONE OR IN COMBINATION WITH YOUR PRODUCTS.
	
	IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL
	OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
	SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
	INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION,
	MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED
	AND WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE),
	STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE
	POSSIBILITY OF SUCH DAMAGE.
	
	Copyright (C) 2012 Apple Inc. All Rights Reserved.
	
*/

#if !defined(__CMIO_CMA_BlockBuffer_h__)
#define __CMIO_CMA_BlockBuffer_h__

// Public Utility
#include "CMIODebugMacros.h"

// CA Public Utility
#include "CAException.h"

// System Includes
#include <CoreMedia/CMBlockBuffer.h>

// System Includes

namespace CMIO { namespace CMA
{
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// BlockBuffer
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	class BlockBuffer
	{
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// BlockBuffer
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// Construction/Destruction
	public:
										BlockBuffer(CMBlockBufferRef ref = NULL, bool retain = false, bool release = true) : mRef(NULL)  { Reset(ref, retain, release); }
										~BlockBuffer() { Reset(); }
		void							Reset(CMBlockBufferRef ref = NULL, bool retain = false, bool release = true) { if (NULL != mRef and mRelease) CFRelease(mRef); if (retain and NULL != ref) CFRetain(ref); mRef = ref; mRelease = release; }
		BlockBuffer&					operator=(const BlockBuffer& that) { if (this != &that) this->Reset(that.Get(), true, that.WillRelease());  return *this; }

	// Excecption throwing versions of the CMBlockBufferCreateXXXXX routines for the underlying CMBlockBufferRef
	public:
		static CMBlockBufferRef		CreateEmpty(CFAllocatorRef structureAllocator, uint32_t subBlockCapacity, CMBlockBufferFlags flags)
										{
											CMBlockBufferRef blockBufferRef;
											OSStatus err = CMBlockBufferCreateEmpty(structureAllocator, subBlockCapacity, flags, &blockBufferRef);
											ThrowIfError(err, CAException(err), "CMIO::BlockBuffer::CreateEmpty: CMBlockBufferCreateEmpty() failed");
											return blockBufferRef; 
										}

		static CMBlockBufferRef		CreateWithMemoryBlock(CFAllocatorRef structureAllocator, void *memoryBlock, size_t blockLength, CFAllocatorRef blockAllocator,
												CMBlockBufferCustomBlockSource *customBlockSource, size_t offsetToData, size_t dataLength, CMBlockBufferFlags flags)
										{
											CMBlockBufferRef blockBufferRef;
											OSStatus err = CMBlockBufferCreateWithMemoryBlock(structureAllocator, memoryBlock, blockLength, blockAllocator, customBlockSource, offsetToData, dataLength, flags, &blockBufferRef);
											ThrowIfError(err, CAException(err), "CMIO::BlockBuffer::CreateWithMemoryBlock: CMBlockBufferCreateWithMemoryBlock() failed");
											return blockBufferRef; 
										}

		static CMBlockBufferRef		CreateWithBufferReference(CFAllocatorRef structureAllocator, CMBlockBufferRef targetBuffer, size_t offsetToData, size_t dataLength, CMBlockBufferFlags flags)
										{
											CMBlockBufferRef blockBufferRef;
											OSStatus err = CMBlockBufferCreateWithBufferReference(structureAllocator, targetBuffer, offsetToData, dataLength, flags,  &blockBufferRef);
											ThrowIfError(err, CAException(err), "CMIO::BlockBuffer::CreateWithBufferReference: CMBlockBufferCreateWithBufferReference() failed");
											return blockBufferRef; 
										}

	private:
		CMBlockBufferRef				mRef;
		bool							mRelease;

	// Attributes
	public:
		bool							IsValid() const { return mRef != NULL; }
		bool							WillRelease() const { return mRelease; }
		void							ShouldRelease(bool release) { mRelease = release; }
	
	// Operations
	public:
		UInt8*							GetDataPointer(size_t offset, size_t* lengthAtOffset, size_t* totalLength)
										{
											char* dataPointer = NULL;
											OSStatus err = CMBlockBufferGetDataPointer(mRef, offset, lengthAtOffset, totalLength, &dataPointer);
											ThrowIfError(err, CAException(err), "CMIO::BlockBuffer::GetDataPointer: CMBlockBufferGetDataPointer() failed");
											return (UInt8*)dataPointer;
										}
										
		size_t							GetDataLength() { return CMBlockBufferGetDataLength(mRef); }
		bool							IsRangeContiguous(size_t offset, size_t length) { return CMBlockBufferIsRangeContiguous(mRef, offset, length); }
		
	// Value Access
	public:
		operator						CMBlockBufferRef() const { return mRef; }
		CMBlockBufferRef				Get() const { return mRef; }		
	};
}}
#endif
