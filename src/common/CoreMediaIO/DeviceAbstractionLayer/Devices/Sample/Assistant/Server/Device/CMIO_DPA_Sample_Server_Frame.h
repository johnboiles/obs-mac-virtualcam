/*
	    File: CMIO_DPA_Sample_Server_Frame.h
	Abstract: n/a
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

#if !defined(__CMIO_DPA_Sample_Server_Frame_h__)
#define __CMIO_DPA_Sample_Server_Frame_h__

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Includes
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

// Internal Includes
#include "CMIO_DPA_Sample_Shared.h"
#include "CMIO_DPA_Sample_Server_Common.h"

// Public Utility Includes
#include "CMIO_CMA_SampleBuffer.h"

// CA Public Utility Includes
#include "CAMutex.h"

// System Includes
#include <IOKit/stream/IOStreamLib.h>
#include <CoreMediaIO/CMIOSampleBuffer.h>

namespace CMIO { namespace DPA { namespace Sample { namespace Server
{
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// Frame
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	class Frame
	{		
	public:
	// Construction/Destruction
										Frame(IOStreamRef stream, FrameType frameType, UInt64 hostTime, const CMA::SampleBuffer::TimingInfo& timingInfo, UInt32 discontinuityFlags, UInt32 droppedFrameCount, UInt64 firstFrameTime, IOStreamBufferID bufferID, size_t size, void* data);
		virtual							~Frame();										
		
	private:
		IOStreamRef						mStream;
		Frame&							operator=(Frame& that);							// Unimplemented - don't allow copying

	// Attributes
	public:
		FrameType						GetFrameType() const { return mFrameType; }

	protected:
		FrameType						mFrameType;

	// Time Stamps
	public:
		void							SetHostTime(UInt64 hostTime) { mHostTime = hostTime; }
		UInt64							GetHostTime() const { return mHostTime; }
		void							SetTimingInfo(const CMA::SampleBuffer::TimingInfo& timingInfo)  { mTimingInfo = timingInfo; }
		CMA::SampleBuffer::TimingInfo	GetTimingInfo() const { return mTimingInfo; }
	
	protected:
		UInt64							mHostTime;				// Host time for frame
		CMA::SampleBuffer::TimingInfo	mTimingInfo;			// Samplebuffer timing info for frame
	
	// Discontinuity Flags
	public:
		void							SetDiscontinuityFlags(UInt32 discontinuityFlags) { mDiscontinuityFlags = discontinuityFlags; }
		UInt32							GetDiscontinuityFlags() const { return mDiscontinuityFlags; }
	
	protected:
		UInt32							mDiscontinuityFlags;	// Discontinuity flags associated with the frame
	
	// Dropped frame count
	public:
		void							SetDroppedFrameCount(UInt32 droppedFrameCount) { mDroppedFrameCount = droppedFrameCount; }
		UInt32							GetDroppedFrameCount() const { return mDroppedFrameCount; }
	
	protected:
		UInt32							mDroppedFrameCount;		// Dropped frame count associated with the frame
	
	// First frame time
	public:
		void							SetFirstFrameTime(UInt32 firstFrameTime) { mFirstFrameTime = firstFrameTime; }
		UInt64							GetFirstFrameTime() const { return mFirstFrameTime; }
	
	protected:
		UInt64							mFirstFrameTime;		// Hosttime associated with first frame from device

	// Accessors
	public:
		UInt32							Size() { return mSize; }
		void*							Get() { return mFrameData; }

	protected:
		IOStreamBufferID				mBufferID;
		UInt32							mSize;
		void*							mFrameData;

	// Clients
	public:
		void							AddClient(Client client);
		void							RemoveClient(Client client);

	protected:
		Clients							mClients;				// Clients to send the message frame to
		CAMutex							mClientsMutex;			// Protect state of the set of clients
	};
}}}}

#endif
