/*
	    File: CMIO_DP_Sample_Stream.h
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

#if !defined(__CMIO_DP_Sample_Stream_h__)
#define __CMIO_DP_Sample_Stream_h__

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Includes
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

// Super Class Includes
#include "CMIO_DP_Stream.h"

// Internal Includes
#include "CMIO_DPA_Sample_Shared.h"

// Public Utility Includes
#include "CMIO_CMA_SampleBuffer.h"
#include "CMIO_CMA_SimpleQueue.h"
#include "CMIO_PTA_CFMachPortThread.h"

// CA Public Utility Includes
#include "CACFData.h"
#include "CACFString.h"
#include "CAPThread.h"
#include "CAGuard.h"

// System Includes
#include <CoreMedia/CMSampleBuffer.h>

namespace CMIO { namespace DP { namespace Property
{
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// Types in the CMIO::DP::Property namespace
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	class EndOfData;
	class NoData;
	class OutputBuffers;
	class FirstOutputPresentationTimeStamp;
	class ScheduledOutputNotificationProc;
	class Clock;
	class Deck;
}}}

namespace CMIO { namespace DP { namespace Sample
{
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// Types in the CMIO::DP::Sample namespace
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	class PlugIn;
	class Device;
	class Deck;
}}}

namespace CMIO { namespace DP { namespace Sample
{
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// Stream
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	class Stream : public DP::Stream
	{
	private:
		//-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
		// Types
		//-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
		typedef std::pair<CMFormatDescriptionRef, DPA::Sample::FrameFormat> FormatPair;
		typedef std::map<DPA::Sample::FrameType, FormatPair> FormatPairMap;
		
		//-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
		// Stream:::FormatDescriptionEquals
		//	A unary predicate object which reports if the FormatPair.second.first equals the specified CMFormatDescriptionRef.
		//-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
		class FormatDescriptionEquals
		{
		public:
					FormatDescriptionEquals(CMFormatDescriptionRef formatDescription) : mFormatDescription(formatDescription) {};
			bool	operator()(const std::pair<DPA::Sample::FrameType, FormatPair>& pair) const { return CMFormatDescriptionEqual(pair.second.first, mFormatDescription); }

		private:
			CMFormatDescriptionRef	mFormatDescription;
		};
		
		//-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
		// Stream:::RecentTimingInfo
		//	A struct that holds recent timing info
		//-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
		struct RecentTimingInfoStruct
		{
			CMTime	mPTS;
			UInt64	mHostTime;
			bool	mValid;
		};
		typedef struct RecentTimingInfoStruct RecentTimingInfo;
		
	#pragma mark - Stream
	// Construction/Destruction
	public:
												Stream(CMIOStreamID streamID, PlugIn& plugIn, Device& owningDevice, CMIOObjectPropertyScope scope, UInt32 startingDeviceChannelNumber);
		virtual									~Stream();
	
		virtual void							Initialize();
		virtual void							Teardown();
		virtual void							Finalize();
	
	// Attributes
	public:
		Device&									GetOwningDevice() const { return reinterpret_cast<Device&>(mOwningDevice); }
		virtual CFStringRef						CopyStreamName() const;
		virtual UInt32							GetTerminalType() const;
		virtual void							SetNoDataTimeout(UInt32 noDataTimeout);
		virtual void							SetDeviceSyncTimeout(UInt32 deviceSyncTimeout);
		virtual void							SetEndOfData(UInt32 endOfData);
        virtual UInt32                          GetLatency() const;
        UInt32                                  GetMinimumInFlightFramesForThrottledPlayback() const;
	
	protected:
		void									SetDiscontinuityFlags(UInt32 discontinuityFlags) { mDiscontinuityFlags = discontinuityFlags; }
		UInt32									GetDiscontinuityFlags() const { return mDiscontinuityFlags; }

		CACFString								mStreamName;
		UInt32									mDiscontinuityFlags;
		UInt64									mExtendedDurationHostTime;
		CMA::SampleBuffer::TimingInfo			mExtendedDurationTimingInfo;

	// Property Access
	public:
		virtual bool							HasProperty(const CMIOObjectPropertyAddress& address) const;
		virtual bool							IsPropertySettable(const CMIOObjectPropertyAddress& address) const;
		virtual UInt32							GetPropertyDataSize(const CMIOObjectPropertyAddress& address, UInt32 qualifierDataSize, const void* qualifierData) const;
		virtual void							GetPropertyData(const CMIOObjectPropertyAddress& address, UInt32 qualifierDataSize, const void* qualifierData, UInt32 dataSize, UInt32& dataUsed, void* data) const;
		virtual void							SetPropertyData(const CMIOObjectPropertyAddress& address, UInt32 qualifierDataSize, const void* qualifierData, UInt32 dataSize, const void* data);
	
		void									UpdatePropertyState(const PropertyAddress& address, bool sendChangeNotifications = true);

	// Properties
	protected:
		DP::Property::NoData*					mNoData;			// Only used for input streams 
		DP::Property::OutputBuffers*			mOutputBuffers;		// Only used for output streams
		DP::Property::EndOfData*				mEndOfData;			// Only used for output streams
		DP::Property::FirstOutputPresentationTimeStamp*	mFirstOutputPresentationTimeStamp;  // Only used for output streams 
		DP::Property::ScheduledOutputNotificationProc* mScheduledOutputNotificationProc;	// Only used for output streams
		DP::Property::Deck*						mDeck;				// Manages deck properties

	// Format Management
	public:
		virtual bool							TellHardwareToSetFormatDescription(CMFormatDescriptionRef format);
		virtual void							RefreshAvailableFormatDescriptions();
		virtual void							AddAvailableFormatDescriptions();

	protected:
		FormatPairMap							mFormatPairs;
		DPA::Sample::FrameType					mFrameType;				// The current DPA::Sample::FrameType being generated 

	// Frame rate
    public:
		virtual bool							TellHardwareToSetFrameRate(Float64 frameRate);
		
	// IO Management
	public:
		virtual void							Start();
		virtual void							Stop();
		virtual void							Unplug();
		virtual CMSimpleQueueRef				CopyBufferQueue(CMIODeviceStreamQueueAlteredProc queueAlteredProc, void* queueAlteredRefCon);

		static void								Messages(CFMachPortRef port, mach_msg_header_t* header, CFIndex size, Stream& stream);
		void									FrameArrived(DPA::Sample::FrameArrivedMessage* message);
//		void									DriveClock(UInt64 hostTime, const CMTime& duration);
		static void								ReleaseBufferCallback(void* refCon, void *doomedMemoryBlock, size_t sizeInBytes);

		void									GetOutputBuffer(DPA::Sample::OutputBufferRequestedMessage* message);

		void									DriveOutputClock(CMTime presentationTimeStamp, CMTime clockTime, UInt64 nanosecondsHostTime);

	protected:
		virtual void							PropertyListenerAdded(const CMIOObjectPropertyAddress& address);
		virtual void							PropertyListenerRemoved(const CMIOObjectPropertyAddress& address);

	// Deck Control
	public:
		virtual void							DeckPlay();
		virtual void							DeckStop();
		virtual void							DeckJog(SInt32 speed);
		virtual void							DeckCueTo(Float64 requestedTimecode, Boolean playOnCue);

		static CMIOStreamDeck					StreamDeckOneShot(DP::Stream& stream);
		static Float64							TimeCodeOneShot(DP::Stream& stream);
		static UInt32							DropnessOneShot(DP::Stream& stream);
		static UInt32							ThreadedOneShot(DP::Stream& stream);
		static UInt32							LocalOneShot(DP::Stream& stream);
		static SInt32							CueingOneShot(DP::Stream& stream);
		
		void									CueComplete(SInt32 cueStatus);
		void									TimecodeChanged(Float64 timecode);
		void									StreamDeckChanged(UInt32 changed, UInt16 opcode, UInt16 operand);

	protected:
		PropertyAddressList						mDeckPropertyListeners;

		
	protected:
		PTA::CFMachPortThread					mMessageThread;
		CMA::SimpleQueue<CMSampleBufferRef>		mBufferQueue;
		CMIODeviceStreamQueueAlteredProc		mQueueAlteredProc;
		void*									mQueueAlteredRefCon;
		UInt64									mBufferSequenceNumber;
		CMA::SampleBuffer::TimingInfo			mTimingInfo;
		bool									mSuspended;
		SMPTETime								mVideoSMPTETime;


	// Implementation
	protected:
	// Input		
		UInt64									mDeferredNoDataBufferSequenceNumber;
		UInt32									mDeferredNoDataBufferEvent;

	private:
		// Clock support
		UInt64									mOutputHosttimeCorrection;
		UInt32									mPreviousCycleTimeSeconds;
		bool									mSyncClock;
		CMTime									mClockTime;
		RecentTimingInfo						mRecentTimingInfo[2];
		UInt32									mRecentTimingInfoIdx;
		
	};
}}}

#endif
