/*
	    File: CMIO_DPA_Sample_Server_Frame.cpp
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

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Includes
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

// Self Include
#include "CMIO_DPA_Sample_Server_Frame.h"


namespace CMIO { namespace DPA { namespace Sample { namespace Server
{
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// Frame
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	Frame::Frame(IOStreamRef stream, FrameType frameType, UInt64 hostTime, const CMA::SampleBuffer::TimingInfo& timingInfo, UInt32 discontinuityFlags, UInt32 droppedFrameCount, UInt64 firstFrameTime, IOStreamBufferID bufferID, size_t size, void* data) :
		mStream(stream),
		mFrameType(frameType),
		mHostTime(hostTime),
		mTimingInfo(timingInfo),
		mDiscontinuityFlags(discontinuityFlags),
		mDroppedFrameCount(droppedFrameCount),
		mFirstFrameTime(firstFrameTime),
		mBufferID(bufferID),
		mSize(size),
		mFrameData(data),
		mClients(),
		mClientsMutex("frame clients mutex")
	{
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// ~Frame()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	Frame::~Frame()
	{
		// Return the frame to the queue
		(**mStream).EnqueueInputBuffer(mStream, mBufferID, 0, 0, 0, 0);
		(**mStream).SendInputNotification(mStream, 0xAA);
	}
	
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// AddClient()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void Frame::AddClient(Client client)
	{
		// Grab the mutex for the the set of clients
		CAMutex::Locker locker(mClientsMutex);

		// Add the client to the set of that will be messaged with the frame
		DebugMessageLevel(4, "Frame::AddClient: frame (%x) - Adding client - %d with count: %d", this, client, mClients.size() + 1);
		mClients.insert(client);
	}
	
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// RemoveClient()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void Frame::RemoveClient(Client client)
	{
		{
			// Grab the mutex for the the set of clients
			CAMutex::Locker locker(mClientsMutex);

			// Remove the client from the to-be-messaged set
			mClients.erase(client);
			DebugMessageLevel(4, "Frame::RemoveClient: Native frame (%x) - Removing client - %d with count: %d", this, client, mClients.size());
			
			// If there are still clients, simply return
			if (not mClients.empty())
				return;
		}
			
		// Clear the discontinuity flags
		SetDiscontinuityFlags(kCMIOSampleBufferNoDiscontinuities);

		// This frame is no longer needed so delete it
		delete this;
	}
}}}}
