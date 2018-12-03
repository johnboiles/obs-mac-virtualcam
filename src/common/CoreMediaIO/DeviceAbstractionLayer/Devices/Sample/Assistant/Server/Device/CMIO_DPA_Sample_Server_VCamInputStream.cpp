/*
	    File: CMIO_DPA_Sample_Server_VCamInputStream.cpp
	Abstract: n/a
	 Version: 1.2
	
*/

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//	Includes
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

// Self Include
#include "CMIO_DPA_Sample_Server_VCamInputStream.h"

// Internal Includes
#include "CMIO_DPA_Sample_Server_ClientStream.h"
#include "CMIO_DPA_Sample_Server_Device.h"
#include "CMIO_DPA_Sample_Server_Frame.h"
#include "CMIO_DPA_Sample_Server_Deck.h"
#include "CMIO_DPA_Sample_Shared.h"

// Public Utility Includes
#include "CMIODebugMacros.h"

// CA Public Utility Includes
#include "CACFNumber.h"
#include "CAHostTimeBase.h"

// System Includes
#include <IOKit/video/IOVideoTypes.h>
#include <CoreMediaIO/CMIOHardware.h>
#include "CMIO_CVA_Pixel_Buffer.h"
#include "CMIO_SA_Assistance.h"
#include <mach/mach.h>

#define kMaxRequestsPerCallback 4

namespace CMIO { namespace DPA { namespace Sample { namespace Server
{
	#pragma mark -
	#pragma mark VCamInputStream
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// VCamInputStream()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	VCamInputStream::VCamInputStream(Device* device, CFDictionaryRef streamDictionary, CMIOObjectPropertyScope scope) :
        Stream(device, streamDictionary, scope)
	{
	}

#pragma mark -
    //-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    // Start()
    //-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    void VCamInputStream::SetStreamFormat(IOVideoStreamDescription *newStreamFormat) {
        // ??
    }

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// Start()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void VCamInputStream::Start(Client client, mach_port_t messagePort, UInt32 initialDiscontinuityFlags)
	{
		try
		{
			// Grab the mutex for the device's state
			CAMutex::Locker locker(mStateMutex);
			
			// Add the  Client to the set of clients to which are listening to the stream
			if (MACH_PORT_NULL != client)
			{
				// Don't do anything if the client is already in the map of ClientStreams
				if (mClientStreams.end() != mClientStreams.find(client))
					return;
					
				// Create the ClientStream object to help manage this client's connection to the stream
				ClientStream* clientStream = new ClientStream(client, messagePort, mFrameAvailableGuard);
				
				// Add it to the map of clients using the stream
				CAMutex::Locker clientStreamsLocker(mClientStreamsMutex);
				mClientStreams[client] = clientStream;
			}
			
			// If the stream is currently active nothing else needs to be done so simply return
			if (mStreaming)
				return;

			SetDiscontinuityFlags(initialDiscontinuityFlags);			// Set the initial discontinuity flags

			// The stream is active now
			mStreaming = true;
		}
		catch (...)
		{
			// Something went wrong, so clean up
			ClientStreamMap::iterator i = mClientStreams.find(client);
			if (i != mClientStreams.end())
			{
				// The ClientStream had been succesfully created, so delete it and erase it from the map
				CAMutex::Locker clientStreamsLocker(mClientStreamsMutex);
				delete (*i).second;
				mClientStreams.erase(i);
			}
			else if (MACH_PORT_NULL != messagePort)
			{
				// The ClientStream had not been created, so deallocate the message port since no ~ClientStreams() was invoked (which normally handles the deallocation)
				(void) mach_port_deallocate(mach_task_self(), messagePort);

				// Reset mOutputBufferRequestPort & mOutputClient to MACH_PORT_NULL
				mOutputBufferRequestPort = MACH_PORT_NULL;
				mOutputClient = MACH_PORT_NULL;
			}
			throw;
		}
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// Stop()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void VCamInputStream::Stop(Client client)
	{
		// Grab the mutex for the overall device's state
		CAMutex::Locker locker(mStateMutex);

		// Delete and erase the client's from the ClientStream map
		ClientStreamMap::iterator i = mClientStreams.find(client);
		if (i != mClientStreams.end())
		{
			CAMutex::Locker clientStreamsLocker(mClientStreamsMutex);
			delete (*i).second;
			mClientStreams.erase(i);
		}

		// Don't stop the stream if it isn't streaming
		if (not mStreaming)
			return;

		// Don't stop the stream if it is an input stream but there are still clients watching it
		if (IsInput() and (not mClientStreams.empty() and MACH_PORT_NULL != client))
			return;

		// Throw an exception if this is an output stream but the stop request is coming from a client that did not make the initial start request
		ThrowIf(IsOutput() and (mOutputClient != client and MACH_PORT_NULL != client), CAException(kCMIODevicePermissionsError), "Device::StopStream: stream was started by a different client"); 

		// The stream is no longer active
		mStreaming = false;

		// Make sure StreamOutputCallback() is not being invoked on another thread
		while (mInOutputCallBack)
			usleep(1000);
			
        // Notify all the client stream message threads that a frame might be available
        mFrameAvailableGuard.NotifyAll();

        // If there are still clients streaming, wait for each client queue of any existing frames be drained
        for (ClientStreamMap::iterator i = mClientStreams.begin() ; i != mClientStreams.end() ; std::advance(i, 1))
        {
            // Yield if a client still has frames in its queue to send
            while (0 != (*i).second->GetQueue().GetCount())
                pthread_yield_np();
        }
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// FrameArrived()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void VCamInputStream::FrameArrived(size_t frameSize, uint8_t *frameData, UInt64 vbiTime)
	{
        // Grab the mutex for the overall device's state
        CAMutex::Locker locker(mStateMutex);

        // Indicate that the output callback is being invoked so Stop() won't release resources
        mInOutputCallBack = true;
        DEFER([this]{ mInOutputCallBack = false; });
        
        if (not mStreaming)
        {
            // The stream has been stopped already, so exit from callback
            return;
        }

        UInt64 firstVBITime = 0;
        UInt64 droppedFrameCount = 0;

		// Create the presentation time stamp
        CMTime presentationTimeStamp = CMTimeMakeWithSeconds(CAHostTimeBase::ConvertToNanos(vbiTime) / 1000000000.0, GetNominalFrameDuration().timescale);
		
		// Create the timing information
        CMA::SampleBuffer::TimingInfo timingInfo(GetNominalFrameDuration(), presentationTimeStamp, kCMTimeInvalid);
		
		// Wrap the entry in a Frame
        Frame* frame = new Frame(GetFrameType(), vbiTime, timingInfo, GetDiscontinuityFlags(), static_cast<UInt32>(droppedFrameCount), firstVBITime, frameSize, frameData);

		// Clear the discontinuity flags since any accumulated discontinuties have passed onward with the frame
		SetDiscontinuityFlags(kCMIOSampleBufferNoDiscontinuities);
	
		// Create a queue of client queues into which the frame should be inserted
		std::vector<CMA::SimpleQueue<Frame*>*> queues;
		
		// Grab the ClientStreams mutex so they won't be altered in the midst of this routine
		CAMutex::Locker clientStreamsLocker(mClientStreamsMutex);

		// Insert the frame into each client's queue
		for (ClientStreamMap::iterator i = mClientStreams.begin() ; i != mClientStreams.end() ; std::advance(i, 1))
		{
			if (1.0 != (*i).second->GetQueue().Fullness())
			{
				// Add the client to the set of clients to which this frame will be messaged
				frame->AddClient((*i).first);
				queues.push_back(&(*i).second->GetQueue());
			}
			else
			{
				// This condition should never be hit, but if that assumption proves false then some code changes would be necessary to properly record the dropped frame.
				DebugMessage("Stream::FrameArrived: client (%d) queue full <---------- NEED TO EXTEND", (*i).first);
			}
		}

		// Enqueue all the frame for all the clients that had room
		for (std::vector<CMA::SimpleQueue<Frame*>*>::iterator i = queues.begin() ; i != queues.end() ; std::advance(i, 1))
			(**i).Enqueue(frame);

		// Make sure at least one client wanted the message
		if (queues.empty())
		{
			// There were no clients to message this too, so invoke RemoveClient(MACH_PORT_NULL) on the frame to make it available for refilling 
			frame->RemoveClient(MACH_PORT_NULL);
		}
		
		if (true)
		{
			mDeck.FrameArrived();
		}
		
		// Notify all the client stream message threads that a frame is available
		mFrameAvailableGuard.NotifyAll();		
    }

}}}}
