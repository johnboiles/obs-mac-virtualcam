/*
	    File: CMIO_Buffer.h
	Abstract: A subclass of the CMIO::CMA::SampleBuffer C++ wrapper for CMIO's variant CMSampleBufferRef.
			As described in  <CoreMediaIO/CMIOSampleBuffer.h>, CMIO variant differs soley by having a defined set of "required" attachments.
	
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

#if !defined(__CMIO_Buffer_h__)
#define __CMIO_Buffer_h__

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Includes
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

// Super Class Include
#include "CMIO_CMA_BlockBuffer.h"
#include "CMIO_CMA_SampleBuffer.h"
#include "CMIO_CVA_Image_Buffer.h"

// System Includes
#include <CoreMediaIO/CMIOSampleBuffer.h>

namespace CMIO
{
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// Buffer
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	class Buffer : public CMA::SampleBuffer
	{
	// Construction/Destruction
	public:
									Buffer(CMSampleBufferRef ref = NULL, bool retain = false, bool release = true) : CMA::SampleBuffer(ref, retain, release) {}
		virtual						~Buffer() {}
		Buffer&						operator=(const Buffer& that) { if (this != &that) this->Reset(that.Get(), true, that.WillRelease());  return *this; }

	// Exception throwing versions of the CMIOSampleBufferCreateXXXXX routines for the underlying CMSampleBufferRefs
	public:
		static CMSampleBufferRef	Create(CFAllocatorRef allocator, CMBlockBufferRef dataBuffer, CMFormatDescriptionRef format, UInt32 numberOfSamples, UInt32 numberOfSampleTimingEntries,
											const CMSampleTimingInfo *sampleTimingArray, UInt32 numberOfSampleSizeEntries, const size_t *sampleSizeArray, UInt64 sequenceNumber,
											UInt32 discontinuityFlags)
									{
										CMSampleBufferRef sampleBufferRef;
										OSStatus err = CMIOSampleBufferCreate(allocator, dataBuffer, format, numberOfSamples, numberOfSampleTimingEntries, sampleTimingArray,
												numberOfSampleSizeEntries,  sampleSizeArray, sequenceNumber, discontinuityFlags, &sampleBufferRef);
										ThrowIfError(err, CAException(err), "CMIO::Buffer::Create: CMIOSampleBufferCreate() failed");
										return sampleBufferRef; 
									}

		static CMSampleBufferRef	CreateForImageBuffer(CFAllocatorRef allocator, CVImageBufferRef cvImageBuffer, CMFormatDescriptionRef format, const CMSampleTimingInfo *sampleTiming,
										   UInt64 sequenceNumber, UInt32 discontinuityFlags)
									{
										CMSampleBufferRef sampleBufferRef;
										OSStatus err = CMIOSampleBufferCreateForImageBuffer(allocator, cvImageBuffer, format, sampleTiming, sequenceNumber, discontinuityFlags, &sampleBufferRef);
										ThrowIfError(err, CAException(err), "CMIO::Buffer::Create: CMIOSampleBufferCreateForImageBuffer() failed");
										return sampleBufferRef; 
									}

		static CMSampleBufferRef	CreateNoDataMarker(CFAllocatorRef allocator, UInt32 noDataEvent, CMFormatDescriptionRef format, UInt64 sequenceNumber, UInt32 discontinuityFlags)
									{
										CMSampleBufferRef sampleBufferRef;
										OSStatus err = CMIOSampleBufferCreateNoDataMarker(allocator, noDataEvent, format, sequenceNumber, discontinuityFlags, &sampleBufferRef);
										ThrowIfError(err, CAException(err), "CMIO::Buffer::CreateNoDataMarker: CMIOSampleBufferCreateNoDataMarker() failed");
										return sampleBufferRef; 
									}
									
		static CMSampleBufferRef	Clone(CFAllocatorRef allocator, Buffer& source, UInt64 sequenceNumber, const CMSampleTimingInfo& timingInfo)
									{
										// Get the underlying BlockBuffer or CVImageBuffer
										CMA::BlockBuffer blockBuffer(source.GetDataBuffer(), false, false);
										CVA::Image::Buffer imageBuffer(source.GetImageBuffer(), false, false);
										ThrowIf(((not blockBuffer.IsValid()) && (not imageBuffer.IsValid())), CAException(kCMSampleBufferError_AllocationFailed), "CMIO::Buffer::Clone: source BlockBuffer is not valid");
										
										// Select the source's or the provided timing info (based on whether or not the latter's duration is valid)
										CMSampleTimingInfo* cloneTimingInfo = &const_cast<CMSampleTimingInfo&>(timingInfo);
										if (CMTIME_IS_INVALID(timingInfo.duration))
										{
											// Get the timing info in the source since no valid timing info was provided
											CMSampleTimingInfo	sourceTimingInfo;
											CMItemCount timingArrayEntriesNeeded;
											source.GetSampleTimingInfoArray(1, &sourceTimingInfo, &timingArrayEntriesNeeded);
											ThrowIf(1 != timingArrayEntriesNeeded , CAException(kCMSampleBufferError_BufferHasNoSampleTimingInfo), "CMIO::Buffer::Clone: 1 != timingArrayEntriesNeeded");
											cloneTimingInfo = &sourceTimingInfo;
										}
										
										if (blockBuffer.IsValid())
										{
											// Determine the sample size
											size_t sampleSize;
											CMItemCount sizeArrayEntriesNeeded;
											source.GetSampleSizeArray(1, &sampleSize, &sizeArrayEntriesNeeded);
											ThrowIf(1 != sizeArrayEntriesNeeded , CAException(kCMSampleBufferError_BufferHasNoSampleSizes), "CMIO::Buffer::Clone: 1 != sizeArrayEntriesNeeded");
											
											// Create the clone
											Buffer clone(Create(allocator, blockBuffer, source.GetFormatDescription(), ToUInt32(CMSampleBufferGetNumSamples(source)), 1, cloneTimingInfo, 1, &sampleSize,
													sequenceNumber, source.GetDiscontinuityFlags()), false, false);
											
											// Copy over the attachments
											OSStatus err = CMIOSampleBufferCopyNonRequiredAttachments(source, clone, kCMAttachmentMode_ShouldPropagate);
											ThrowIfError(err, CAException(err), "CMIO::Buffer::Clone: CMIOSampleBufferCopyNonRequiredAttachments() failed for propogating attachments");
											
											err = CMIOSampleBufferCopyNonRequiredAttachments(source, clone, kCMAttachmentMode_ShouldNotPropagate);
											ThrowIfError(err, CAException(err), "CMIO::Buffer::Clone: CMIOSampleBufferCopyNonRequiredAttachments() failed for non-propogating attachments");
											
											err = CMIOSampleBufferCopySampleAttachments(source, clone);
											ThrowIfError(err, CAException(err), "CMIO::Buffer::Clone: CMIOSampleBufferCopySampleAttachments() failed");
											
											// Return the clone
											return clone;
										}
										else
										{
											// Create the clone
											Buffer clone(CreateForImageBuffer(allocator, imageBuffer, source.GetFormatDescription(), cloneTimingInfo, sequenceNumber, source.GetDiscontinuityFlags()), false, false);
											
											// Copy over the attachments
											OSStatus err = CMIOSampleBufferCopyNonRequiredAttachments(source, clone, kCMAttachmentMode_ShouldPropagate);
											ThrowIfError(err, CAException(err), "CMIO::Buffer::Clone: CMIOSampleBufferCopyNonRequiredAttachments() failed for propogating attachments");
											
											err = CMIOSampleBufferCopyNonRequiredAttachments(source, clone, kCMAttachmentMode_ShouldNotPropagate);
											ThrowIfError(err, CAException(err), "CMIO::Buffer::Clone: CMIOSampleBufferCopyNonRequiredAttachments() failed for non-propogating attachments");
											
											err = CMIOSampleBufferCopySampleAttachments(source, clone);
											ThrowIfError(err, CAException(err), "CMIO::Buffer::Clone: CMIOSampleBufferCopySampleAttachments() failed");
											
											// Return the clone
											return clone;
										}
									}

	// Attributes
	public:
		UInt64						GetSequenceNumber() { return CMIOSampleBufferGetSequenceNumber(mRef); }
		void						SetDiscontinuityFlags(CFAllocatorRef allocator, UInt32 discontinuityFlags) { CMIOSampleBufferSetDiscontinuityFlags(allocator, mRef, discontinuityFlags); }
		UInt32						GetDiscontinuityFlags() { return CMIOSampleBufferGetDiscontinuityFlags(mRef); }
	};
}
#endif
