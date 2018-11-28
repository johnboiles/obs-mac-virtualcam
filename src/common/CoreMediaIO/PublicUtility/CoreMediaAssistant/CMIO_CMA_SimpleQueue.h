/*
	    File: CMIO_CMA_SimpleQueue.h
	Abstract: C++ wrapper for CMSimpleQueueRef.
				
				------ WARNING -------
				As written, T MUST be a pointer type.
	
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

#if !defined(__CMIO_CMA_SimpleQueue_h__)
#define __CMIO_CMA_SimpleQueue_h__

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Includes
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

// Public Utility Includes
#include "CMIODebugMacros.h"

// CA Public Utility Includes
#include "CAException.h"

// System Includes
#include <CoreMedia/CMSimpleQueue.h>

namespace CMIO { namespace CMA
{
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// SimpleQueue
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	template <typename T> class SimpleQueue
	{
	// Construction/Destruction
	public:
									SimpleQueue(CMSimpleQueueRef ref = NULL, bool retain = false, bool release = true) : mRef(NULL) { Reset(ref, retain, release); }
									~SimpleQueue() { Reset(); }
		void						Reset(CMSimpleQueueRef ref = NULL, bool retain = false, bool release = true) { if (NULL != mRef and mRelease) CFRelease(mRef); if (retain and NULL != ref) CFRetain(ref); mRef = ref; mRelease = release; }
		SimpleQueue&				operator=(const SimpleQueue& that) { if (this != &that) this->Reset(that.Get(), true, that.WillRelease()); return *this; }

	// Excecption throwing versions of the CMSimpleQueueCreateXXXXX routines for the underlying CMSimpleQueueRefs
	public:
		static CMSimpleQueueRef	Create(CFAllocatorRef allocator, UInt32 size) { CMSimpleQueueRef queueRef; OSStatus err = CMSimpleQueueCreate(allocator, size, &queueRef); ThrowIfError(err, CAException(err), "CMIO::SimpleQueue::Create: CMSimpleQueueCreate() failed"); return queueRef; }

	private:
		CMSimpleQueueRef			mRef;
		bool						mRelease;

	// Attributes
	public:
		bool						IsValid() const { return mRef != NULL; }
		bool						WillRelease() const { return mRelease; }
		void						ShouldRelease(bool release) { mRelease = release; }
		
	// Queue Operations
	public:
		void						Enqueue(const T value) { OSStatus err = CMSimpleQueueEnqueue(mRef, value); ThrowIfError(err, CAException(err), "CMIO::SimpleQueue::Enqueue: CMSimpleQueueEnqueue() failed"); }
		T							Dequeue() { T value = static_cast<T>(const_cast<void*>(CMSimpleQueueDequeue(mRef))); ThrowIfNULL(value, CAException(kCMSimpleQueueError_ParameterOutOfRange), "CMIO::SimpleQueue::Dequeue: CMSimpleQueueDequeue() returned NULL"); return value;}
		T							GetHead() { T value = static_cast<T>(const_cast<void*>(CMSimpleQueueGetHead(mRef))); ThrowIfNULL(value, CAException(kCMSimpleQueueError_ParameterOutOfRange), "CMIO::SimpleQueue::GetHead: CMSimpleQueueGetHead() returned NULL"); return value;}
		UInt32						Capacity() const { return CMSimpleQueueGetCapacity(mRef); }
		UInt32						GetCount() const { return CMSimpleQueueGetCount(mRef); }
		Float32						Fullness() const { return CMSimpleQueueGetFullness(mRef); }
		void						ResetToEmpty() { OSStatus err = CMSimpleQueueReset(mRef); ThrowIfError(err, CAException(err), "CMIO::SimpleQueue::ResetToEmpty: CMSimpleQueueGetCount() failed"); }

	// Value Access
	public:
		operator					CMSimpleQueueRef() const { return mRef; }
		CMSimpleQueueRef			Get() const { return mRef; }
	};
}}
#endif
