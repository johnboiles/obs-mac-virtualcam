/*
	    File: CMIO_DPA_Sample_Shared.h
	Abstract: Items in the CMIO::DPA::Sample namespace shared between the Assistant (the server) and its clients.
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

#if !defined(__CMIO_DPA_Sample_Shared_h__)
#define __CMIO_DPA_Sample_Shared_h__

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Includes
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

// Internal Includes
//#include "CMIO_DPA_Sample_Registers.h"

// Public Utility Includes
#include "CMIO_PropertyAddress.h"
#include "CMIO_CMA_SampleBuffer.h"

// System Includes
#include <CoreFoundation/CFBase.h>
#include <CoreMedia/CMFormatDescription.h>
#include <CoreMediaIO/CMIOHardware.h>
#include <mach/port.h>
#include <mach/message.h>
#include <sys/types.h>			// for pid_t

#pragma pack(push, 4)

/*!
 @enum		CodecFlags.
 @discussion .
 */
enum
{
	kSampleCodecNoFlags                                 = 0,			/*! @constant kSampleCodecNoFlags							no rates supported. */
	
	kSampleCodecFlags_24fps                             = (1L << 0),	/*! @constant kSampleCodecFlags_24fps		0x00000001  framerate 24fps. */
	kSampleCodecFlags_25fps                             = (1L << 1),	/*! @constant kSampleCodecFlags_25fps       0x00000002  framerate 25fps. */
	kSampleCodecFlags_30fps                             = (1L << 2),	/*! @constant kSampleCodecFlags_30fps		0x00000004  framerate 30fps. */
	kSampleCodecFlags_50fps                             = (1L << 3),	/*! @constant kSampleCodecFlags_50fps		0x00000008  framerate 50fps. */
	kSampleCodecFlags_60fps                             = (1L << 4),	/*! @constant kSampleCodecFlags_60fps		0x00000010  framerate 60fps. */

	kSampleCodecFlags_1001_1000_adjust                  = (1L << 16),	/*! @constant kSampleCodecFlags_1001_1000_adjust	0x00010000  multiply framerate by 1000/1001. */
    kSampleCodecFlags_psf_frame                         = (1L << 17)    /*! @constant kSampleCodecFlags_psf_frame			0x00020000  frame is psf. */
};


// Strings that we use for keys, etc. that didn't quite make it public yet.
#define kCMIO_DPA_Sample_VideoOnlyHasIFrames	"com.apple.cmio.format_extension.video.only_has_i_frames"
#define kCMIO_DPA_Sample_AudioSampleBuffer		"com.apple.cmio.buffer_attachment.audio_sample_buffer"

struct SampleVideoDeviceControlBuffer
{
	UInt64	vbiTime;
	UInt64	outputTime;
	UInt64	totalFrameCount;
	UInt64	droppedFrameCount;
	UInt64	firstVBITime;
    UInt64  sequenceNumber;
    UInt32  discontinuityFlags;
    SMPTETime smpteTime;
};

namespace CMIO
{
	typedef PropertyAddress* PropertyAddressPtr;			// A typedef'd pointer for use by MIG
}

namespace CMIO { namespace DPA { namespace Sample
{

	#pragma mark Constants

	typedef UInt8 ByteArray512[512];						// Up to 512 bytes

	#pragma mark CMTime Overrides
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	//	CoreMedia & CoreMediaPrivate have a different packing of their CMTime structures, so having a single CoreMedia based Assistant feeding CoreMedia & CoreMediaPrivate based clients
	//	requires an invariant form of CMTime to be used.
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	struct CMTimeOverrideSimple
	{
		CMTimeValue		value;
		CMTimeScale		timescale;
		CMTimeFlags		flags;
		CMTimeEpoch		epoch;
	};
	typedef struct CMTimeOverrideSimple CMTimeOverrideSimple;
	
	struct CMTimeOverride : public CMTimeOverrideSimple
	{
		CMTimeOverride() : CMTimeOverrideSimple() { value = kCMTimeInvalid.value; timescale = kCMTimeInvalid.timescale; flags = kCMTimeInvalid.flags; epoch = kCMTimeInvalid.epoch; }
		CMTimeOverride(const CMTime& time) : CMTimeOverrideSimple() { value = time.value; timescale = time.timescale; flags= time.flags; epoch = time.epoch; }
		CMTimeOverride(const CMTimeOverrideSimple& time) : CMTimeOverrideSimple(time) {}
		CMTimeOverride&	operator=(const CMTime& time) { this->value = time.value; this->timescale = time.timescale; this->flags = time.flags; this->epoch = time.epoch; return *this; }
		operator			CMTime() const { CMTime time; time.value = this->value, time.timescale = this->timescale, time.flags = this->flags; time.epoch = this->epoch; return time;}
	};
	
	#pragma mark FrameType
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// FrameType
	//	Arbitrary constants representing the types of frames the device can generate.
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	enum FrameType
	{
		kYUV422_720x480,
        kYUV422_720x486,
        kYUV422_720x576,
		kYUV422_1280x720,
		kYUV422_1920x1080,
        kYUV422_10_720x480,
        kYUV422_10_720x486,
        kYUV422_10_720x576,
        kYUV422_10_1280x720,
        kYUV422_10_1920x1080,
		kFrameTypePad = 0xFFFFFFFFUL
	};

	#pragma mark FrameFormat
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// FrameFormat
	//	Details enough information about each frame type so the DAL PlugIn can create a CMFormatDescriptionRef to describe it.
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	struct FrameFormat
	{
		//-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
		// FrameFormat::Equal
		//	A unary predicate object which reports if the FrameFormat's frame type equals the specified frame type.
		//-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
		class Equal
		{
		public:
					Equal(FrameType frameType) : mFrameType(frameType) {};
			bool	operator()(const FrameFormat& frameFormat) const { return frameFormat.mFrameType == mFrameType; }
		
		private:
					UInt64 mFrameType;
		};

		//-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
		// FrameFormat::Less
		//	A function object to determine if one frame format is "less" than another.
		//-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
		class Less
		{
		public:
			inline bool operator()(const FrameFormat& x, const FrameFormat& y) const
			{
				// If the codec types are equal, then "less" is determined by the width and height
				if (x.mCodecType == y.mCodecType)
				{
					if (x.mWidth == y.mWidth)
						return x.mHeight < y.mHeight;
						
					return x.mWidth < y.mWidth;
				}
				
				// The codec types are NOT equal, so  have "less" be determined by the sorting of the codec type
				return x.mCodecType < y.mCodecType;
			}
		};
		
		//	Construction/Destruction
		FrameFormat() { memset(this, 0, sizeof(FrameFormat)); }
		FrameFormat(FrameType frameType, CMVideoCodecType codecType, SInt32 width, SInt32 height) { mFrameType = frameType; mCodecType = codecType; mWidth = width; mHeight = height; }
		FrameFormat(const FrameFormat& v) { memcpy(this, &v, sizeof(FrameFormat)); }

		FrameType			mFrameType;
		CMVideoCodecType	mCodecType;
		SInt32				mWidth;
		SInt32				mHeight;
	};
	typedef FrameFormat* FrameFormatPtr;				// A typedef'd pointer for use by MIG

	#pragma mark DeviceState
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// DeviceState
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	struct DeviceState
	{
		UInt64		mGUID;								// The GUID of the device
		io_string_t	mRegistryPath;						// The registry path for the device 
	};
	typedef DeviceState* DeviceStatePtr;				// A typedef'd pointer for use by MIG


	#pragma mark ControlChanges
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// ControlChanges
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	struct ControlChanges
	{
						ControlChanges(UInt32 controlID, bool valueChanged, bool rangeChanged) : mControlID(controlID), mValueChanged(valueChanged), mRangeChanged(rangeChanged) {}
		UInt32			mControlID;						// The kIOVideoControlKey_ControlID
		UInt32			mValueChanged;					// true if the value has changed, false otherwise
		UInt32			mRangeChanged;					// true if the range has changed, false otherwise
	};
	typedef ControlChanges* ControlChangesPtr;			// A typedef'd pointer for use by MIG

	#pragma mark MessageIDs
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// MessageID
	//  These are the messages that the Assistant will send to the clients
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	enum MessageID
	{
		kDeviceStatesChanged,							// Devices have been added or removed
		kControlStatesChanged,							// Controls have changed
		kPropertyStatesChanged,							// Properties have changed
		kFrameArrived,									// A video frame arrived
		kOutputBufferRequested,							// An output buffer is needed for the device to transmit
		kOutputBufferSupplied,							// An output buffer is supplied 
		kOutputSurfaceSupplied,							// An output surface is supplied
		kNoOutputBufferSupplied							// No output buffer is avaialble
	};

	#pragma mark Messages
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// Messages
	//  These are the messages that the Assistant will send to the clients
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

	//
	// DeviceStatesChangedMessage
	//	This will be sent (and received by the DP::Sample::PlugIn object) when devices have been added / removed
	struct DeviceStatesChangedMessage
	{
		mach_msg_header_t	mHeader;				// Standard Mach message header
	};

	//
	// ControlStatesChangedMessage
	//	This will be sent (and received by a DP::Sample::Device object) when a control has changed state
	struct ControlStatesChangedMessage
	{
		mach_msg_header_t	mHeader;				// Standard Mach message header
	};

	//
	// PropertyStatesChangedMessage
	//	This will be sent (and received by a DP::Sample::Device object) when a property has changed state
	struct PropertyStatesChangedMessage
	{
		mach_msg_header_t	mHeader;				// Standard Mach message header
	};

	//
	// FrameArrivedMessage
	//	This will be sent (and received by a DP::Sample::Stream object) when a new frame has arrived
	struct FrameArrivedMessage
	{
		mach_msg_header_t				mHeader;				// Standard Mach message header
		mach_msg_body_t					mBody;					// Message requires 1 out-of-line (OOL) descriptor for frame data
		mach_msg_ool_descriptor_t		mDescriptor;			// Out-of-Line (OOL) descriptor for the frame's data
		FrameType						mFrameType;				// The FrameType contained in the message
		UInt64							mHostTime;				// Host time for frame
		CMA::SampleBuffer::TimingInfo	mTimingInfo;			// Samplebuffer timing info for frame
		UInt32							mDiscontinuityFlags;	// Discontinuities associated with the frame
		UInt32							mDroppedFrameCount;		// Dropped frame count
		UInt64							mFirstFrameTime;		// Host time we wanted device to start at
	};

	//
	// OutputBufferRequestedMessage
	//	This will be sent (and received by a DP::Sample::Stream object) when an output buffer is needed
	struct OutputBufferRequestedMessage
	{
		mach_msg_header_t				mHeader;				// Standard Mach message header
		CMTimeOverrideSimple			mClockTime;				// Time of device clock for this buffer
		UInt64							mHostTimeInNanos;		// Hosttime (in nanoseconds) to associate with device clock time
        UInt64                          mLastSequenceNumber;	// Sequence number returned that carried over from the OutputBufferSuppliedMessage
	};

	//
	// OutputBufferSuppliedMessage
	//	This will be sent from the DP::Sample::Stream to the Assistant in response to a OutputBufferRequestedMessage when buffers are avaialble
	struct OutputBufferSuppliedMessage
	{
		mach_msg_header_t				mHeader;				// Standard Mach message header
		mach_msg_body_t					mBody;					// Message requires 1 out-of-line (OOL) descriptor for the buffer
		mach_msg_ool_descriptor_t		mDescriptor;			// Out-of-Line (OOL) descriptor for buffer
        UInt64                          mSequenceNumber;        // Sequence number associated with the buffer
        UInt32                          mDiscontinuityFlags;    // flags to indicate play or scrub
        SMPTETime                       mSMPTETime;              // SMPTETime associated with the buffer
	};

    //
    // OutputSurfaceSuppliedMessage
    //	This will be sent from the DP::AJA::Stream to the Assistant in response to a OutputBufferRequestedMessage when buffers are avaialble
    struct OutputSurfaceSuppliedMessage
    {
        mach_msg_header_t				mHeader;				// Standard Mach message header
        mach_msg_body_t					mBody;					// Message requires 1 out-of-line (OOL) descriptor for the buffer
        mach_msg_port_descriptor_t      mDescriptor;			// Describes the port gotten from IOSurfaceCreateMachPort()
        UInt64                          mSequenceNumber;        // Sequence number associated with the buffer
        UInt32                          mDiscontinuityFlags;    // flags to indicate play or scrub
        SMPTETime                       mSMPTETime;              // SMPTETime associated with the buffer
    };

	//
	// NoOutputBufferSuppliedMessage
	//	This will be sent from the DP::Sample::Stream to the Assistant in response to a OutputBufferRequestedMessage when buffers are NOT avaialble
	struct NoOutputBufferSuppliedMessage
	{
		mach_msg_header_t				mHeader;				// Standard Mach message header
	};	
	//
	// OutputBufferMessages
	//	A union of the different types of output buffer request and supplied messages
	union OutputBufferMessages
	{
		struct OutputBufferRequestedMessage		asOutputBufferRequestedMessage;
		struct OutputBufferSuppliedMessage		asOutputBufferSuppliedMessage;
		struct OutputSurfaceSuppliedMessage		asOutputSurfaceSuppliedMessage;
		struct NoOutputBufferSuppliedMessage	asNoOutputBufferSuppliedMessage;
	};
	

}}}
#pragma pack(pop)
#endif
