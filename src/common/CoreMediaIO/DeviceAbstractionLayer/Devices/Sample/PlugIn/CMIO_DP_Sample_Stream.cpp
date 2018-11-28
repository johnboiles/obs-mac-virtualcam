/*
	    File: CMIO_DP_Sample_Stream.cpp
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
#include "CMIO_DP_Sample_Stream.h"

// Internal Includes
#include "CMIO_DP_Sample_PlugIn.h"
#include "CMIO_DP_Sample_Device.h"
#include "CMIO_DP_Property_EndOfData.h"
#include "CMIO_DP_Property_NoData.h"
#include "CMIO_DP_Property_Deck.h"
#include "CMIO_DP_Property_OutputBuffers.h"
#include "CMIO_DP_Property_FirstOutputPresentationTimeStamp.h"
#include "CMIO_DP_Property_ScheduledOutputNotificationProc.h"

// Public Utility Includes
#include "CMIODebugMacros.h"
#include "CMIO_Buffer.h"
#include "CMIO_CMA_BlockBuffer.h"
#include "CMIO_CMA_Time.h"
#include "CMIO_CVA_Pixel_Buffer.h"
#include "CMIO_SMPTETimeBase.h"

// CA Public Utility Includes
#include "CACFArray.h"
#include "CACFDictionary.h"
#include "CACFNumber.h"
#include "CAException.h"
#include "CAHostTimeBase.h"

// System Includes
#include <IOKit/audio/IOAudioTypes.h>

namespace
{
	const UInt64 kClockTimescale = 8000;
	
	bool IsDeckPropertyAddress(const CMIO::PropertyAddress& address)
	{
		switch (address.mSelector)
		{
			case kCMIOStreamPropertyDeck:
			case kCMIOStreamPropertyDeckFrameNumber:
			case kCMIOStreamPropertyDeckDropness:
			case kCMIOStreamPropertyDeckThreaded:
			case kCMIOStreamPropertyDeckLocal:
			case kCMIOStreamPropertyDeckCueing:
				return true;
		}
		
		return false;
	}
}

namespace CMIO { namespace DP { namespace Sample
{
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// Stream()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	Stream::Stream(CMIOStreamID streamID, PlugIn& plugIn, Device& owningDevice, CMIOObjectPropertyScope scope, UInt32 startingDeviceChannelNumber) :
		DP::Stream(streamID, plugIn, owningDevice, scope, startingDeviceChannelNumber),
		mStreamName(CFSTR("Sample Stream"), false),
		mDiscontinuityFlags(kCMIOSampleBufferNoDiscontinuities),
		mExtendedDurationHostTime(0),
		mExtendedDurationTimingInfo(),
		mNoData(NULL),
		mOutputBuffers(NULL),
		mEndOfData(NULL),
		mFirstOutputPresentationTimeStamp(NULL),
		mScheduledOutputNotificationProc(NULL),
		mDeck(NULL),
		mFormatPairs(),
		mFrameType(DPA::Sample::kYUV422_720x480),
		mDeckPropertyListeners(),
		mMessageThread(),
		mBufferQueue(CMA::SimpleQueue<CMSampleBufferRef>::Create(NULL, 30)),
		mQueueAlteredProc(NULL),
		mQueueAlteredRefCon(NULL),
        mBufferSequenceNumber(kCMIOInvalidSequenceNumber),
        mTimingInfo(kCMTimeInvalid, kCMTimeInvalid,  kCMTimeInvalid),
        mSuspended(false),
		mDeferredNoDataBufferSequenceNumber(kCMIOInvalidSequenceNumber),
		mDeferredNoDataBufferEvent(kCMIOSampleBufferNoDataEvent_Unknown),
		mOutputHosttimeCorrection(0LL),
		mPreviousCycleTimeSeconds(0xFFFFFFFF),
		mSyncClock(true)
	{
	}
	

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// ~Stream()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	Stream::~Stream()
	{
	}
	
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// Initialize()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void Stream::Initialize()
	{
		// Initialize the super class
		DP::Stream::Initialize();

		// Activate the frame rate properties (kCMIOStreamPropertyFrameRate & kCMIOStreamPropertyFrameRates) since the stream's frame rates can be determined
		mFormatList->ActivateFrameRateProperties(true);
 
		// Add the supported formats
		AddAvailableFormatDescriptions();
        
		// Add the Deck property object
		mDeck = new DP::Property::Deck(*this, StreamDeckOneShot, TimeCodeOneShot, DropnessOneShot, ThreadedOneShot, LocalOneShot, CueingOneShot);
		AddProperty(mDeck);
		
		if (IsInput())
		{
			// Add the NoData property
			mNoData = new DP::Property::NoData(*this, 250);
			AddProperty(mNoData);
			
            
            // Create the clock
			if (NULL == mClock->GetClock())
			{
				CFTypeRef clock = NULL;
				OSStatus err = CMIOStreamClockCreate(kCFAllocatorDefault,  CFSTR("CMIO::DP::Sample::Stream"), this,  CMTimeMake(1, 10), 100, 10, (CFTypeRef *)&clock);
				ThrowIfError(err, CAException(kCMIOHardwareUnspecifiedError), "DP::Sample::Stream::Initialize: could not create clock");
				mClock->SetClock(clock);
				CFRelease(clock);
			}

		}
		else if (IsOutput())
		{

			// Add the OutputBuffers property object (allows clients to control output queue size and detect buffer underruns)
			mOutputBuffers = new DP::Property::OutputBuffers(*this, 6);
			AddProperty(mOutputBuffers);

			// Add the EndOfData property object (allows clients to control output queue size and detect buffer underruns)
			mEndOfData = new DP::Property::EndOfData(*this);
			AddProperty(mEndOfData);

			// Add the FirstOutputPresentationTimeStamp property object (allows clients to monitor when output is being sent)
			mFirstOutputPresentationTimeStamp = new DP::Property::FirstOutputPresentationTimeStamp(*this);
			AddProperty(mFirstOutputPresentationTimeStamp);
			
			// Add the ScheduledOutputNotificationProc property object (allows clients to monitor when each frame is being sent)
			mScheduledOutputNotificationProc = new DP::Property::ScheduledOutputNotificationProc(*this);
			AddProperty(mScheduledOutputNotificationProc);
			
            // Create the clock
			if (NULL == mClock->GetClock())
			{
				// Create the clock
				CFTypeRef clock;
				OSStatus err = CMIOStreamClockCreate(kCFAllocatorDefault,  CFSTR("CMIO::DP::Sample::Stream"), this,  CMTimeMake(1, 10), 100, 10, &clock);
				ThrowIfError(err, CAException(kCMIOHardwareUnspecifiedError), "DP::Sample::Stream::Start: could not create clock");
				mClock->SetClock(clock);
				CFRelease(clock);
			}		
		}
	}
	
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// Finalize()
	//	Finalize() is called in place of Teardown() when being lazy about cleaning up. The idea is to do as little work as possible.
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void Stream::Finalize()
	{
	}
	
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// Teardown()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void Stream::Teardown()
	{		
		// Empty all the format descriptions from the format list
		mFormatList->RemoveAllAvailableFormats();
		
		if (NULL != mDeck)
		{
			RemoveProperty(mDeck);
			delete mDeck;
			mDeck = NULL;
		}

		if (IsInput())
		{
			if (NULL != mNoData)
			{
				RemoveProperty(mNoData);
				delete mNoData;
				mNoData = NULL;
			}
		}
		else if (IsOutput())
		{
			if (NULL != mEndOfData)
			{
				RemoveProperty(mEndOfData);
				delete mEndOfData;
				mEndOfData = NULL;
			}

			if (NULL != mOutputBuffers)
			{
				RemoveProperty(mOutputBuffers);
				delete mOutputBuffers;
				mOutputBuffers = NULL;
			}
			if (NULL != mFirstOutputPresentationTimeStamp)
			{
				RemoveProperty(mFirstOutputPresentationTimeStamp);
				delete mFirstOutputPresentationTimeStamp;
				mFirstOutputPresentationTimeStamp = NULL;
			}
			if (NULL != mScheduledOutputNotificationProc)
			{
				RemoveProperty(mScheduledOutputNotificationProc);
				delete mScheduledOutputNotificationProc;
				mScheduledOutputNotificationProc = NULL;
			}
		}
			
		// Invalidate and release the clock (by invalidating the clock, it will return kCMTimeInvalid for any other clients who happen to retain it)
		if (mClock->GetClock())
		{
			CMIOStreamClockInvalidate(mClock->GetClock());
			mClock->SetClock(NULL);
		}
		
		// Teardown the super class
		DP::Stream::Teardown();
	}
	
	#pragma mark -
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// CopyStreamName()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	CFStringRef Stream::CopyStreamName() const
	{
		return mStreamName.CopyCFString();
	}
	
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// GetTerminalType()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	UInt32 Stream::GetTerminalType() const
	{
		// ••• What is the proper type for a CMIO device?
		return INPUT_UNDEFINED;
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// SetNoDataTimeout()
	//	This gets called from the CMIO::DP::Property::NoData::SetPropertyData(), so it should instruct the Assistant to set the no data timeout interval.
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void Stream::SetNoDataTimeout(UInt32 noDataTimeout)
	{
		// Instruct the Assistant to try and set force discontinuity state
		DPA::Sample::SetNoDataTimeout(GetOwningDevice().GetAssistantPort(), GetOwningDevice().GetDeviceGUID(), GetStartingDeviceChannelNumber(), noDataTimeout);
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// SetDeviceSyncTimeout()
	//	This gets called from the CMIO::DP::Property::NoData::SetPropertyData(), so override if anything is needed to be set the device sync timeout interval.
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void Stream::SetDeviceSyncTimeout(UInt32 deviceSyncTimeout)
	{
		// Instruct the Assistant to try and set force discontinuity state
		DPA::Sample::SetDeviceSyncTimeout(GetOwningDevice().GetAssistantPort(), GetOwningDevice().GetDeviceGUID(), GetStartingDeviceChannelNumber(), deviceSyncTimeout);
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// SetEndOfData()
	//	This gets called from the CMIO::DP::Property::EndOfData::SetPropertyData(), so override if anything is needed to be set the stream's end-of-data marker.
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void Stream::SetEndOfData(UInt32 endOfData)
	{
		// Instruct the Assistant to update its end-of-data marker
		DPA::Sample::SetEndOfData(GetOwningDevice().GetAssistantPort(), GetOwningDevice().GetDeviceGUID(), GetStartingDeviceChannelNumber(), endOfData);
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// GetLatency()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    UInt32 Stream::GetLatency() const
    {
        return 6;
    }
    
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// GetMinimumInFlightFramesForThrottledPlayback()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    UInt32 Stream::GetMinimumInFlightFramesForThrottledPlayback() const
    {
        return 6;
    }

	#pragma mark -
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// HasProperty()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	bool Stream::HasProperty(const CMIOObjectPropertyAddress& address) const
	{
		bool answer = false;
		
		// Take and hold the state mutex
		CAMutex::Locker stateMutex(GetOwningDevice().GetStateMutex());
		
		// Do the work if we still have to
		switch (address.mSelector)
		{
			case kCMIOStreamPropertyInitialPresentationTimeStampForLinkedAndSyncedAudio:
				answer = true;
				break;
			
			case kCMIOStreamPropertyOutputBuffersNeededForDroplessPlayback:
				answer = true;
				break;
			
			default:
				answer = DP::Stream::HasProperty(address);
				break;
		};
		
		return answer;
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// IsPropertySettable()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	bool Stream::IsPropertySettable(const CMIOObjectPropertyAddress& address) const
	{
		bool answer = false;
		
		// Take and hold the state mutex
		CAMutex::Locker stateMutex(GetOwningDevice().GetStateMutex());
		
		// Do the work if we still have to
		switch (address.mSelector)
		{
			case kCMIOStreamPropertyInitialPresentationTimeStampForLinkedAndSyncedAudio:
				answer = false;
				break;

			case kCMIOStreamPropertyOutputBuffersNeededForDroplessPlayback:
				answer = false;
				break;
			
			default:
				answer = DP::Stream::IsPropertySettable(address);
				break;
		};
		
		return answer;
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// GetPropertyDataSize()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	UInt32 Stream::GetPropertyDataSize(const CMIOObjectPropertyAddress& address, UInt32 qualifierDataSize, const void* qualifierData) const
	{
		UInt32	answer = 0;
		
		// Take and hold the state mutex
		CAMutex::Locker stateMutex(GetOwningDevice().GetStateMutex());
		
		// Do the work if we still have to
		switch (address.mSelector)
		{
			case kCMIOStreamPropertyInitialPresentationTimeStampForLinkedAndSyncedAudio:
				answer = sizeof(CMTime);
				break;
				
			case kCMIOStreamPropertyOutputBuffersNeededForDroplessPlayback:
				answer = sizeof(UInt32);
				break;
				
			default:
				answer = DP::Stream::GetPropertyDataSize(address, qualifierDataSize, qualifierData);
				break;
		};
		
		return answer;
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// GetPropertyData()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void Stream::GetPropertyData(const CMIOObjectPropertyAddress& address, UInt32 qualifierDataSize, const void* qualifierData, UInt32 dataSize, UInt32& dataUsed, void* data) const
	{
		// Take and hold the state mutex
		CAMutex::Locker stateMutex(GetOwningDevice().GetStateMutex());
		
		// Do the work if we still have to
		switch (address.mSelector)
		{
			case kCMIOStreamPropertyInitialPresentationTimeStampForLinkedAndSyncedAudio:
				{
					ThrowIf(dataSize != GetPropertyDataSize(address, qualifierDataSize, qualifierData), CAException(kCMIOHardwareBadPropertySizeError), "CMIO::DP::Sample::Device::GetPropertyData: wrong data size for kCMIOStreamPropertyInitialPresentationTimeStampForLinkedAndSyncedAudio");
					ThrowIf(qualifierDataSize != sizeof(AudioTimeStamp), CAException(kCMIOHardwareBadPropertySizeError), "CMIO::DP::Sample::Device::GetPropertyData: wrong qualifier data size for kCMIOStreamPropertyInitialPresentationTimeStampForLinkedAndSyncedAudio");
					
					// Get the audio time stamp
					const AudioTimeStamp *audioTimeStamp = reinterpret_cast<const AudioTimeStamp *>(qualifierData);
					ThrowIf((kAudioTimeStampHostTimeValid & audioTimeStamp->mFlags) == 0, CAException(kCMIOHardwareUnspecifiedError), "CMIO::DP::Sample::Device::GetPropertyData: qualifier data AudioTimeStamp hosttime is not valid for kCMIOStreamPropertyInitialPresentationTimeStampForLinkedAndSyncedAudio");
					
					// If we are not streaming we really shouldn't be in here, so signal that we won't ever have a timestamp for the buffer
					if (not Streaming())
					{
						*(static_cast<CMTime*>(data)) = kCMTimeInvalid;
					}
					else
					{
						// Try to get the most recent PTS;  get twice so we can
						// be assured of getting a coherit value.
						
						RecentTimingInfo	recentTimingInfo[ 4 ];
						
						OSMemoryBarrier();
						recentTimingInfo[ 0 ] =	mRecentTimingInfo[ 0 ];
						recentTimingInfo[ 1 ] =	mRecentTimingInfo[ 0 ];
						recentTimingInfo[ 2 ] =	mRecentTimingInfo[ 1 ];
						recentTimingInfo[ 3 ] =	mRecentTimingInfo[ 1 ];
						
						// Determine which events are valid.
						
						bool timing0Valid = (	 recentTimingInfo[ 0 ].mValid and recentTimingInfo[ 1 ].mValid
											 and CMTIME_IS_VALID(recentTimingInfo[ 0 ].mPTS)
											 and (recentTimingInfo[ 0 ].mHostTime == recentTimingInfo[ 1 ].mHostTime)
											 and (CMTimeCompare(recentTimingInfo[ 0 ].mPTS, recentTimingInfo[ 1 ].mPTS) == 0));
						
						bool timing1Valid = (	 recentTimingInfo[ 2 ].mValid and recentTimingInfo[ 3 ].mValid
											 and CMTIME_IS_VALID(recentTimingInfo[ 2 ].mPTS)
											 and (recentTimingInfo[ 2 ].mHostTime == recentTimingInfo[ 3 ].mHostTime)
											 and (CMTimeCompare(recentTimingInfo[ 2 ].mPTS, recentTimingInfo[ 3 ].mPTS) == 0));
						
						// Find index of most recent time.  If neither are valid, set index to an invalid value.
						UInt32	mostRecentIdx;
						
						if (timing0Valid and timing1Valid)
						{
							mostRecentIdx = (recentTimingInfo[ 0 ].mHostTime > recentTimingInfo[ 2 ].mHostTime) ? 0 : 2;
						}
						else if (timing0Valid or timing1Valid)
						{
							mostRecentIdx = timing0Valid ? 0 : 2;
						}
						else
						{
							mostRecentIdx = 0xFFFFFFFF;
						}
						
						// If we don't have a valid time, signal that the client needs to try again later
						if (0xFFFFFFFF == mostRecentIdx)
						{
							DebugMessage("Sample::Stream::GetPropertyData kCMIOStreamPropertyInitialPresentationTimeStampForLinkedAndSyncedAudio: waiting on audio ht %lld", audioTimeStamp->mHostTime);
							*(static_cast<CMTime*>(data)) = kCMTimeIndefinite;
						}
						else
						{
							// For the Sample device, which is simple, we will use the difference in hosttimes
							// between the audio timestamp and our most recent frame's hosttime to calculate
							// a PTS for the audio that is relative to the PTS for the video.
							if (audioTimeStamp->mHostTime == recentTimingInfo[ mostRecentIdx ].mHostTime)
							{
								*(static_cast<CMTime*>(data)) = recentTimingInfo[ mostRecentIdx ].mPTS;
							}
							else if (audioTimeStamp->mHostTime < recentTimingInfo[ mostRecentIdx ].mHostTime)
							{
								// Audio hostttime is before frame's hosttime, so audio PTS will be before frame's PTS.
								CMTime hostTimeDelta = CMTimeMake(CAHostTimeBase::ConvertToNanos(recentTimingInfo[ mostRecentIdx ].mHostTime - audioTimeStamp->mHostTime), 1000000000);
								hostTimeDelta = CMTimeConvertScale(hostTimeDelta, recentTimingInfo[ mostRecentIdx ].mPTS.timescale, kCMTimeRoundingMethod_Default);
								*(static_cast<CMTime*>(data)) = CMTimeSubtract(recentTimingInfo[ mostRecentIdx ].mPTS, hostTimeDelta);
								{
									UInt64 now = CAHostTimeBase::GetTheCurrentTime();
									DebugMessage("Sample::Stream::GetPropertyData kCMIOStreamPropertyInitialPresentationTimeStampForLinkedAndSyncedAudio: now:  %lld, most recent video ht: %lld, audio ht: %lld", now, recentTimingInfo[ mostRecentIdx ].mHostTime, audioTimeStamp->mHostTime);
									DebugMessage("Sample::Stream::GetPropertyData kCMIOStreamPropertyInitialPresentationTimeStampForLinkedAndSyncedAudio: now/vht delta:  %lf, now/aht delta %lf, v/a ht delta %lf",
												 (Float64)CAHostTimeBase::ConvertToNanos(now - recentTimingInfo[ mostRecentIdx ].mHostTime) / 1000000000.0,
												 (Float64)CAHostTimeBase::ConvertToNanos(now - audioTimeStamp->mHostTime) / 1000000000.0,
												 (Float64)CAHostTimeBase::ConvertToNanos(recentTimingInfo[ mostRecentIdx ].mHostTime - audioTimeStamp->mHostTime) / 1000000000.0);
									DebugMessage("Sample::Stream::GetPropertyData kCMIOStreamPropertyInitialPresentationTimeStampForLinkedAndSyncedAudio: most recent v PTS:  %lld : %d, audio PTS:  %lld : %d, delta = %lf",
												 recentTimingInfo[ mostRecentIdx ].mPTS.value, recentTimingInfo[ mostRecentIdx ].mPTS.timescale,
												 (static_cast<CMTime*>(data))->value, (static_cast<CMTime*>(data))->timescale,
												 CMTimeGetSeconds(CMTimeSubtract(recentTimingInfo[ mostRecentIdx ].mPTS, *(static_cast<CMTime*>(data)))));
								}
							}
							else
							{
								// Audio hostttime is after frame's hosttime, so audio PTS will be after frame's PTS.
								CMTime hostTimeDelta = CMTimeMake(CAHostTimeBase::ConvertToNanos(audioTimeStamp->mHostTime - recentTimingInfo[ mostRecentIdx ].mHostTime), 1000000000);
								hostTimeDelta = CMTimeConvertScale(hostTimeDelta, recentTimingInfo[ mostRecentIdx ].mPTS.timescale, kCMTimeRoundingMethod_Default);
								*(static_cast<CMTime*>(data)) = CMTimeAdd(recentTimingInfo[ mostRecentIdx ].mPTS, hostTimeDelta);
								{
									UInt64 now = CAHostTimeBase::GetTheCurrentTime();
									DebugMessage("Sample::Stream::GetPropertyData kCMIOStreamPropertyInitialPresentationTimeStampForLinkedAndSyncedAudio: now:  %lld, most recent video ht: %lld, audio ht: %lld", now, recentTimingInfo[ mostRecentIdx ].mHostTime, audioTimeStamp->mHostTime);
									DebugMessage("Sample::Stream::GetPropertyData kCMIOStreamPropertyInitialPresentationTimeStampForLinkedAndSyncedAudio: now/vht delta:  %lf, now/aht delta %lf, v/a ht delta %lf",
												 (Float64)CAHostTimeBase::ConvertToNanos(now - recentTimingInfo[ mostRecentIdx ].mHostTime) / 1000000000.0,
												 (Float64)CAHostTimeBase::ConvertToNanos(now - audioTimeStamp->mHostTime) / 1000000000.0,
												 (Float64)CAHostTimeBase::ConvertToNanos(recentTimingInfo[ mostRecentIdx ].mHostTime - audioTimeStamp->mHostTime) / 1000000000.0);
									DebugMessage("Sample::Stream::GetPropertyData kCMIOStreamPropertyInitialPresentationTimeStampForLinkedAndSyncedAudio: most recent v PTS:  %lld : %d, audio PTS:  %lld : %d, delta = %lf",
												 recentTimingInfo[ mostRecentIdx ].mPTS.value, recentTimingInfo[ mostRecentIdx ].mPTS.timescale,
												 (static_cast<CMTime*>(data))->value, (static_cast<CMTime*>(data))->timescale,
												 CMTimeGetSeconds(CMTimeSubtract(recentTimingInfo[ mostRecentIdx ].mPTS, *(static_cast<CMTime*>(data)))));
								}
							}
						}
					}
				}
				dataUsed = sizeof(CMTime);
				break;
				
			case kCMIOStreamPropertyOutputBuffersNeededForDroplessPlayback:
				ThrowIf(dataSize != GetPropertyDataSize(address, qualifierDataSize, qualifierData), CAException(kCMIOHardwareBadPropertySizeError), "CMIO::DP::Stream::GetPropertyData: wrong data size for kCMIOStreamPropertyOutputBuffersNeededForDroplessPlayback");
				*static_cast<UInt32*>(data) = GetMinimumInFlightFramesForThrottledPlayback();
				dataUsed = sizeof(UInt32);
				break;

			default:
				DP::Stream::GetPropertyData(address, qualifierDataSize, qualifierData, dataSize, dataUsed, data);
				break;
		};
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// SetPropertyData()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void Stream::SetPropertyData(const CMIOObjectPropertyAddress& address, UInt32 qualifierDataSize, const void* qualifierData, UInt32 dataSize, const void* data)
	{
		ThrowIf(not GetOwningDevice().HogModeIsOwnedBySelfOrIsFree(), CAException(kCMIODevicePermissionsError), "CMIO::DP::Sample::Stream::SetPropertyData: can't set the property because hog mode is owned by another process");

		// Take and hold the state mutex
		CAMutex::Locker stateMutex(GetOwningDevice().GetStateMutex());

		switch (address.mSelector)
		{
			default:
				DP::Stream::SetPropertyData(address, qualifierDataSize, qualifierData, dataSize, data);
				break;
		};
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// PropertyListenerAdded()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void Stream::PropertyListenerAdded(const CMIOObjectPropertyAddress& address)
	{
		DebugMessage("DP::Sample::Stream::PropertyListenerAdded");
		DP::Stream::PropertyListenerAdded(address);
		
		// Don't do anything if the address is not deck related
		if (not IsDeckPropertyAddress(address))
			return;
		
		// Remember that this deck property is being listened too
		mDeckPropertyListeners.AppendUniqueExactItem(address);
		
		// If the deck was all ready set up to NOT use one-shot getters, return
		if (not mDeck->UseOneShotGetters())
			return;
		
        DebugMessage("DP::Sample::Stream::PropertyListenerAdded calling StartDeckThreads");
		
		// Tell the deck to stop using one-shot getters
		mDeck->SetUseOneShotGetters(false);
		
		// Tell the Assitant to start the threads it uses to track the deck properties
		DPA::Sample::StartDeckThreads(GetOwningDevice().GetAssistantPort(), GetOwningDevice().GetDeviceGUID(), GetDevicePropertyScope(), GetStartingDeviceChannelNumber());
	}
	
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// PropertyListenerRemoved()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void Stream::PropertyListenerRemoved(const CMIOObjectPropertyAddress& address)
	{
		// Call the super class
		DP::Stream::PropertyListenerRemoved(address);
		
		// Don't do anything if the address is not deck related
		if (not IsDeckPropertyAddress(address))
			return;
		
		// Remove the address from the list of deck properties being listend too
		mDeckPropertyListeners.EraseExactItem(address);
		
		// Return if there are still any listeners
		if (not mDeckPropertyListeners.IsEmpty())
			return;
		
		// Tell the Assistant it can stop the threads it was using to track the deck properties
		DPA::Sample::StopDeckThreads(GetOwningDevice().GetAssistantPort(), GetOwningDevice().GetDeviceGUID(), GetDevicePropertyScope(), GetStartingDeviceChannelNumber());
		
		// Tell the deck to use one-shot getters
		mDeck->SetUseOneShotGetters(true);
	}
	
	
	#pragma mark -
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// UpdatePropertyState()
	//	Ultimately invoked via the Assistant process notfiying this process of property changes.
	//	NOTE:  The address is specfied in DEVICE RELATIVE terms (i.e., { selector, kCMIODevicePropertyScopeXXX, element # })
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void Stream::UpdatePropertyState(const PropertyAddress& address, bool sendChangeNotifications) 
	{
		// Take and hold the state mutex
		CAMutex::Locker stateMutex(GetOwningDevice().GetStateMutex());

		if (PropertyAddress::IsSameAddress(address, PropertyAddress(kCMIOStreamPropertyFormatDescription, GetDevicePropertyScope(), GetStartingDeviceChannelNumber())))
		{
			// Get the current FrameType
			DPA::Sample::FrameType frameType = DPA::Sample::GetFormatDescription(GetOwningDevice().GetAssistantPort(), GetOwningDevice().GetDeviceGUID(), GetDevicePropertyScope(), GetStartingDeviceChannelNumber());
			
			if (mFrameType != frameType)
			{
				// Inform the format list what the current format description is
				mFormatList->SetCurrentFormat(mFormatPairs[frameType].first, false);

				// Figure out out the necessary property changed notifications
				PropertyAddressList notifications;
				DP::Property::FormatList::DetermineNotifications(*this, mFormatPairs[mFrameType].first, mFormatPairs[frameType].first, notifications);
				
				// Update the FrameType
				mFrameType = frameType;

				// Send out the property changed notifications 
				PropertiesChanged(notifications.GetNumberItems(), notifications.GetItems());
			}
		
		}
		else if (PropertyAddress::IsSameAddress(address, PropertyAddress(kCMIOStreamPropertyFrameRate, GetDevicePropertyScope(), GetStartingDeviceChannelNumber())))
		{
			// Get the current frame rate 
			Float64 frameRate = DPA::Sample::GetFrameRate(GetOwningDevice().GetAssistantPort(), GetOwningDevice().GetDeviceGUID(), GetDevicePropertyScope(), GetStartingDeviceChannelNumber());
			
			if (frameRate != mFormatList->GetCurrentFrameRate())
			{
				// Inform the format list what the current frame rate is
				mFormatList->SetCurrentFrameRate(frameRate, false);

				// Indicate that kCMIOStreamPropertyFrameRate has changed
				PropertyAddress address(kCMIOStreamPropertyFrameRate);
				PropertiesChanged(1, &address);
			}
		
		}
		else if (PropertyAddress::IsSameAddress(address, PropertyAddress(kCMIOStreamPropertyFrameRates, GetDevicePropertyScope(), GetStartingDeviceChannelNumber())))
		{
			// Get the current FrameType
			DPA::Sample::FrameType frameType = DPA::Sample::GetFormatDescription(GetOwningDevice().GetAssistantPort(), GetOwningDevice().GetDeviceGUID(), GetDevicePropertyScope(), GetStartingDeviceChannelNumber());
			
			// Get the frame rates for this format
			DPA::Sample::AutoFreeUnboundedArray<Float64> frameRates;
			DPA::Sample::GetFrameRates(GetOwningDevice().GetAssistantPort(), GetOwningDevice().GetDeviceGUID(), GetDevicePropertyScope(), GetStartingDeviceChannelNumber(), frameType, frameRates);
			mFormatList->SetFrameRates(mFormatPairs[frameType].first, frameRates.GetLength(), &frameRates[0]);

			// Indicate that kCMIOStreamPropertyFrameRates has changed
			PropertyAddress address(kCMIOStreamPropertyFrameRates);
			PropertiesChanged(1, &address);
		}
		else if (PropertyAddress::IsSameAddress(address, PropertyAddress(kCMIOStreamPropertyNoDataTimeoutInMSec, kCMIODevicePropertyScopeInput, GetStartingDeviceChannelNumber())))
		{
			if (NULL != mNoData)
				mNoData->SetNoDataTimeoutInMSec(DPA::Sample::GetNoDataTimeout(GetOwningDevice().GetAssistantPort(), GetOwningDevice().GetDeviceGUID(), GetStartingDeviceChannelNumber()));
		}
		else if (PropertyAddress::IsSameAddress(address, PropertyAddress(kCMIOStreamPropertyDeviceSyncTimeoutInMSec, kCMIODevicePropertyScopeInput, GetStartingDeviceChannelNumber())))
		{
			if (NULL != mNoData)
				mNoData->SetDeviceSyncTimeoutInMSec(DPA::Sample::GetDeviceSyncTimeout(GetOwningDevice().GetAssistantPort(), GetOwningDevice().GetDeviceGUID(), GetStartingDeviceChannelNumber()));
		}
		else if (PropertyAddress::IsSameAddress(address, PropertyAddress(kCMIOStreamPropertyNoDataEventCount, kCMIODevicePropertyScopeInput, GetStartingDeviceChannelNumber())))
		{
			if (NULL != mNoData)
				mNoData->BumpNoDataEventCount();
		}
		else if (PropertyAddress::IsSameAddress(address, PropertyAddress(kCMIOStreamPropertyEndOfData, kCMIODevicePropertyScopeOutput, GetStartingDeviceChannelNumber())))
		{
			if (NULL != mEndOfData)
				mEndOfData->SetEndOfData(DPA::Sample::GetEndOfData(GetOwningDevice().GetAssistantPort(), GetOwningDevice().GetDeviceGUID(), GetStartingDeviceChannelNumber()), sendChangeNotifications);
		}
		else if (PropertyAddress::IsSameAddress(address, PropertyAddress(kCMIOStreamPropertyOutputBufferUnderrunCount, kCMIODevicePropertyScopeOutput, GetStartingDeviceChannelNumber())))
		{
			if (NULL != mOutputBuffers)
				mOutputBuffers->BumpUnderrunCount();
		}
		else if (PropertyAddress::IsSameAddress(address, PropertyAddress(kCMIOStreamPropertyDeck, GetDevicePropertyScope(), GetStartingDeviceChannelNumber())))
		{
			mDeck->SetStreamDeck(DPA::Sample::GetDeck(GetOwningDevice().GetAssistantPort(), GetOwningDevice().GetDeviceGUID(), GetDevicePropertyScope(), GetStartingDeviceChannelNumber()), sendChangeNotifications);
		}
		else if (PropertyAddress::IsSameAddress(address, PropertyAddress(kCMIOStreamPropertyDeckFrameNumber, GetDevicePropertyScope(), GetStartingDeviceChannelNumber())))
		{
			mDeck->SetTimecode(DPA::Sample::GetDeckTimecode(GetOwningDevice().GetAssistantPort(), GetOwningDevice().GetDeviceGUID(), GetDevicePropertyScope(), GetStartingDeviceChannelNumber()), sendChangeNotifications);
		}
		else if (PropertyAddress::IsSameAddress(address, PropertyAddress(kCMIOStreamPropertyDeckCueing, GetDevicePropertyScope(), GetStartingDeviceChannelNumber())))
		{
			mDeck->SetCueing(DPA::Sample::GetDeckCueing(GetOwningDevice().GetAssistantPort(), GetOwningDevice().GetDeviceGUID(), GetDevicePropertyScope(), GetStartingDeviceChannelNumber()), sendChangeNotifications);
		}
		else
		{
			// An unknown property
			throw CAException(kCMIOHardwareUnknownPropertyError);
		}
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// DriveOutputClock()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void Stream::DriveOutputClock(CMTime presentationTimeStamp, CMTime clockTime, UInt64 nanosecondsHostTime)
	{
		// If we are syncing the clock, then we need to get the current hosttime and calculate a correction to pull the driving clock time into the past.  This is because we want to make
		// sure that the device clock is not trying to see into the future.
		if (mSyncClock)
		{
			UInt64 nowInNanos = CAHostTimeBase::GetCurrentTimeInNanos();
			
			if (nowInNanos < nanosecondsHostTime)
			{
				mOutputHosttimeCorrection = nanosecondsHostTime - nowInNanos;
			}
			else
			{
				mOutputHosttimeCorrection = 0;
			}
			
			mOutputHosttimeCorrection += (1000000000 / 10);
		}
		
		// Correct the hosttime and drive the clock
		nanosecondsHostTime -= mOutputHosttimeCorrection;

		#if 1
			DebugMessage("DP::Sample::Stream::DriveOutputClock: +++\t%p\t%lld\t%d\t%lld", mClock->GetClock(), clockTime.value, clockTime.timescale, nanosecondsHostTime);
		#endif
		
		OSStatus err = CMIOStreamClockPostTimingEvent(clockTime, CAHostTimeBase::ConvertFromNanos(nanosecondsHostTime), mSyncClock, mClock->GetClock());
		DebugMessageIfError(err, "DP::Sample::Stream::DriveOutputClock: CMIOStreamClockPostTimingEvent() failed");
				
		// If we are syncing the clock then we set the first output presentation timestamp so that clients can
		// start their graph timebase
		if (mSyncClock)
		{
			#if 1
				printf("+++\t%p\t%lld\t%d\t%lld\n", mClock->GetClock(), clockTime.value, clockTime.timescale, nanosecondsHostTime);
			#endif
			
			mFirstOutputPresentationTimeStamp->SetFirstOutputPresentationTimeStamp(presentationTimeStamp);
		}
		
		mSyncClock = false;
	}
	
	
	#pragma mark -
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// TellHardwareToSetFormatDescription()
	//	This method is called to tell the hardware to change format. It returns true if the format change takes place immediately, which is the case for this device.
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	bool Stream::TellHardwareToSetFormatDescription(CMFormatDescriptionRef format)
	{
		// Make sure this process is allowed to make changes
		ThrowIf(not GetOwningDevice().DeviceMasterIsOwnedBySelfOrIsFree(), CAException(kCMIODevicePermissionsError), "CMIO::DP::Sample::Stream::TellHardwareToSetFormatDescription: can't set the property because the device master is owned by another process");

		// If the format is the same as the current format or the formats are equal, no need to do anything
		if (format == mFormatList->GetCurrentFormat() or CMFormatDescriptionEqual(format, mFormatList->GetCurrentFormat()))
			return true;
		
		// Find the entry in the FormatPairMap for the requested format, and extract the FrameType
		FormatPairMap::const_iterator i = std::find_if(mFormatPairs.begin(), mFormatPairs.end(), FormatDescriptionEquals(format));
		ThrowIf(i == mFormatPairs.end(), CAException(kIOReturnNoDevice), "CMIO::DP::Sample::Stream::TellHardwareToSetFormatDescription: cound not find new format description in FormatPair map");

		// Attempt to set the stream to generate the desired frame type
		DPA::Sample::SetFormatDescription(GetOwningDevice().GetAssistantPort(), GetOwningDevice().GetDeviceGUID(), GetDevicePropertyScope(), GetStartingDeviceChannelNumber(), (*i).first);

		return true;
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// RefreshAvailableFormatDescriptions()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void Stream::RefreshAvailableFormatDescriptions()
	{
		mFormatList->RemoveAllAvailableFormats();
		AddAvailableFormatDescriptions();
		
		// Indicate that kCMIOStreamPropertyFormatDescription has changed
		PropertyAddressList changedProperties;
		PropertyAddress address(kCMIOStreamPropertyFormatDescription);
		changedProperties.AppendUniqueItem(address);

		// Indicate that kCMIOStreamPropertyFormatDescriptions has changed
		address.mSelector = kCMIOStreamPropertyFormatDescriptions;
		changedProperties.AppendUniqueItem(address);

		// Indicate that kCMIOStreamPropertyFrameRate has changed
		address.mSelector = kCMIOStreamPropertyFrameRate;
		changedProperties.AppendUniqueItem(address);

		// Indicate that kCMIOStreamPropertyFrameRates has changed
		address.mSelector = kCMIOStreamPropertyFrameRates;
		changedProperties.AppendUniqueItem(address);

		// Send out the property changed notifications 
		PropertiesChanged(changedProperties.GetNumberItems(), changedProperties.GetItems());
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// AddAvailableFormatDescriptions()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void Stream::AddAvailableFormatDescriptions()
	{
		// Get frame formats
		DPA::Sample::AutoFreeUnboundedArray<DPA::Sample::FrameFormat> frameFormats;
		DPA::Sample::GetFormatDescriptions(GetOwningDevice().GetAssistantPort(), GetOwningDevice().GetDeviceGUID(), GetDevicePropertyScope(), GetStartingDeviceChannelNumber(), frameFormats);
		
		// Create a dictionary to hold the extensions for the format description
		CACFDictionary extensions(true);
		
		// Only has I frames
		extensions.AddCFType(CFSTR(kCMIO_DPA_Sample_VideoOnlyHasIFrames), kCFBooleanTrue);

		// Only 1 field
		extensions.AddCFType(kCMFormatDescriptionExtension_FieldCount, CACFNumber(static_cast<SInt32>(1)).GetCFNumber());	
		
		// Mark 6-1-6 (SMPTE-C) color tags
		extensions.AddCFType(kCMFormatDescriptionExtension_ColorPrimaries, kCMFormatDescriptionColorPrimaries_SMPTE_C);
		extensions.AddCFType(kCMFormatDescriptionExtension_TransferFunction, kCMFormatDescriptionTransferFunction_ITU_R_709_2);
		extensions.AddCFType(kCMFormatDescriptionExtension_YCbCrMatrix, kCMFormatDescriptionYCbCrMatrix_ITU_R_601_4);

		for (int i = 0; i < frameFormats.GetLength() ; ++i)
		{
			switch (frameFormats[i].mCodecType)
			{
				case kCMVideoCodecType_422YpCbCr8:
					extensions.AddCFType(kCMFormatDescriptionExtension_FormatName, CFSTR("Component Video - CCIR-601 uyvy"));
					break;

				case kCMPixelFormat_422YpCbCr10:
					extensions.AddCFType(kCMFormatDescriptionExtension_FormatName, CFSTR("Component Video - CCIR-601 v210"));
					break;
                    
				case kCMPixelFormat_32ARGB:
					extensions.AddCFType(kCMFormatDescriptionExtension_FormatName, CFSTR("Component Video - CCIR-601 RGB"));
					break;
				
				default:
					ThrowIf(true, CAException(kCMIODeviceUnsupportedFormatError), "CMIO::DP::Sample::Stream::AddAvailableFormatDescriptions: Assistant returned an unknown format");
			}

			// Create the format description
			CMA::FormatDescription description(CMA::FormatDescription::VideoFormatDescriptionCreate(NULL, frameFormats[i].mCodecType, frameFormats[i].mWidth, frameFormats[i].mHeight, extensions.GetCFMutableDictionary()), false);
			
			// Add it to the format list
			mFormatList->AddAvailableFormat(description);

			// Add the pair:<CMFormatDescriptionRef, DPA::Sample::FrameFormat> to the format pair map
			mFormatPairs[frameFormats[i].mFrameType] = std::make_pair(description.Get(), frameFormats[i]);
			
			// Get the frame rates for this format
			DPA::Sample::AutoFreeUnboundedArray<Float64> frameRates;
			DPA::Sample::GetFrameRates(GetOwningDevice().GetAssistantPort(), GetOwningDevice().GetDeviceGUID(), GetDevicePropertyScope(), GetStartingDeviceChannelNumber(), frameFormats[i].mFrameType, frameRates);
			mFormatList->SetFrameRates(description, frameRates.GetLength(), &frameRates[0]);
		}

		// Get the current FrameType
		DPA::Sample::FrameType frameType = DPA::Sample::GetFormatDescription(GetOwningDevice().GetAssistantPort(), GetOwningDevice().GetDeviceGUID(), GetDevicePropertyScope(), GetStartingDeviceChannelNumber());

		// Inform the format list what the current format description is
		mFormatList->SetCurrentFormat(mFormatPairs[frameType].first, false);

		// Get the current frame rate
		Float64 frameRate = DPA::Sample::GetFrameRate(GetOwningDevice().GetAssistantPort(), GetOwningDevice().GetDeviceGUID(), GetDevicePropertyScope(), GetStartingDeviceChannelNumber());
		mFormatList->SetCurrentFrameRate(frameRate, false);
	}

	#pragma mark -
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// TellHardwareToSetFrameRate()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	bool Stream::TellHardwareToSetFrameRate(Float64 frameRate)
	{
		// Make sure this process is allowed to make changes
		ThrowIf(not GetOwningDevice().DeviceMasterIsOwnedBySelfOrIsFree(), CAException(kCMIODevicePermissionsError), "CMIO::DP::Sample::Stream::TellHardwareToSetFrameRate: can't set the property because the device master is owned by another process");

		// Don't do anything if it is the same as the current frame rate
		if (frameRate == mFormatList->GetCurrentFrameRate())
			return true;
		
		// Set the frame rate
		DPA::Sample::SetFrameRate(GetOwningDevice().GetAssistantPort(), GetOwningDevice().GetDeviceGUID(), GetDevicePropertyScope(), GetStartingDeviceChannelNumber(), frameRate);		

		return true;
	}

	#pragma mark -
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// Start()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void Stream::Start()
	{
		// Throw an exception if another process is hogging the device
		ThrowIf(not GetOwningDevice().HogModeIsOwnedBySelfOrIsFree(), CAException(kCMIODevicePermissionsError), "CMIO::DP::Sample::Stream::Start: can't start the stream because hog mode is owned by another process");

		// Throw an exception if already streaming
		ThrowIf(Streaming(), CAException(kCMIOHardwareNotStoppedError), "CMIO::DP::Sample::Stream::Start: Can't start stream when it is already running");
		
		// Spawn the thread that will get messaged from the Assistant with frames
		mMessageThread.Reset(reinterpret_cast<CFMachPortCallBack>(Messages), this);

		// Wait for the message thread to be running prior to continuing
		while (PTA::CFMachPortThread::kStarting == mMessageThread.GetState())
			pthread_yield_np();

		// Make sure the notification port is not invalid
		ThrowIf(PTA::CFMachPortThread::kInvalid == mMessageThread.GetState(), -1, "CMIO::DP::Sample::Stream::Start: CFMachPortThread thread invalid");

		if (IsInput())
		{
			// Initialize the clock support
			mTimingInfo = kCMTimingInfoInvalid;
			mSyncClock = true;
			mRecentTimingInfo[0].mPTS = kCMTimeInvalid;
			mRecentTimingInfo[0].mHostTime = 0;
			mRecentTimingInfo[0].mValid = false;
			mRecentTimingInfo[1] = mRecentTimingInfo[0];
			mRecentTimingInfoIdx = 0;
			OSMemoryBarrier();
			
			// Clear any accumulated discontinuity flags
			SetDiscontinuityFlags(kCMIOSampleBufferNoDiscontinuities);
			
			// No need to insert a deferred "no data" buffer
			mDeferredNoDataBufferSequenceNumber = kCMIOInvalidSequenceNumber;
	
			// Start the stream
			DPA::Sample::StartStream(GetOwningDevice().GetAssistantPort(), GetOwningDevice().GetDeviceGUID(), mMessageThread.GetMachPort(), GetDevicePropertyScope(), GetStartingDeviceChannelNumber());
		}
		else if (IsOutput())
		{
			// Initialize the clock support
			// (Do what is necessary to intialize the output clock)

			// Start the stream
			DPA::Sample::StartStream(GetOwningDevice().GetAssistantPort(), GetOwningDevice().GetDeviceGUID(), mMessageThread.GetMachPort(), GetDevicePropertyScope(), GetStartingDeviceChannelNumber());

			// Initialize the clock support
			mOutputHosttimeCorrection =	0LL;
			mSyncClock = true;
		}

		// The stream is active now
		mStreaming = true;
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// Stop()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void Stream::Stop()
	{
		// Simply return if not streaming
		if (not Streaming())
			return;
			
		if (IsInput())
		{
			// Stop the stream
			(void) CMIODPASampleStopStream(GetOwningDevice().GetAssistantPort(), GetOwningDevice().GetDeviceGUID(), GetDevicePropertyScope(), GetStartingDeviceChannelNumber());
		}
		else if (IsOutput())
		{
			// Stop the stream
			(void) CMIODPASampleStopStream(GetOwningDevice().GetAssistantPort(), GetOwningDevice().GetDeviceGUID(), GetDevicePropertyScope(), GetStartingDeviceChannelNumber());
			mFirstOutputPresentationTimeStamp->SetFirstOutputPresentationTimeStamp(kCMTimeInvalid);
		}
		
		// Release the message thread
		mMessageThread.Reset();
			
		// Extract the individual CMSampleBufferRefs from the queue and release them (conveniently, when CMIO::Buffer goes out of scope the wrapped CMSampleBufferRef will be released)
		while (0 != mBufferQueue.GetCount())
			CMIO::Buffer buffer(reinterpret_cast<CMSampleBufferRef>(mBufferQueue.Dequeue()));
		
		// Neuter the clock reporting
		mRecentTimingInfo[0].mPTS = kCMTimeInvalid;
		mRecentTimingInfo[0].mHostTime = 0;
		mRecentTimingInfo[0].mValid = false;
		mRecentTimingInfo[1] = mRecentTimingInfo[0];
		mRecentTimingInfoIdx = 0;
		OSMemoryBarrier();

		// The stream is no longer active
		mStreaming = false;
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// Unplug()
	//	This is invoked in response to getting unplugged while the underlying hardware devcie was opened.
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void Stream::Unplug()
	{
		// Simply return if not streaming or already suspended
		if (not Streaming() or mSuspended)
			return;
			
		if (IsInput())
		{
		}
		else if (IsOutput())
		{
		}
		
		// Mark the stream as suspended
		mSuspended = true;
	}
    
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// CopyBufferQueue()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	CMSimpleQueueRef Stream::CopyBufferQueue(CMIODeviceStreamQueueAlteredProc queueAlteredProc, void* queueAlteredRefCon)
	{
		// Initialize the queue altered proc and its associated refCon
		mQueueAlteredProc = queueAlteredProc;
		mQueueAlteredRefCon	= queueAlteredRefCon;
       
		#if (MAC_OS_X_VERSION_MIN_REQUIRED > MAC_OS_X_VERSION_10_8)
			// A NULL callback function indicates the client is unregistering, so return NULL as the buffer queue
			if (NULL == mQueueAlteredProc)
				return NULL;
		#endif

		// Retain the buffer queue since it will be handed back to the client
		CFRetain(mBufferQueue);

		return mBufferQueue;
	}

	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// ReleaseBufferCallback()
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void Stream::ReleaseBufferCallback(void* refCon, void* doomedMemoryBlock, size_t sizeInBytes)
	{
		(void) vm_deallocate(mach_task_self(), reinterpret_cast<vm_address_t>(doomedMemoryBlock), sizeInBytes);
	}

	#pragma mark -
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// Messages()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void Stream::Messages(CFMachPortRef port, mach_msg_header_t* header, CFIndex size, Stream& stream) 
	{
		// Examine the message ID
		switch (header->msgh_id)
		{
			case DPA::Sample::kFrameArrived:
				stream.FrameArrived(reinterpret_cast<DPA::Sample::FrameArrivedMessage*>(header));
				break;

			case DPA::Sample::kOutputBufferRequested:
				stream.GetOutputBuffer(reinterpret_cast<DPA::Sample::OutputBufferRequestedMessage*>(header));
				break;
		}
	}

	#pragma mark -
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// FrameArrived()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void Stream::FrameArrived(DPA::Sample::FrameArrivedMessage* message) 
	{
		// Don't do anything if the buffer queue is full
		if (1.0 == mBufferQueue.Fullness())
		{
			// Deallocate the frame data
			vm_deallocate(mach_task_self(), reinterpret_cast<vm_address_t>(message->mDescriptor.address), message->mDescriptor.size);

			// Try and use "extended duration" timing if this frame and the stream lacks any "hard" discontinuities
			if (~kCMIOSampleBufferDiscontinuityFlag_DurationWasExtended & (message->mDiscontinuityFlags | GetDiscontinuityFlags()))
			{
				// The were hard discontinuities, so update the stream's discontinuity flags so the logical sum can be passed on with the next frame
				SetDiscontinuityFlags(GetDiscontinuityFlags() | message->mDiscontinuityFlags | kCMIOSampleBufferDiscontinuityFlag_DataWasDropped);
			}
			else
			{
				// Rather than mark a "hard" discontinuity, remember the frame's duration so the NEXT frame's duration can be extended accordingly
				if (0 == mExtendedDurationHostTime)
				{
					// This is the first extension needed, so use this frame's host & cycle time for the next frame
					mExtendedDurationHostTime = message->mHostTime;
					mExtendedDurationTimingInfo = message->mTimingInfo;
				}
				else
				{
					// An extension is already taking place, so simply add this frame's duration to the extended duration
					mExtendedDurationTimingInfo.duration = CMTimeAdd(mExtendedDurationTimingInfo.duration, message->mTimingInfo.duration);
				}
			}
			
			return;
		}
		
		// Use the "extended duration" timing if needed (indicated by a non-zero value for mExtendedDurationHostTime)
		if (0 != mExtendedDurationHostTime)
		{
			// Replace the frame's host with those from the "extended duration"
			message->mHostTime = mExtendedDurationHostTime;

			// Add the current frame's duration to the extended duration
			mExtendedDurationTimingInfo.duration = CMTimeAdd(mExtendedDurationTimingInfo.duration, message->mTimingInfo.duration);
			
			// Replace the frame's info with with the "extended duration" version
			message->mTimingInfo = mExtendedDurationTimingInfo;
	
			// Indicate this frame has an extended duration
			message->mDiscontinuityFlags |= kCMIOSampleBufferDiscontinuityFlag_DurationWasExtended;
		}

		// Set the discontinuity flag to be the logical sum of the stream's and the frame's
		SetDiscontinuityFlags(GetDiscontinuityFlags() | message->mDiscontinuityFlags);

		// Don't let any exceptions leave this routine
		try
		{
			// Resync the presentiontime time stamp if it is invalid or there have been any "hard" discontinuities
			if (CMTIME_IS_INVALID(mTimingInfo.presentationTimeStamp) or (~kCMIOSampleBufferDiscontinuityFlag_DurationWasExtended & GetDiscontinuityFlags()))
			{
				mSyncClock = (~kCMIOSampleBufferDiscontinuityFlag_DurationWasExtended & GetDiscontinuityFlags());
				
				// Set the presentation time to the converted value from the device's clock
				CMTime hosttimeCM = CMTimeConvertScale(message->mTimingInfo.presentationTimeStamp, 1000000000, kCMTimeRoundingMethod_Default);
				if (mSyncClock)
					mTimingInfo.presentationTimeStamp = hosttimeCM;
				else
					mTimingInfo.presentationTimeStamp = CMIOStreamClockConvertHostTimeToDeviceTime(CAHostTimeBase::ConvertFromNanos(hosttimeCM.value), mClock->GetClock());
				CMA::Time::ConformTimescale(mTimingInfo.presentationTimeStamp, message->mTimingInfo.duration.timescale);
				if (not CMTIME_IS_NUMERIC(mTimingInfo.presentationTimeStamp))
				{
					DebugMessage("CMIO::DP::Sample::Stream::FrameArrived: presentationTimeStamp is not numeric");
				}
				
				// Report its duration
				mTimingInfo.duration = message->mTimingInfo.duration;
			}
			else
			{
				// Bump the presentation time by the PREVIOUS frame's duration (since the presentation time of THIS immediately follows it since there were no discontinuities)
				mTimingInfo.presentationTimeStamp = CMTimeAdd(mTimingInfo.presentationTimeStamp, mTimingInfo.duration);
				CMA::Time::ConformTimescale(mTimingInfo.presentationTimeStamp, mTimingInfo.duration.timescale);

				// Report its duration
				mTimingInfo.duration = message->mTimingInfo.duration;
			}
			
			// Update recent timing info
			UInt32 prevTimingInfoIdx = mRecentTimingInfoIdx;
			mRecentTimingInfoIdx = (mRecentTimingInfoIdx + 1) & 0x00000001;
			
			mRecentTimingInfo[ mRecentTimingInfoIdx ].mPTS = mTimingInfo.presentationTimeStamp;
			mRecentTimingInfo[ mRecentTimingInfoIdx ].mHostTime = message->mHostTime;

			OSMemoryBarrier();
			mRecentTimingInfo[ mRecentTimingInfoIdx ].mValid = true;
			OSMemoryBarrier();
			mRecentTimingInfo[ prevTimingInfoIdx ].mValid = false;
			OSMemoryBarrier();
			
			// Drive the clock
			CMIOStreamClockPostTimingEvent(mTimingInfo.presentationTimeStamp, message->mHostTime, mSyncClock, mClock->GetClock());
			mSyncClock = false;			
			

			CMBlockBufferCustomBlockSource customBlockSource = { kCMBlockBufferCustomBlockSourceVersion, NULL, ReleaseBufferCallback, this };

			// Get the size & data for the frame
			size_t frameSize = message->mDescriptor.size;
			void* data = message->mDescriptor.address;

			DebugMessageLevel(2, "CMIO::DP::Sample::Stream::FrameArrived: Frametype = %d discontinuity = %d frameSize = %ld", message->mFrameType, GetDiscontinuityFlags(), frameSize);
			
			// Wrap the native frame in a block buffer.  kCFAllocatorNull will be used for the block allocator, so no memory will be deallocated when the block buffer goes out of scope.
			CMA::BlockBuffer blockBuffer(CMA::BlockBuffer::CreateWithMemoryBlock(NULL, data, frameSize, kCFAllocatorNull, &customBlockSource, 0, frameSize, 0));

			// Bump the sequence number
			mBufferSequenceNumber = CMIOGetNextSequenceNumber(mBufferSequenceNumber);

			// Wrap the block buffer in the CMIO variant of a CMSampleBufferRef.  (Buffers used by CMIO are CMSampleBuffers that have a required set of attachments.)
			Buffer buffer(Buffer::Create(NULL, blockBuffer, mFormatPairs[message->mFrameType].first, 1, 1, &mTimingInfo, 1, &frameSize, mBufferSequenceNumber, GetDiscontinuityFlags()));

			// Add the "host time" attachment to the buffer
			CMSetAttachment(buffer, kCMIOSampleBufferAttachmentKey_HostTime, CACFNumber(message->mHostTime).GetCFNumber(), kCMAttachmentMode_ShouldPropagate);
			
			// Add the "SMPTE time" attachment to the buffer
			{
				SMPTETime smpteTime;				
				smpteTime.mCounter	= mDeck->GetTimecode();
				smpteTime.mType		= kSMPTETimeType30;
				smpteTime.mFlags	= kSMPTETimeValid;
				
				SMPTETimeBase::CalculateSMPTE_HMSFsFromCounter(smpteTime, false);
				
				// Wrap the SMPTE time stamp and attach it to the buffer
				CACFData smpteData(CFDataCreate(NULL, reinterpret_cast<UInt8*>(&smpteTime), sizeof(smpteTime)));
				if (smpteData.IsValid())
					CMSetAttachment(buffer, kCMIOSampleBufferAttachmentKey_SMPTETime, smpteData.GetCFData(), kCMAttachmentMode_ShouldPropagate);
			}
			
			// Put the buffer at the back of the queue
			mBufferQueue.Enqueue(buffer);

			// The buffer was sucessfully enqueued, so make sure the CMIO::Buffer wrapper won't release the enqueued reference when it goes out of scope
			buffer.ShouldRelease(false);
			
			// Clear any accumulated discontinuity flags since the buffer was succesfully enqueued
			SetDiscontinuityFlags(kCMIOSampleBufferNoDiscontinuities);

			// Reset the extended duration host time to 0 to signify that no extension is needed
			mExtendedDurationHostTime = 0;

			// Inform the clients that the queue has been altered
			if (NULL != mQueueAlteredProc)
				(mQueueAlteredProc)(GetObjectID(), buffer, mQueueAlteredRefCon);
		}
		catch (...)
		{
			// Something went wrong, so deallocate the frame data
			vm_deallocate(mach_task_self(), reinterpret_cast<vm_address_t>(message->mDescriptor.address), message->mDescriptor.size);

			// Mark an unknown discontinuity
			SetDiscontinuityFlags(GetDiscontinuityFlags() | kCMIOSampleBufferDiscontinuityFlag_UnknownDiscontinuity);
		}
	}

	#pragma mark -
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// GetOutputBuffer()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void Stream::GetOutputBuffer(DPA::Sample::OutputBufferRequestedMessage* request) 
	{
        if (request->mHostTimeInNanos > 0)
        {
 			// Inform that we are going to play the buffer
			DebugMessageLevel(3, "DP::Sample::Stream::GetOutputBuffer SendNotification request->mHostTimeInNanos = %lld request->mSequenceNumber = %lld",request->mHostTimeInNanos , request->mLastSequenceNumber);
			mScheduledOutputNotificationProc->SendNotification(request->mLastSequenceNumber, CAHostTimeBase::ConvertFromNanos(request->mHostTimeInNanos));
        }
            
		// Setup the invariant portion of the reply message
		DPA::Sample::OutputBufferMessages reply;
		reply.asOutputBufferSuppliedMessage.mHeader.msgh_bits			= MACH_MSGH_BITS(MACH_MSGH_BITS_REMOTE(request->mHeader.msgh_bits), 0)  | MACH_MSGH_BITS_COMPLEX;
		reply.asOutputBufferSuppliedMessage.mHeader.msgh_remote_port	= request->mHeader.msgh_remote_port;
		reply.asOutputBufferSuppliedMessage.mHeader.msgh_local_port		= MACH_PORT_NULL;
		reply.asOutputBufferSuppliedMessage.mHeader.msgh_reserved		= 0;

		// Don't let any exceptions leave this callback
		try
		{
			// Make sure there are buffers available
			ThrowIf(0 == mBufferQueue.GetCount(), CAException(kCMIOHardwareUnspecifiedError), "CMIO::DP::Sample::Stream::GetOutputBuffer: 0 == mBufferQueue.GetCount()");
			
			// Get the next buffer to transmit from the queue
			Buffer buffer(reinterpret_cast<CMSampleBufferRef>(mBufferQueue.Dequeue()));
			
			// Inform the clients that the queue has been altered
			if (NULL != mQueueAlteredProc)
				(mQueueAlteredProc)(GetObjectID(), buffer, mQueueAlteredRefCon);

			// Get the underlying block buffer (no need to release it since it is maintained by 'buffer')
			CMA::BlockBuffer blockBuffer(buffer.GetDataBuffer(), false, false);
            if (blockBuffer.IsValid())
            {
                DebugMessage("DP::Sample::Stream::GetOutputBuffer Blockbuffer is valid");
               
                // Make sure the size array has only a single entry
                CMItemCount sizeArrayEntriesCount = 0;
                buffer.GetSampleSizeArray(0, NULL, &sizeArrayEntriesCount);
                ThrowIf(1 != sizeArrayEntriesCount, CAException(kCMIODeviceUnsupportedFormatError), "CMIO::DP::Sample::Stream::GetOutputBuffer: sizeArrayEntriesCount is not 1");
                
                // Make sure the data is ready
                bool isReady = buffer.DataIsReady();
                ThrowIf(not isReady, CAException(kCMIODeviceUnsupportedFormatError), "CMIO::DP::Sample::Stream::GetOutputBuffer: data is not ready");
                
                // Make sure the data length is not zero
                size_t dataLength = blockBuffer.GetDataLength();
                ThrowIf(0 == dataLength, CAException(kCMIODeviceUnsupportedFormatError), "CMIO::DP::Sample::Stream::GetOutputBuffer: data length is 0");
                
                // Make sure the range is contiguous
                bool isRangeContiguous = blockBuffer.IsRangeContiguous(0, dataLength);
                ThrowIf(not isRangeContiguous, CAException(kCMIODeviceUnsupportedFormatError), "CMIO::DP::Sample::Stream::GetOutputBuffer: range is not contiguous");
                
                CMTime presentationTimeStamp;
                presentationTimeStamp = CMSampleBufferGetPresentationTimeStamp(buffer.Get());
                if (CMTIME_IS_VALID(presentationTimeStamp))
                {
                    CMTime clockTime = DPA::Sample::CMTimeOverride(request->mClockTime);
                    if (CMTIME_IS_VALID(clockTime))
                    {
                        
                        DebugMessage("DP::Sample::Stream::DriveOutputClock hostNanos = %lld", request->mHostTimeInNanos);
                        DriveOutputClock(presentationTimeStamp, clockTime, request->mHostTimeInNanos);
                    }
                }
                else
                {
                    DebugMessage("DP::Sample::Stream::GetOutputBuffer invalid presentation timestamp");
                }
                
                // Look for audio
                CMSampleBufferRef audioSampleBuffer = reinterpret_cast<CMSampleBufferRef>(const_cast<void*>(CMGetAttachment(buffer.Get(), CFSTR(kCMIO_DPA_Sample_AudioSampleBuffer), NULL)));
                if (audioSampleBuffer)
                {
                    // At some point, we will do something with it...
                    // CFShow(audioSampleBuffer);
                }
                
                SMPTETime theSMPTETime;
                theSMPTETime.mFlags = 0;

                CFDataRef smpteDataRef = reinterpret_cast<CFDataRef>(const_cast<void *>(CMGetAttachment(buffer.Get(),kCMIOSampleBufferAttachmentKey_SMPTETime,NULL)));
                if ((NULL != smpteDataRef) && (sizeof(SMPTETime) == CFDataGetLength(smpteDataRef)))
                {
                    DebugMessage("DP::Sample::Stream::GetOutputBuffer found SMPTE attachement"); 
                    CFRange range = { 0, sizeof(SMPTETime) };
                    
                    CFDataGetBytes(smpteDataRef, range, reinterpret_cast<UInt8 *>(&(theSMPTETime)));                        
                }
                
                // Inform that we are going to play the buffer
                // DebugMessage("DP::Sample::Stream::GetOutputBuffer SendNotification request->mHostTimeInNanos = %lld request->mSequenceNumber = %lld", request->mHostTimeInNanos , request->mLastSequenceNumber);
                
                // Set the message ID to kOutputBufferSupplied to indicate data is being carried in the payload
                reply.asOutputBufferSuppliedMessage.mHeader.msgh_size			= sizeof(DPA::Sample::OutputBufferSuppliedMessage);
                reply.asOutputBufferSuppliedMessage.mHeader.msgh_id				= DPA::Sample::kOutputBufferSupplied;
                
                // Indicate one message descriptor will be needed to in this message to describe this buffer
                reply.asOutputBufferSuppliedMessage.mBody.msgh_descriptor_count	= 1;
                
                // Describe the buffer
                reply.asOutputBufferSuppliedMessage.mDescriptor.address			= blockBuffer.GetDataPointer(0, NULL, NULL);
                reply.asOutputBufferSuppliedMessage.mDescriptor.size			= dataLength;
                reply.asOutputBufferSuppliedMessage.mDescriptor.deallocate		= false;
                reply.asOutputBufferSuppliedMessage.mDescriptor.copy			= MACH_MSG_VIRTUAL_COPY;
                reply.asOutputBufferSuppliedMessage.mDescriptor.pad1			= 0;
                reply.asOutputBufferSuppliedMessage.mDescriptor.type			= MACH_MSG_OOL_DESCRIPTOR;
                reply.asOutputBufferSuppliedMessage.mSequenceNumber             = CMIOSampleBufferGetSequenceNumber(buffer.Get());
                reply.asOutputBufferSuppliedMessage.mDiscontinuityFlags         = CMIOSampleBufferGetDiscontinuityFlags(buffer.Get());
                reply.asOutputBufferSuppliedMessage.mSMPTETime                  = theSMPTETime;
                
                //            DebugMessage("DP::Sample::Stream::GetOutputBuffer reply.asOutputBufferSuppliedMessage.mSequenceNumber = %lld",reply.asOutputBufferSuppliedMessage.mSequenceNumber);
//                DebugMessage("DP::Sample::Stream::GetOutputBuffer reply.asOutputBufferSuppliedMessage.mSequenceNumber = %lld\n",reply.asOutputBufferSuppliedMessage.mSequenceNumber);
//                DebugMessage("DP::Sample::Stream::GetOutputBuffer reply.asOutputBufferSuppliedMessageSMPTETime.mCounter = %ld",reply.asOutputBufferSuppliedMessage.mSMPTETime.mCounter); 
//                DebugMessage("DP::Sample::Stream::GetOutputBuffer reply.asOutputBufferSuppliedMessageSMPTETime.mHours = %ld",reply.asOutputBufferSuppliedMessage.mSMPTETime.mHours); 
//                DebugMessage("DP::Sample::Stream::GetOutputBuffer reply.asOutputBufferSuppliedMessageSMPTETime.minutes = %ld",reply.asOutputBufferSuppliedMessage.mSMPTETime.mMinutes); 
//                DebugMessage("DP::Sample::Stream::GetOutputBuffer reply.asOutputBufferSuppliedMessageSMPTETime.mSeconds = %ld",reply.asOutputBufferSuppliedMessage.mSMPTETime.mSeconds); 
//                DebugMessage("DP::Sample::Stream::GetOutputBuffer reply.asOutputBufferSuppliedMessageSMPTETime.mFrames = %ld",reply.asOutputBufferSuppliedMessage.mSMPTETime.mFrames); 
                
                // Send the reply
                mach_msg_return_t err = mach_msg(&(reply.asOutputBufferSuppliedMessage.mHeader), MACH_SEND_MSG, reply.asOutputBufferSuppliedMessage.mHeader.msgh_size, 0, MACH_PORT_NULL, MACH_MSG_TIMEOUT_NONE, MACH_PORT_NULL);
                if (MACH_MSG_SUCCESS != err)
                {
                    DebugMessage("DP::Sample::Stream::GetOutputBuffer: mach_msg() failed - error: 0x%X (%s)", err, mach_error_string(err));
                }
            }
            else
            {
                DebugMessage("DP::Sample::Stream::GetOutputBuffer trying pixelBuffer");
                
                // Get the underlying pixel buffer (no need to release it since it is maintained by 'buffer')
                CVA::Pixel::Buffer pixelBuffer(buffer.GetImageBuffer(), false, false);
				ThrowIf(not pixelBuffer.IsValid(), CAException(kCMIOHardwareUnspecifiedError), "CMIO::DP::Sample::Stream::GetOutputBuffer: buffer is not valid");
                
				// Make sure the format is supported
                OSType format = CVPixelBufferGetPixelFormatType(pixelBuffer);
                ThrowIf((kCMPixelFormat_422YpCbCr8 != format) and (kCMPixelFormat_422YpCbCr10 != format), CAException(kCMIODeviceUnsupportedFormatError), "CMIO::DP::Sample::Stream::GetOutputBuffer: only kCMPixelFormat_422YpCbCr8 or kCMPixelFormat_422YpCbCr10 supported");
                
				// Drive the output clock
                CMTime presentationTimeStamp;
                presentationTimeStamp = CMSampleBufferGetPresentationTimeStamp(buffer.Get());
                if (CMTIME_IS_VALID(presentationTimeStamp))
                {
                    CMTime clockTime = DPA::Sample::CMTimeOverride(request->mClockTime);
                    if (CMTIME_IS_VALID(clockTime))
                    {
                        
                        DebugMessage("DP::Sample::Stream::DriveOutputClock hostNanos = %lld", request->mHostTimeInNanos);
                        DriveOutputClock(presentationTimeStamp, clockTime, request->mHostTimeInNanos);
                    }
                }
                else
                {
                    DebugMessage("DP::Sample::Stream::GetOutputBuffer invalid presentation timestamp");
                }
                
                // Look for audio
                CMSampleBufferRef audioSampleBuffer = reinterpret_cast<CMSampleBufferRef>(const_cast<void*>(CMGetAttachment(buffer.Get(), CFSTR(kCMIO_DPA_Sample_AudioSampleBuffer), NULL)));
                if (audioSampleBuffer)
                {
                    // At some point, we will do something with it...
                    // CFShow(audioSampleBuffer);
                }

                SMPTETime theSMPTETime;
                theSMPTETime.mFlags = 0;

                CFDataRef smpteDataRef = reinterpret_cast<CFDataRef>(const_cast<void *>(CMGetAttachment(buffer.Get(),kCMIOSampleBufferAttachmentKey_SMPTETime,NULL)));
                if ((NULL != smpteDataRef) && (sizeof(SMPTETime) == CFDataGetLength(smpteDataRef)))
                {
                    DebugMessage("DP::Sample::Stream::GetOutputBuffer found SMPTE attachement");
                    
                    CFRange range = { 0, sizeof(SMPTETime) };
                    
                    CFDataGetBytes(smpteDataRef, range, reinterpret_cast<UInt8 *>(&(theSMPTETime)));
                }
                   
				// Make sure the format is supported
                IOSurfaceRef backingSurface = CVPixelBufferGetIOSurface(pixelBuffer);
                if(backingSurface == NULL)
                {
                    // Lock the base address
                    pixelBuffer.LockBaseAddress(kCVPixelBufferLock_ReadOnly);
                    
                    
                    // Inform that we are going to play the buffer
                    //            DebugMessage("DP::Sample::Stream::GetOutputBuffer SendNotification request->mHostTimeInNanos = %lld request->mSequenceNumber = %lld",request->mHostTimeInNanos , request->mLastSequenceNumber);
                    
                    // Set the message ID to kOutputBufferSupplied to indicate data is being carried in the payload
                    reply.asOutputBufferSuppliedMessage.mHeader.msgh_size			= sizeof(DPA::Sample::OutputBufferSuppliedMessage);
                    reply.asOutputBufferSuppliedMessage.mHeader.msgh_id				= DPA::Sample::kOutputBufferSupplied;
                    
                    // Indicate one message descriptor will be needed to in this message to describe this buffer
                    reply.asOutputBufferSuppliedMessage.mBody.msgh_descriptor_count	= 1;
                    
                    // Describe the buffer
                    reply.asOutputBufferSuppliedMessage.mDescriptor.address			= CVPixelBufferGetBaseAddress(pixelBuffer);
                    reply.asOutputBufferSuppliedMessage.mDescriptor.size			= CVPixelBufferGetHeight(pixelBuffer) * CVPixelBufferGetBytesPerRow(pixelBuffer);
                    reply.asOutputBufferSuppliedMessage.mDescriptor.deallocate		= false;
                    reply.asOutputBufferSuppliedMessage.mDescriptor.copy			= MACH_MSG_VIRTUAL_COPY;
                    reply.asOutputBufferSuppliedMessage.mDescriptor.pad1			= 0;
                    reply.asOutputBufferSuppliedMessage.mDescriptor.type			= MACH_MSG_OOL_DESCRIPTOR;
                    reply.asOutputBufferSuppliedMessage.mSequenceNumber             = CMIOSampleBufferGetSequenceNumber(buffer.Get());
                    reply.asOutputBufferSuppliedMessage.mDiscontinuityFlags         = CMIOSampleBufferGetDiscontinuityFlags(buffer.Get());
                    reply.asOutputBufferSuppliedMessage.mSMPTETime                  = theSMPTETime;
                    
                    
                    DebugMessage("DP::Sample::Stream::GetOutputBuffer reply.asOutputBufferSuppliedMessage.mSequenceNumber = %lld disconinuity = %x", reply.asOutputBufferSuppliedMessage.mSequenceNumber, reply.asOutputBufferSuppliedMessage.mDiscontinuityFlags);
                    
                    //            DebugMessage("DP::Sample::Stream::GetOutputBuffer reply.asOutputBufferSuppliedMessage.mSequenceNumber = %lld",reply.asOutputBufferSuppliedMessage.mSequenceNumber);
                    //               DebugMessage("DP::Sample::Stream::GetOutputBuffer reply.asOutputBufferSuppliedMessage.mSequenceNumber = %lld disconinuity = %x",reply.asOutputBufferSuppliedMessage.mSequenceNumber,reply.asOutputBufferSuppliedMessage.mDiscontinuityFlags);
                    //                DebugMessage("DP::Sample::Stream::GetOutputBuffer reply.asOutputBufferSuppliedMessage.mSequenceNumber = %lld\n",reply.asOutputBufferSuppliedMessage.mSequenceNumber);
                    //                DebugMessage("DP::Sample::Stream::GetOutputBuffer reply.asOutputBufferSuppliedMessageSMPTETime.mCounter = %ld",reply.asOutputBufferSuppliedMessage.mSMPTETime.mCounter);
                    //                DebugMessage("DP::Sample::Stream::GetOutputBuffer reply.asOutputBufferSuppliedMessageSMPTETime.mHours = %ld",reply.asOutputBufferSuppliedMessage.mSMPTETime.mHours);
                    //                DebugMessage("DP::Sample::Stream::GetOutputBuffer reply.asOutputBufferSuppliedMessageSMPTETime.minutes = %ld",reply.asOutputBufferSuppliedMessage.mSMPTETime.mMinutes);
                    //                DebugMessage("DP::Sample::Stream::GetOutputBuffer reply.asOutputBufferSuppliedMessageSMPTETime.mSeconds = %ld",reply.asOutputBufferSuppliedMessage.mSMPTETime.mSeconds);
                    //                DebugMessage("DP::Sample::Stream::GetOutputBuffer reply.asOutputBufferSuppliedMessageSMPTETime.mFrames = %ld",reply.asOutputBufferSuppliedMessage.mSMPTETime.mFrames);
                    
                    // Send the reply
                    mach_msg_return_t err = mach_msg(&(reply.asOutputBufferSuppliedMessage.mHeader), MACH_SEND_MSG, reply.asOutputBufferSuppliedMessage.mHeader.msgh_size, 0, MACH_PORT_NULL, MACH_MSG_TIMEOUT_NONE, MACH_PORT_NULL);
                    if (MACH_MSG_SUCCESS != err)
                    {
                        DebugMessage("DP::Sample::Stream::GetOutputBuffer: mach_msg() failed - error: 0x%X (%s)", err, mach_error_string(err));
                    }
                    
                    // Unlock the base address
                    (void) CVPixelBufferUnlockBaseAddress(pixelBuffer, kCVPixelBufferLock_ReadOnly);
                }
                else
                {
                    DebugMessage("DP::Sample::Stream::GetOutputBuffer found IOSurface");
                    
                    // Inform that we are going to play the buffer
                    //            DebugMessage("DP::Sample::Stream::GetOutputBuffer SendNotification request->mHostTimeInNanos = %lld request->mSequenceNumber = %lld",request->mHostTimeInNanos , request->mLastSequenceNumber);
                    
                    // Set the message ID to kOutputBufferSupplied to indicate data is being carried in the payload
                    reply.asOutputSurfaceSuppliedMessage.mHeader.msgh_size			= sizeof(DPA::Sample::OutputSurfaceSuppliedMessage);
                    reply.asOutputSurfaceSuppliedMessage.mHeader.msgh_id			= DPA::Sample::kOutputSurfaceSupplied;
                    
                    // Indicate one message descriptor will be needed to in this message to describe this buffer
                    reply.asOutputBufferSuppliedMessage.mBody.msgh_descriptor_count	= 1;
                    
                    // Describe the buffer
                    reply.asOutputSurfaceSuppliedMessage.mDescriptor.name           = IOSurfaceCreateMachPort(CVPixelBufferGetIOSurface(pixelBuffer.Get()));
                    reply.asOutputSurfaceSuppliedMessage.mDescriptor.disposition	= MACH_MSG_TYPE_MOVE_SEND;
                    reply.asOutputSurfaceSuppliedMessage.mDescriptor.type           = MACH_MSG_PORT_DESCRIPTOR;

                    
                    reply.asOutputSurfaceSuppliedMessage.mSequenceNumber             = CMIOSampleBufferGetSequenceNumber(buffer.Get());
                    reply.asOutputSurfaceSuppliedMessage.mDiscontinuityFlags         = CMIOSampleBufferGetDiscontinuityFlags(buffer.Get());
                    reply.asOutputSurfaceSuppliedMessage.mSMPTETime                  = theSMPTETime;
                    
                    
                    DebugMessage("DP::Sample::Stream::GetOutputBuffer reply.asOutputSurfaceSuppliedMessage.mSequenceNumber = %lld disconinuity = %x", reply.asOutputSurfaceSuppliedMessage.mSequenceNumber, reply.asOutputSurfaceSuppliedMessage.mDiscontinuityFlags);
                    
                    //            DebugMessage("DP::Sample::Stream::GetOutputBuffer reply.asOutputSurfaceSuppliedMessage.mSequenceNumber = %lld",reply.asOutputSurfaceSuppliedMessage.mSequenceNumber);
                    //               DebugMessage("DP::Sample::Stream::GetOutputBuffer reply.asOutputSurfaceSuppliedMessage.mSequenceNumber = %lld disconinuity = %x",reply.asOutputSurfaceSuppliedMessage.mSequenceNumber,reply.asOutputSurfaceSuppliedMessage.mDiscontinuityFlags);
                    //                DebugMessage("DP::Sample::Stream::GetOutputBuffer reply.asOutputSurfaceSuppliedMessage.mSequenceNumber = %lld\n",reply.asOutputSurfaceSuppliedMessage.mSequenceNumber);
                    //                DebugMessage("DP::Sample::Stream::GetOutputBuffer reply.asOutputSurfaceSuppliedMessageSMPTETime.mCounter = %ld",reply.asOutputSurfaceSuppliedMessage.mSMPTETime.mCounter);
                    //                DebugMessage("DP::Sample::Stream::GetOutputBuffer reply.asOutputSurfaceSuppliedMessageSMPTETime.mHours = %ld",reply.asOutputSurfaceSuppliedMessage.mSMPTETime.mHours);
                    //                DebugMessage("DP::Sample::Stream::GetOutputBuffer reply.asOutputSurfaceSuppliedMessageSMPTETime.minutes = %ld",reply.asOutputSurfaceSuppliedMessage.mSMPTETime.mMinutes);
                    //                DebugMessage("DP::Sample::Stream::GetOutputBuffer reply.asOutputSurfaceSuppliedMessageSMPTETime.mSeconds = %ld",reply.asOutputSurfaceSuppliedMessage.mSMPTETime.mSeconds);
                    //                DebugMessage("DP::Sample::Stream::GetOutputBuffer reply.asOutputSurfaceSuppliedMessageSMPTETime.mFrames = %ld",reply.asOutputSurfaceSuppliedMessage.mSMPTETime.mFrames);
                    
                    // Send the reply
                    mach_msg_return_t err = mach_msg(&(reply.asOutputSurfaceSuppliedMessage.mHeader), MACH_SEND_MSG, reply.asOutputSurfaceSuppliedMessage.mHeader.msgh_size, 0, MACH_PORT_NULL, MACH_MSG_TIMEOUT_NONE, MACH_PORT_NULL);
                    if (MACH_MSG_SUCCESS != err)
                    {
                        DebugMessage("DP::Sample::Stream::GetOutputBuffer: mach_msg() failed - error: 0x%X (%s)", err, mach_error_string(err));
                    }
                }
            }
		}
		catch (...)
		{
			// There are no buffers, so no descriptiors will be needed in the message

			// Set the message ID to kNoOutputBufferSupplied to indicate there are NO buffers in the payload
			reply.asNoOutputBufferSuppliedMessage.mHeader.msgh_size			= sizeof(DPA::Sample::NoOutputBufferSuppliedMessage);
			reply.asNoOutputBufferSuppliedMessage.mHeader.msgh_id			= DPA::Sample::kNoOutputBufferSupplied;
			
			// Send the reply
			mach_msg_return_t err = mach_msg(&(reply.asNoOutputBufferSuppliedMessage.mHeader), MACH_SEND_MSG, reply.asNoOutputBufferSuppliedMessage.mHeader.msgh_size, 0, MACH_PORT_NULL, MACH_MSG_TIMEOUT_NONE, MACH_PORT_NULL);
			if (MACH_MSG_SUCCESS != err)
			{
				DebugMessage("DP::Sample::Stream::GetOutputBuffer: mach_msg() failed - error: 0x%X (%s)", err, mach_error_string(err));
			}
		}
	}	

	#pragma mark -
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// DeckPlay()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void Stream::DeckPlay()
	{
		DPA::Sample::DeckPlay(GetOwningDevice().GetAssistantPort(), GetOwningDevice().GetDeviceGUID(), GetDevicePropertyScope(), GetStartingDeviceChannelNumber());
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// DeckStop()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void Stream::DeckStop()
	{
		DPA::Sample::DeckStop(GetOwningDevice().GetAssistantPort(), GetOwningDevice().GetDeviceGUID(), GetDevicePropertyScope(), GetStartingDeviceChannelNumber());
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// DeckJog()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void Stream::DeckJog(SInt32 speed)
	{
		DPA::Sample::DeckJog(GetOwningDevice().GetAssistantPort(), GetOwningDevice().GetDeviceGUID(), GetDevicePropertyScope(), GetStartingDeviceChannelNumber(), speed);
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// DeckCueTo()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void Stream::DeckCueTo(Float64 requestedTimecode, Boolean	playOnCue)
	{
		DPA::Sample::DeckCueTo(GetOwningDevice().GetAssistantPort(), GetOwningDevice().GetDeviceGUID(), GetDevicePropertyScope(), GetStartingDeviceChannelNumber(), requestedTimecode, playOnCue);
	}

	#pragma mark -
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// StreamDeckOneShot()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	CMIOStreamDeck Stream::StreamDeckOneShot(DP::Stream& stream)
	{
		Stream& sampleStream = static_cast<Stream&>(stream);
		return DPA::Sample::GetDeck(sampleStream.GetOwningDevice().GetAssistantPort(), sampleStream.GetOwningDevice().GetDeviceGUID(), sampleStream.GetDevicePropertyScope(), sampleStream.GetStartingDeviceChannelNumber()); 
	}
	
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// TimeCodeOneShot()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	Float64 Stream::TimeCodeOneShot(DP::Stream& stream)
	{
		Stream& sampleStream = static_cast<Stream&>(stream);
		return DPA::Sample::GetDeckTimecode(sampleStream.GetOwningDevice().GetAssistantPort(), sampleStream.GetOwningDevice().GetDeviceGUID(), sampleStream.GetDevicePropertyScope(), sampleStream.GetStartingDeviceChannelNumber()); 
	}
	
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// DropnessOneShot()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	UInt32 Stream::DropnessOneShot(DP::Stream& stream)
	{
		return 1;
		//Stream& avcStream = static_cast<Stream&>(stream);
		//return DPA::AVC::GetDeckThreaded(avcStream.GetOwningDevice().GetAssistantPort(), avcStream.GetOwningDevice().GetDeviceGUID(), avcStream.GetDevicePropertyScope(), 1); 
	}
	
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// ThreadedOneShot()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	UInt32 Stream::ThreadedOneShot(DP::Stream& stream)
	{
		return 1;
		// Stream& sampleStream = static_cast<Stream&>(stream);
		// return DPA::AVC::GetDeckThreaded(sampleStream.GetOwningDevice().GetAssistantPort(), sampleStream.GetOwningDevice().GetDeviceGUID(), sampleStream.GetDevicePropertyScope(), 1); 
	}
	
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// LocalOneShot()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	UInt32 Stream::LocalOneShot(DP::Stream& /*stream*/)
	{
		return 1;
		//Stream& sampleStream = static_cast<Stream&>(stream);
		//return DPA::AVC::GetDeckLocal(sampleStream.GetOwningDevice().GetAssistantPort(), sampleStream.GetOwningDevice().GetDeviceGUID(), sampleStream.GetDevicePropertyScope(), 1); 
	}
	
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// CueingOneShot()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	SInt32 Stream::CueingOneShot(DP::Stream& stream)
	{
		Stream& sampleStream = static_cast<Stream&>(stream);
		return DPA::Sample::GetDeckCueing(sampleStream.GetOwningDevice().GetAssistantPort(), sampleStream.GetOwningDevice().GetDeviceGUID(), sampleStream.GetDevicePropertyScope(), sampleStream.GetStartingDeviceChannelNumber()); 
	}
	
	#pragma mark -
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// CueComplete()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void Stream::CueComplete(SInt32 cueStatus)
	{
		mDeck->SetCueing(cueStatus);
	}
	
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// TimecodeChanged()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void Stream::TimecodeChanged(Float64 timecode)
	{
		mDeck->SetTimecode(timecode);
	}
	
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// StreamDeckChanged()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void Stream::StreamDeckChanged(UInt32 changed, UInt16 opcode, UInt16 operand)
	{
        DebugMessage("CMIO::DP::Sample::Stream::StreamDeckChanged: changed = %d opcode = %d operand = %d", changed, opcode, operand);
		
        CMIOStreamDeck streamDeck;
		
		streamDeck.mStatus = changed;
		streamDeck.mState = opcode;
		streamDeck.mState2 = operand;
		
		mDeck->SetStreamDeck(streamDeck);
	}
}}}
