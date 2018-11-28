/*
	    File: CMIO_CMA_SampleBuffer.h
	Abstract: C++ wrapper for CMSampleBufferRef
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

#if !defined(__CMIO_CMA_SampleBuffer_h__)
#define __CMIO_CMA_SampleBuffer_h__

// Public Utility
#include "CMIODebugMacros.h"

// CA Public Utility
#include "CAException.h"

// System Includes
#include <CoreMedia/CMSampleBuffer.h>

namespace CMIO { namespace CMA
{
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// SampleBuffer
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	class SampleBuffer
	{
	public:
		//-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
		// SampleBuffer::TimingInfo
		//-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
		class TimingInfo : public CMSampleTimingInfo
		{
		// Construction/Destruction
		public:
				TimingInfo() { *this = kCMTimingInfoInvalid; }
				TimingInfo(const CMSampleTimingInfo& cmSampleTimingInfo) { *this = cmSampleTimingInfo; }
				TimingInfo(const CMTime& inDuration, const CMTime& inPresentationTimeStamp, const CMTime& inDecodeTimeStamp) { this->duration = inDuration; this->presentationTimeStamp = inPresentationTimeStamp; this->decodeTimeStamp = inDecodeTimeStamp; }
				
		// Assignment
		public:
			TimingInfo&	operator=(const CMSampleTimingInfo& that) { memcpy(this, &that, sizeof(CMSampleTimingInfo)); return *this; }

		};
		
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// SampleBuffer
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// Construction/Destruction
	public:
										SampleBuffer(CMSampleBufferRef ref = NULL, bool retain = false, bool release = true) : mRef(NULL)  { Reset(ref, retain, release); }
		virtual							~SampleBuffer() { Reset(); }
		void							Reset(CMSampleBufferRef ref = NULL, bool retain = false, bool release = true) { if (NULL != mRef and mRelease) CFRelease(mRef); if (retain and NULL != ref) CFRetain(ref); mRef = ref; mRelease = release; }
		SampleBuffer&					operator=(const SampleBuffer& that) { if (this != &that) this->Reset(that.Get(), true, that.WillRelease());  return *this; }

	// Exception throwing versions of the CMSampleBufferCreate() routines for the underlying CMSampleBufferRef
	public:
		static CMSampleBufferRef		Create(CFAllocatorRef allocator, CMBlockBufferRef dataBuffer, Boolean dataReady, CMSampleBufferMakeDataReadyCallback makeDataReadyCallback,	
												void *makeDataReadyRefcon, CMFormatDescriptionRef formatDescription, CMItemCount numSamples, CMItemCount numSampleTimingEntries,
												const CMSampleTimingInfo *sampleTimingArray, CMItemCount numSampleSizeEntries, const size_t *sampleSizeArray)
											{
												CMSampleBufferRef sampleBufferRef;
												OSStatus err = CMSampleBufferCreate(allocator, dataBuffer, dataReady, makeDataReadyCallback, makeDataReadyRefcon, formatDescription,
														numSamples, numSampleTimingEntries, sampleTimingArray, numSampleSizeEntries, sampleSizeArray, &sampleBufferRef);
												
												ThrowIfError(err, CAException(err), "CMIO::CMA::SampleBuffer::Create: CMSampleBufferCreate() failed");
												return sampleBufferRef; 
											}

	protected:
		CMSampleBufferRef				mRef;
		bool							mRelease;

	// Attributes
	public:
		bool							IsValid() const { return mRef != NULL; }
		bool							WillRelease() const { return mRelease; }
		void							ShouldRelease(bool release) { mRelease = release; }

	// Timing information
	public:
		CMSampleTimingInfo				GetTimingInfo() { CMSampleTimingInfo timingInfo; timingInfo.decodeTimeStamp = CMSampleBufferGetDecodeTimeStamp(mRef); timingInfo.presentationTimeStamp = CMSampleBufferGetPresentationTimeStamp(mRef); timingInfo.duration = CMSampleBufferGetDuration(mRef); return timingInfo; }
		void							GetSampleTimingInfoArray(CMItemCount timingArrayEntries, CMSampleTimingInfo *timingArrayOut, CMItemCount *timingArrayEntriesNeededOut) { OSStatus err = CMSampleBufferGetSampleTimingInfoArray(mRef, timingArrayEntries, timingArrayOut, timingArrayEntriesNeededOut); ThrowIfError(err, CAException(err), "CMIO::CMA::SampleBuffer::GetSampleTimingInfoArray: CMSampleBufferGetSampleTimingInfoArray() failed"); }
		bool							DataIsReady() { return CMSampleBufferDataIsReady(mRef); }
		CMBlockBufferRef				GetDataBuffer() { return CMSampleBufferGetDataBuffer(mRef); }		
		CVImageBufferRef				GetImageBuffer() { return CMSampleBufferGetImageBuffer(mRef); }		
		void							GetSampleSizeArray(CMItemCount sizeArrayEntries, size_t* sizeArrayOut, CMItemCount* sizeArrayEntriesNeededOut) { OSStatus err = CMSampleBufferGetSampleSizeArray(mRef, sizeArrayEntries, sizeArrayOut, sizeArrayEntriesNeededOut); ThrowIfError(err, CAException(err), "CMIO::CMA::SampleBuffer::GetSampleSizeArray: CMSampleBufferGetSampleSizeArray() failed"); }
		size_t							GetSampleSize(CMItemIndex sampleIndex) { return CMSampleBufferGetSampleSize(mRef, sampleIndex); }
		
	// Value Access
	public:
		operator						CMSampleBufferRef() const { return mRef; }
		CMSampleBufferRef				Get() const { return mRef; }
		CMFormatDescriptionRef			GetFormatDescription() const { CMFormatDescriptionRef formatDescriptionRef = CMSampleBufferGetFormatDescription(mRef); ThrowIfNULL(formatDescriptionRef, CAException(-1), "CMIO::CMA::SampleBuffer::GetFormatDescriptionRef: can't get format description"); return formatDescriptionRef; }
	};
}}
#endif
