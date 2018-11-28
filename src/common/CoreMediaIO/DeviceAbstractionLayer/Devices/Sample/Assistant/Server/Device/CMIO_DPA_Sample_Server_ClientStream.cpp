/*
	    File: CMIO_DPA_Sample_Server_ClientStream.cpp
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
//	Includes
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

// Self Include
#include "CMIO_DPA_Sample_Server_ClientStream.h"

// Internal Includes
#include "CMIO_DPA_Sample_Server_Frame.h"

// Public Utility Includes
#include "CMIODebugMacros.h"

// CA Public Utility Includes
#include "CAException.h"
#include "CAPThread.h"

// System Includes
#include <mach/mach.h>

#pragma mark -
namespace CMIO { namespace DPA { namespace Sample { namespace Server
{
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// ClientStream()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	ClientStream::ClientStream(Client client, mach_port_t messagePort, CAGuard& frameAvailableGuard) :
		mQueue(CMA::SimpleQueue<Frame*>::Create(NULL, 30)),
		mDiscontinuityFlags(kCMIOSampleBufferNoDiscontinuities),
		mExtendedFrameHostTime(0),
		mExtendedFrameTimingInfo(),
		mClient(client),
		mMessagePort(messagePort),
		mStopMessageLoop(false),
		mMessageThread(reinterpret_cast<CAPThread::ThreadRoutine>(MessageThreadEntry), this),
		mMessageThreadGuard("ClientStream Message Thread Guard"),
		mFrameAvailableGuard(frameAvailableGuard)
	{
		StartMessageThread();
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// ~ClientStream()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	ClientStream::~ClientStream()
	{
		// Stop the message thread if necessary
		StopMessageThread();

		// Release the Mach port to which frames were sent
		(void) mach_port_deallocate(mach_task_self(), mMessagePort);
		
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// StartMessageThread()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void ClientStream::StartMessageThread()
	{
		// Grab the message thread guard
		CAGuard::Locker messageThreadGuard(mMessageThreadGuard);
		
		// Spawn the message thread
		mMessageThread.Start();
		
		// Wait for the message thread to indicate it has started
		messageThreadGuard.Wait();
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// StopMessageThread()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void ClientStream::StopMessageThread()
	{
		// Grab the message thread guard
		CAGuard::Locker messageThreadGuard(mMessageThreadGuard);

		// Lock the mFrameAvailableGuard while mStopMessageLoop is being set to true
		(void) mFrameAvailableGuard.Lock();

		// Indicate that the message thread should stop
		mStopMessageLoop = true;
		
		// Unlock the mFrameAvailableGuard since mStopMessageLoop has been set to true
		mFrameAvailableGuard.Unlock();

		// Notify the message thread to wake up if it had been waiting for frames to be available
		mFrameAvailableGuard.NotifyAll();
		
		// Wait for the message thread to terminate
		messageThreadGuard.Wait();
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// MessageThreadEntry()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void* ClientStream::MessageThreadEntry(ClientStream& clientStream)
	{
		{
			// Grab the message thread guard
			CAGuard::Locker	messageThreadGuard(clientStream.mMessageThreadGuard);
				
			// Signal that the message thread is running
			messageThreadGuard.NotifyAll();
		}
		
		while (not clientStream.mStopMessageLoop)
		{
			// Message any frames in the queue to the client
			if (0 != clientStream.mQueue.GetCount())
			{
				// Message the frame to the client
				clientStream.SendFrameArrivedMessage(clientStream.mMessagePort, *clientStream.mQueue.GetHead());
				
				// Pop it off the queue
				(void) clientStream.mQueue.Dequeue();
			}
			else
			{
				// Grab the mFrameAvailableGuard
				CAGuard::Locker locker(clientStream.mFrameAvailableGuard);
				
				// Wait if there are no frames
				if (0 == clientStream.mQueue.GetCount() and not clientStream.mStopMessageLoop)
					locker.Wait();
			}
		}
		
		// Signal that the message thread has terminated
		CAGuard::Locker locker(clientStream.mMessageThreadGuard);
		clientStream.mMessageThreadGuard.NotifyAll();
		
		return NULL;
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// SendFrameArrivedMessage()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void ClientStream::SendFrameArrivedMessage(mach_port_t& recipient, Frame& frame)
	{
		// Setup the message
		FrameArrivedMessage message =
		{
			{
				MACH_MSGH_BITS_REMOTE(MACH_MSG_TYPE_COPY_SEND) | MACH_MSGH_BITS_COMPLEX,	// mHeader.msgh_bits
				sizeof(FrameArrivedMessage),												// mHeader.msgh_size
				recipient,																	// mHeader.msgh_remote_port
				MACH_PORT_NULL,																// mHeader.msgh_local_port
				0,																			// mHeader.msgh_reserved
				kFrameArrived																// mHeader.msgh_id
			},
			{
				1																			// mBody.msgh_descriptor_count
			},
			{
				// The mach_msg_ool_descriptor_t structure is layed out differently for 32 vs 64 bit architectures, so it is not intialiazed here
			},
			frame.GetFrameType(),															// mFrameType
			frame.GetHostTime(),															// mHostTime
			frame.GetTimingInfo(),															// mTimingInfo
			frame.GetDiscontinuityFlags() | GetDiscontinuityFlags(),						// mDiscontinuityFlags
			frame.GetDroppedFrameCount(),													// mDroppedFrameCount
			frame.GetFirstFrameTime()														// mFirstFrameTime
		};

		// Initialize the mach_msg_ool_descriptor_t portion of the message
		message.mDescriptor.address		= frame.Get();
		message.mDescriptor.size		= frame.Size();
		message.mDescriptor.deallocate	= false;
		message.mDescriptor.copy		= MACH_MSG_VIRTUAL_COPY;
		message.mDescriptor.type		= MACH_MSG_OOL_DESCRIPTOR;
		
		// If this frame or the ClientStream has any "hard" discontinuities in it, "extended duration" timing can't be used
		if (~kCMIOSampleBufferDiscontinuityFlag_DurationWasExtended & (frame.GetDiscontinuityFlags() | GetDiscontinuityFlags()))
			mExtendedFrameHostTime = 0;

		// Use the "extended duration" timing if needed (indicated by a non-zero value for mExtendedFrameHostTime)
		if (0 != mExtendedFrameHostTime)
		{
			// Use the extended frame host & cycle time
			message.mHostTime = mExtendedFrameHostTime;

			// Use the extended frame timing info & add the current frame's duration to the extended frame's duration
			message.mTimingInfo = mExtendedFrameTimingInfo;
			message.mTimingInfo.duration = CMTimeAdd(message.mTimingInfo.duration, frame.GetTimingInfo().duration);
	
			// Indicate this frame has an extended duration
			message.mDiscontinuityFlags |= kCMIOSampleBufferDiscontinuityFlag_DurationWasExtended;
		}

		// Send the message
		mach_msg_return_t err = mach_msg(&message.mHeader, MACH_SEND_MSG | MACH_SEND_TIMEOUT, message.mHeader.msgh_size, 0, MACH_PORT_NULL, 15, MACH_PORT_NULL);
		if (MACH_MSG_SUCCESS == err)
		{
			// The message was sent successfully, so clear the discontinuity flags
			SetDiscontinuityFlags(kCMIOSampleBufferNoDiscontinuities);
			
			// Reset the extended frame host time to 0 to signify that no extension is needed
			mExtendedFrameHostTime = 0;
		}
		else
		{
			DebugMessage("SendFrameArrivedMessage() - Error sending frame to port %d 0x%08X", recipient, err);

			// Something went wrong, so destroy the message so that all of its resources will be properly released
			mach_msg_destroy(&message.mHeader);
			
			// Update the recipient since it might have a new value since the send failed
			recipient = message.mHeader.msgh_remote_port;
			
			// Try and use "extended duration" timing if this frame and the ClientStream lacks any "hard" discontinuities
			if (~kCMIOSampleBufferDiscontinuityFlag_DurationWasExtended & (frame.GetDiscontinuityFlags() | GetDiscontinuityFlags()))
			{
				// The were hard discontinuities, so update the ClientStream's discontinuity flags so they logical sum can be passed on with the next frame
				SetDiscontinuityFlags(GetDiscontinuityFlags() | frame.GetDiscontinuityFlags() | kCMIOSampleBufferDiscontinuityFlag_StreamDiscontinuity);
			}
			else
			{
				// Rather than mark a "hard" discontinuity, remember the frame's duration and so the NEXT frame's can be extended accordingly
				if (0 == mExtendedFrameHostTime)
				{
					// This is the first extension needed, so use this frame's host & cycle time for the next frame
					mExtendedFrameHostTime = frame.GetHostTime();
					mExtendedFrameTimingInfo = frame.GetTimingInfo();
				}
				else
				{
					// An extension is already taking place, so simply add this frame's duration to the extended frame's duration
					mExtendedFrameTimingInfo.duration = CMTimeAdd(mExtendedFrameTimingInfo.duration, frame.GetTimingInfo().duration);
				}
			}
		}
		
		// Remove this client from the set of the native frame needs to message
		frame.RemoveClient(mClient);
	}
}}}}
