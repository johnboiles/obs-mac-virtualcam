/*
	    File: CMIO_DP_Property_OutputBuffers.cpp
	Abstract: Implements the kCMIOStreamPropertyOutputBufferQueueSize, kCMIOStreamPropertyOutputBuffersRequiredForStartup, kCMIOStreamPropertyOutputBufferUnderrunCount, and
				kCMIOStreamPropertyOutputBufferRepeatCount properties.
	
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
#include "CMIO_DP_Property_OutputBuffers.h"

// Internal Includes
#include "CMIO_DP_Stream.h"

// Public Utility Includes
#include "CMIODebugMacros.h"
#include "CMIO_PropertyAddress.h"

// CA Public Utility Includes
#include "CAException.h"

// System Includes
#include <CoreAudio/AudioHardware.h>

namespace CMIO { namespace DP { namespace Property
{
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// OutputBuffers()
	//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	OutputBuffers::OutputBuffers(Stream& owningStream, UInt32 defaultQueueSize) :
		Base(),
		mOwningStream(owningStream),
		mDefaultQueueSize(defaultQueueSize),
		mDefaultBuffersRequiredForStartup((mDefaultQueueSize + 1) / 2),
		mQueueSize(mDefaultQueueSize),
		mBuffersRequiredForStartup(mDefaultBuffersRequiredForStartup),
		mUnderrunCount(0),
		mRepeatCount(0)
	{
	}

	#pragma mark -
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// IsActive()
	//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	bool OutputBuffers::IsActive(const CMIOObjectPropertyAddress& /*address*/) const
	{
		return true;
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// IsPropertySettable()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	bool OutputBuffers::IsPropertySettable(const CMIOObjectPropertyAddress& address) const
	{
		bool answer = false;
		
		switch (address.mSelector)
		{
			case kCMIOStreamPropertyOutputBufferQueueSize:
			case kCMIOStreamPropertyOutputBuffersRequiredForStartup:
				answer = true;
				break;
			
			case kCMIOStreamPropertyOutputBufferUnderrunCount:
			case kCMIOStreamPropertyOutputBufferRepeatCount:
				answer = false;
				break;
		};
		
		return answer;
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// GetPropertyDataSize()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	UInt32 OutputBuffers::GetPropertyDataSize(const CMIOObjectPropertyAddress& address, UInt32 /*qualifierDataSize*/, const void* /*qualifierData*/) const
	{
		UInt32 answer = 0;
		
		switch (address.mSelector)
		{
			case kCMIOStreamPropertyOutputBufferQueueSize:
			case kCMIOStreamPropertyOutputBuffersRequiredForStartup:
			case kCMIOStreamPropertyOutputBufferUnderrunCount:
			case kCMIOStreamPropertyOutputBufferRepeatCount:
				answer = sizeof(UInt32);
				break;
		};
		
		return answer;
	}
	
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// GetPropertyData()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void OutputBuffers::GetPropertyData(const CMIOObjectPropertyAddress& address, UInt32 /*qualifierDataSize*/, const void* /*qualifierData*/, UInt32 dataSize, UInt32& dataUsed, void* data) const
	{
		switch (address.mSelector)
		{
			case kCMIOStreamPropertyOutputBufferQueueSize:
				ThrowIf(dataSize != sizeof(UInt32), CAException(kAudioHardwareBadPropertySizeError), "CMIO::DP::Property::OutputBuffers::GetData: wrong data size for kCMIOStreamPropertyOutputBufferQueueSize");
				*(static_cast<UInt32*>(data)) = mQueueSize;
				dataUsed = sizeof(UInt32);
				break;
			
			case kCMIOStreamPropertyOutputBuffersRequiredForStartup:
				ThrowIf(dataSize != sizeof(UInt32), CAException(kAudioHardwareBadPropertySizeError), "CMIO::DP::Property::OutputBuffers::GetData: wrong data size for kCMIOStreamPropertyOutputBuffersRequiredForStartup");
				*(static_cast<UInt32*>(data)) = mBuffersRequiredForStartup;
				dataUsed = sizeof(UInt32);
				break;
			
			case kCMIOStreamPropertyOutputBufferUnderrunCount:
				ThrowIf(dataSize != sizeof(UInt32), CAException(kAudioHardwareBadPropertySizeError), "CMIO::DP::Property::OutputBuffers::GetData: wrong data size for kCMIOStreamPropertyOutputBufferUnderrunCount");
				*(static_cast<UInt32*>(data)) = mUnderrunCount;
				dataUsed = sizeof(UInt32);
				break;
			
			case kCMIOStreamPropertyOutputBufferRepeatCount:
				ThrowIf(dataSize != sizeof(UInt32), CAException(kAudioHardwareBadPropertySizeError), "CMIO::DP::Property::OutputBuffers::GetData: wrong data size for kCMIOStreamPropertyOutputBufferRepeatCount");
				*(static_cast<UInt32*>(data)) = mRepeatCount;
				dataUsed = sizeof(UInt32);
				break;
		};
	}
	
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// SetPropertyData()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void OutputBuffers::SetPropertyData(const CMIOObjectPropertyAddress& address, UInt32 /*qualifierDataSize*/, const void* /*qualifierData*/, UInt32 dataSize, const void* data)
	{
		switch (address.mSelector)
		{
			case kCMIOStreamPropertyOutputBufferQueueSize:
				ThrowIf(dataSize != sizeof(UInt32), CAException(kAudioHardwareBadPropertySizeError), "CMIO::DP::Property::OutputBuffers::SetData: wrong data size for kCMIOStreamPropertyOutputBufferQueueSize");
				mQueueSize = *(static_cast<const UInt32*>(data));
				if (mBuffersRequiredForStartup > mQueueSize)
				{
					mBuffersRequiredForStartup = mQueueSize;
					
					// Send the property changed notification
					PropertyAddress changedAddress(kCMIOStreamPropertyOutputBuffersRequiredForStartup);
					mOwningStream.PropertiesChanged(1, &changedAddress);
				}
				break;
			
			case kCMIOStreamPropertyOutputBuffersRequiredForStartup:
				ThrowIf(dataSize != sizeof(UInt32), CAException(kAudioHardwareBadPropertySizeError), "CMIO::DP::Property::OutputBuffers::SetData: wrong data size for kCMIOStreamPropertyOutputBuffersRequiredForStartup");
				mBuffersRequiredForStartup = *(static_cast<const UInt32*>(data));
				if (mQueueSize < mBuffersRequiredForStartup)
				{
					mQueueSize = mBuffersRequiredForStartup;

					// Send the property changed notification
					PropertyAddress changedAddress(kCMIOStreamPropertyOutputBufferQueueSize);
					mOwningStream.PropertiesChanged(1, &changedAddress);
				}
				break;
		};
	}
	
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// GetNumberAddressesImplemented()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	UInt32 OutputBuffers::GetNumberAddressesImplemented() const
	{
		return 4;
	}
	
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// GetImplementedAddressByIndex()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void OutputBuffers::GetImplementedAddressByIndex(UInt32 index, CMIOObjectPropertyAddress& address) const
	{
		switch (index)
		{
			case 0:
				address.mSelector = kCMIOStreamPropertyOutputBufferQueueSize;
				address.mScope = kCMIOObjectPropertyScopeWildcard;
				address.mElement = kCMIOObjectPropertyElementWildcard;
				break;
			case 1:
				address.mSelector = kCMIOStreamPropertyOutputBuffersRequiredForStartup;
				address.mScope = kCMIOObjectPropertyScopeWildcard;
				address.mElement = kCMIOObjectPropertyElementWildcard;
				break;
			case 2:
				address.mSelector = kCMIOStreamPropertyOutputBufferUnderrunCount;
				address.mScope = kCMIOObjectPropertyScopeWildcard;
				address.mElement = kCMIOObjectPropertyElementWildcard;
				break;
			case 3:
				address.mSelector = kCMIOStreamPropertyOutputBufferRepeatCount;
				address.mScope = kCMIOObjectPropertyScopeWildcard;
				address.mElement = kCMIOObjectPropertyElementWildcard;
				break;
		};
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// BumpUnderrunCount()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void OutputBuffers::BumpUnderrunCount(bool sendChangeNotifications)
	{
		// Increment the underrun count
		++mUnderrunCount;
		
		if (sendChangeNotifications)
		{
			PropertyAddress address(kCMIOStreamPropertyOutputBufferUnderrunCount);
			mOwningStream.PropertiesChanged(1, &address);
		}
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// BumpRepeatCount()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void OutputBuffers::BumpRepeatCount(bool sendChangeNotifications)
	{
		// Increment the repeat count
		++mRepeatCount;
		
		if (sendChangeNotifications)
		{
			PropertyAddress address(kCMIOStreamPropertyOutputBufferRepeatCount);
			mOwningStream.PropertiesChanged(1, &address);
		}
	}
}}}