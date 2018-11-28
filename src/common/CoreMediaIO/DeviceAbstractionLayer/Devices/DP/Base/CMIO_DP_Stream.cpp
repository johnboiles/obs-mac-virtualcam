/*
	    File: CMIO_DP_Stream.cpp
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
#include "CMIO_DP_Stream.h"

// Internal Includes
#include "CMIO_DP_Device.h"
#include "CMIO_DP_PlugIn.h"

// Public Utility Includes
#include "CMIODebugMacros.h"

// CA Public Utility Includes
#include "CACFString.h"
#include "CAException.h"
#include "CAMutex.h"

namespace CMIO { namespace DP
{
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// Stream()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	Stream::Stream(CMIOStreamID mediaIOStreamID, PlugIn& plugIn, Device& owningDevice, CMIOObjectPropertyScope scope, UInt32 startingDeviceChannelNumber) :
		Object(mediaIOStreamID, kCMIOStreamClassID, plugIn),
		mOwningDevice(owningDevice),
		mIsInput(kCMIODevicePropertyScopeInput == scope),
		mStartingDeviceChannelNumber(startingDeviceChannelNumber),
		mStartingDeviceChannelNumberOffset(0),
		mFormatList(NULL),
		mClock(NULL),
		mStreaming(false)
	{
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// Stream()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	Stream::Stream(CMIOStreamID cmioStreamID, PlugIn& plugIn, Device& owningDevice, CMIOObjectPropertyScope scope, UInt32 startingDeviceChannelNumber, UInt32 startingDeviceChannelNumberOffset) :
		Object(cmioStreamID, kCMIOStreamClassID, plugIn),
		mOwningDevice(owningDevice),
		mIsInput(kCMIODevicePropertyScopeInput == scope),
		mStartingDeviceChannelNumber(startingDeviceChannelNumber),
		mStartingDeviceChannelNumberOffset(startingDeviceChannelNumberOffset),
		mFormatList(NULL),
		mClock(NULL)
	{
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// Stream~()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	Stream::~Stream()
	{
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// Initialize()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void Stream::Initialize()
	{
		mFormatList = new Property::FormatList(this);
		AddProperty(mFormatList);
		
		mClock = new Property::Clock(this);
		AddProperty(mClock);
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// Teardown()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void Stream::Teardown()
	{
		RemoveProperty(mFormatList);
		delete mFormatList;
		mFormatList = NULL;
		
		RemoveProperty(mClock);
		delete mClock;
		mClock = NULL;
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// GetObjectStateMutex()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	CAMutex* Stream::GetObjectStateMutex()
	{
		return mOwningDevice.GetObjectStateMutex();
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// CopyStreamName()
	//	This routine should return a CFStringRef that contains a string that is the human readable and localized name of the stream.  Note that the caller will CFRelease the returned object,
	//	so be sure that the object's ref count is correct.
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	CFStringRef	Stream::CopyStreamName() const
	{
		return NULL;
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// CopyStreamManufacturerName()
	//	This routine should return a CFStringRef that contains a string that is the human readable and localized name of the stream's manufacturer.  Note that the caller will CFRelease the
	//	returned object, so be sure that the object's ref count is correct.
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	CFStringRef	Stream::CopyStreamManufacturerName() const
	{
		return GetOwningDevice().CopyDeviceManufacturerName();
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// CopyElementFullName()
	//	This routine returns a human readable, localized name of the given element this routine shouldn't throw an exception. Just return NULL if the value doesn't exist.
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	CFStringRef	Stream::CopyElementFullName(const CMIOObjectPropertyAddress& address) const
	{
		CMIOObjectPropertyAddress changedAddress = address;
		ChangeStreamPropertyAddressIntoDevicePropertyAddress(changedAddress);
		return GetOwningDevice().CopyElementFullName(changedAddress);
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// CopyElementCategoryName()
	//	This routine returns a human readable, localized name of the category of the given element this routine shouldn't throw an exception. Just return NULL if the value doesn't exist.
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	CFStringRef	Stream::CopyElementCategoryName(const CMIOObjectPropertyAddress& address) const
	{
		CMIOObjectPropertyAddress changedAddress = address;
		ChangeStreamPropertyAddressIntoDevicePropertyAddress(changedAddress);
		return GetOwningDevice().CopyElementCategoryName(changedAddress);
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// CopyElementNumberName()
	//	This routine returns a human readable, localized name of the number of the given element this routine shouldn't throw an exception. Just return NULL if the value doesn't exist
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	CFStringRef	Stream::CopyElementNumberName(const CMIOObjectPropertyAddress& address) const
	{
		CMIOObjectPropertyAddress changedAddress = address;
		ChangeStreamPropertyAddressIntoDevicePropertyAddress(changedAddress);
		return GetOwningDevice().CopyElementNumberName(changedAddress);
	}

	UInt32	Stream::GetTerminalType() const
	{
		return 0;
	}

	UInt32	Stream::GetLatency() const
	{
		return 0;
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// SetNoDataTimeout()
	//	This gets called from the CMIO::DP::Property::NoData::SetPropertyData(), so override if anything is needed to be set the no data timeout interval.
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void Stream::SetNoDataTimeout(UInt32 /*noDataTimeout*/)
	{
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// SetDeviceSyncTimeout()
	//	This gets called from the CMIO::DP::Property::NoData::SetPropertyData(), so override if anything is needed to be set the device sync timeout interval.
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void Stream::SetDeviceSyncTimeout(UInt32 /*deviceSyncTimeout*/)
	{
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// SetEndOfData()
	//	This gets called from the CMIO::DP::Property::EndOfData::SetPropertyData(), so override if anything is needed to be set the stream's end-of-data marker.
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void Stream::SetEndOfData(UInt32 /*endOfData*/)
	{
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// ChangeDevicePropertyAddressIntoStreamPropertyAddress()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void Stream::ChangeDevicePropertyAddressIntoStreamPropertyAddress(CMIOObjectPropertyAddress& address) const
	{
		address.mScope = kCMIOObjectPropertyScopeGlobal;
		address.mElement = (address.mElement == 0) ? 0 : address.mElement - mStartingDeviceChannelNumber + 1;
	}
	
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// SetEndOfData()
	//	This gets called from the CMIO::DP::Property::EndOfData::SetPropertyData(), so override if anything is needed to be set the stream's end-of-data marker.
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void Stream::ChangeStreamPropertyAddressIntoDevicePropertyAddress(CMIOObjectPropertyAddress& address) const
	{
		address.mScope = GetDevicePropertyScope();
		address.mElement = (address.mElement == 0) ? 0 : address.mElement + mStartingDeviceChannelNumber - 1;
	}

	bool	Stream::HasProperty(const CMIOObjectPropertyAddress& address) const
	{
		// Initialize the return value
		bool answer = false;
		
		// Initialize some commonly used variables
		CFStringRef cfstring = NULL;
		
		// Take and hold the state mutex
		CAMutex::Locker stateMutex(const_cast<Device&>(mOwningDevice).GetStateMutex());
		
		switch (address.mSelector)
		{
			case kCMIOObjectPropertyName:
				cfstring = CopyStreamName();
				if (cfstring != NULL)
				{
					answer = true;
					CFRelease(cfstring);
				}
				break;
				
			case kCMIOObjectPropertyManufacturer:
				cfstring = CopyStreamManufacturerName();
				if (cfstring != NULL)
				{
					answer = true;
					CFRelease(cfstring);
				}
				break;
				
			case kCMIOObjectPropertyElementName:
				cfstring = CopyElementFullName(address);
				if (cfstring != NULL)
				{
					answer = true;
					CFRelease(cfstring);
				}
				break;
				
			case kCMIOObjectPropertyElementCategoryName:
				cfstring = CopyElementCategoryName(address);
				if (cfstring != NULL)
				{
					answer = true;
					CFRelease(cfstring);
				}
				break;
				
			case kCMIOObjectPropertyElementNumberName:
				cfstring = CopyElementNumberName(address);
				if (cfstring != NULL)
				{
					answer = true;
					CFRelease(cfstring);
				}
				break;
				
			case kCMIOStreamPropertyDirection:
				answer = true;
				break;
				
			case kCMIOStreamPropertyTerminalType:
				answer = true;
				break;
				
			case kCMIOStreamPropertyStartingChannel:
				answer = true;
				break;
				
			case kCMIOStreamPropertyLatency:
				answer = true;
				break;
				
			default:
				answer = Object::HasProperty(address);
				break;
		};
		
		return answer;
	}

	bool	Stream::IsPropertySettable(const CMIOObjectPropertyAddress& address) const
	{
		bool answer = false;
		
		// Take and hold the state mutex
		CAMutex::Locker stateMutex(const_cast<Device&>(mOwningDevice).GetStateMutex());
		
		switch (address.mSelector)
		{
			case kCMIOObjectPropertyName:
				answer = false;
				break;
				
			case kCMIOObjectPropertyManufacturer:
				answer = false;
				break;
				
			case kCMIOObjectPropertyElementName:
				answer = false;
				break;
				
			case kCMIOObjectPropertyElementCategoryName:
				answer = false;
				break;
				
			case kCMIOObjectPropertyElementNumberName:
				answer = false;
				break;
				
			case kCMIOStreamPropertyDirection:
				answer = false;
				break;
				
			case kCMIOStreamPropertyTerminalType:
				answer = false;
				break;
				
			case kCMIOStreamPropertyStartingChannel:
				answer = false;
				break;
				
			case kCMIOStreamPropertyLatency:
				answer = false;
				break;
				
			default:
				answer = Object::IsPropertySettable(address);
				break;
		};
		
		return answer;
	}

	UInt32	Stream::GetPropertyDataSize(const CMIOObjectPropertyAddress& address, UInt32 qualifierDataSize, const void* qualifierData) const
	{
		UInt32 answer = 0;
		
		// Take and hold the state mutex
		CAMutex::Locker stateMutex(const_cast<Device&>(mOwningDevice).GetStateMutex());
		
		switch (address.mSelector)
		{
			case kCMIOObjectPropertyName:
				answer = sizeof(CFStringRef);
				break;
				
			case kCMIOObjectPropertyManufacturer:
				answer = sizeof(CFStringRef);
				break;
				
			case kCMIOObjectPropertyElementName:
				answer = sizeof(CFStringRef);
				break;
				
			case kCMIOObjectPropertyElementCategoryName:
				answer = sizeof(CFStringRef);
				break;
				
			case kCMIOObjectPropertyElementNumberName:
				answer = sizeof(CFStringRef);
				break;
				
			case kCMIOStreamPropertyDirection:
				answer = sizeof(UInt32);
				break;
				
			case kCMIOStreamPropertyTerminalType:
				answer = sizeof(UInt32);
				break;
				
			case kCMIOStreamPropertyStartingChannel:
				answer = sizeof(UInt32);
				break;
				
			case kCMIOStreamPropertyLatency:
				answer = sizeof(UInt32);
				break;
				
			default:
				answer = Object::GetPropertyDataSize(address, qualifierDataSize, qualifierData);
				break;
		};
		
		return answer;
	}

	void	Stream::GetPropertyData(const CMIOObjectPropertyAddress& address, UInt32 qualifierDataSize, const void* qualifierData, UInt32 dataSize, UInt32& dataUsed, void* data) const
	{
		// Take and hold the state mutex
		CAMutex::Locker stateMutex(const_cast<Device&>(mOwningDevice).GetStateMutex());
		
		switch (address.mSelector)
		{
			case kCMIOObjectPropertyName:
				ThrowIf(dataSize != GetPropertyDataSize(address, qualifierDataSize, qualifierData), CAException(kCMIOHardwareBadPropertySizeError), "CMIO::DP::Stream::GetPropertyData: wrong data size for kCMIOObjectPropertyName");
				*static_cast<CFStringRef*>(data) = CopyStreamName();
				dataUsed = sizeof(CFStringRef);
				break;
				
			case kCMIOObjectPropertyManufacturer:
				ThrowIf(dataSize != GetPropertyDataSize(address, qualifierDataSize, qualifierData), CAException(kCMIOHardwareBadPropertySizeError), "CMIO::DP::Stream::GetPropertyData: wrong data size for kCMIOObjectPropertyName");
				*static_cast<CFStringRef*>(data) = CopyStreamManufacturerName();
				dataUsed = sizeof(CFStringRef);
				break;
				
			case kCMIOObjectPropertyElementName:
				ThrowIf(dataSize != GetPropertyDataSize(address, qualifierDataSize, qualifierData), CAException(kCMIOHardwareBadPropertySizeError), "CMIO::DP::Stream::GetPropertyData: wrong data size for kCMIOObjectPropertyElementName");
				*static_cast<CFStringRef*>(data) = CopyElementFullName(address);
				dataUsed = sizeof(CFStringRef);
				break;
				
			case kCMIOObjectPropertyElementCategoryName:
				ThrowIf(dataSize != GetPropertyDataSize(address, qualifierDataSize, qualifierData), CAException(kCMIOHardwareBadPropertySizeError), "CMIO::DP::Stream::GetPropertyData: wrong data size for kCMIOObjectPropertyElementCategoryName");
				*static_cast<CFStringRef*>(data) = CopyElementCategoryName(address);
				dataUsed = sizeof(CFStringRef);
				break;
				
			case kCMIOObjectPropertyElementNumberName:
				ThrowIf(dataSize != GetPropertyDataSize(address, qualifierDataSize, qualifierData), CAException(kCMIOHardwareBadPropertySizeError), "CMIO::DP::Stream::GetPropertyData: wrong data size for kCMIOObjectPropertyElementNumberName");
				*static_cast<CFStringRef*>(data) = CopyElementNumberName(address);
				dataUsed = sizeof(CFStringRef);
				break;
				
			case kCMIOStreamPropertyDirection:
				ThrowIf(dataSize != GetPropertyDataSize(address, qualifierDataSize, qualifierData), CAException(kCMIOHardwareBadPropertySizeError), "CMIO::DP::Stream::GetPropertyData: wrong data size for kCMIOStreamPropertyDirection");
				*static_cast<UInt32*>(data) = IsInput() ? 1 : 0;
				dataUsed = sizeof(UInt32);
				break;
				
			case kCMIOStreamPropertyTerminalType:
				ThrowIf(dataSize != GetPropertyDataSize(address, qualifierDataSize, qualifierData), CAException(kCMIOHardwareBadPropertySizeError), "CMIO::DP::Stream::GetPropertyData: wrong data size for kCMIOStreamPropertyTerminalType");
				*static_cast<UInt32*>(data) = GetTerminalType();
				dataUsed = sizeof(UInt32);
				break;
				
			case kCMIOStreamPropertyStartingChannel:
				ThrowIf(dataSize != GetPropertyDataSize(address, qualifierDataSize, qualifierData), CAException(kCMIOHardwareBadPropertySizeError), "CMIO::DP::Stream::GetPropertyData: wrong data size for kCMIOStreamPropertyStartingChannel");
				*static_cast<UInt32*>(data) = GetStartingDeviceChannelNumber();
				dataUsed = sizeof(UInt32);
				break;

			case kCMIOStreamPropertyLatency:
				ThrowIf(dataSize != GetPropertyDataSize(address, qualifierDataSize, qualifierData), CAException(kCMIOHardwareBadPropertySizeError), "CMIO::DP::Stream::GetPropertyData: wrong data size for kCMIOStreamPropertyLatency");
				*static_cast<UInt32*>(data) = GetLatency();
				dataUsed = sizeof(UInt32);
				break;

			default:
				Object::GetPropertyData(address, qualifierDataSize, qualifierData, dataSize, dataUsed, data);
				break;
		};
	}

	void	Stream::SetPropertyData(const CMIOObjectPropertyAddress& address, UInt32 qualifierDataSize, const void* qualifierData, UInt32 dataSize, const void* data)
	{
		// Take and hold the state mutex
		CAMutex::Locker stateMutex(mOwningDevice.GetStateMutex());
		
		switch (address.mSelector)
		{
			default:
				Object::SetPropertyData(address, qualifierDataSize, qualifierData, dataSize, data);
				break;
		};
	}

	UInt32	Stream::GetStreamID() const
	{
		// override in objects that need this
		return 0;
	}

	void	Stream::Show() const
	{
		// Get the object's name
		PropertyAddress address(kCMIOObjectPropertyName, kCMIOObjectPropertyScopeGlobal, kCMIOObjectPropertyElementMaster);
		CFStringRef cfname = NULL;
		UInt32 dataUsed = 0;
		try
		{
			GetPropertyData(address, 0, NULL, sizeof(CFStringRef), dataUsed, &cfname);
		}
		catch(...)
		{
			cfname = NULL;
		}
		
		// Make a C string out of the name
		char name[256];
		name[0] = 0;
		if (cfname != NULL)
		{
			CFIndex length = 0;
			CFRange range = { 0, CFStringGetLength(cfname) };
			CFStringGetBytes(cfname, range, kCFStringEncodingUTF8, 0, false, (UInt8*)name, 255, &length);
			name[length] = 0;
			CFRelease(cfname);
		}
		
		// Print the information to the standard output
		printf("CMIOObjectID:\t0x%lX\n\tClass:\t\t%s\n\tName:\t\t%s\n\tDirection:\t%s\n\tChannels:\t%lu\n", (long unsigned int)mObjectID, "CMIO Stream", name, (IsInput() ? "Input" : "Output"), (long unsigned int)GetCurrentNumberChannels());
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// TellHardwareToSetFormatDescription()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	bool Stream::TellHardwareToSetFormatDescription(CMFormatDescriptionRef /*format*/)
	{
		return true;
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// TellHardwareToSetFrameRate()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	bool Stream::TellHardwareToSetFrameRate(Float64 /*framerate*/)
	{
		return true;
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// TellHardwareToSetMinimumFrameRate()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	bool Stream::TellHardwareToSetMinimumFrameRate(Float64 /*framerate*/)
	{
		return true;
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// TellHardwareToGetStillImage()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	CMSampleBufferRef Stream::TellHardwareToGetStillImage(CMFormatDescriptionRef /*description*/)
	{
		SubclassResponsibility("CMIO::DP::Stream::TellHardwareToGetStillImage", kCMIOHardwareIllegalOperationError);
		return NULL;
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// TellHardwareToSetPreferredFormatDescription()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	bool Stream::TellHardwareToSetPreferredFormatDescription(CMFormatDescriptionRef /*preferredFormat*/)
	{
		return true;
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// TellHardwareToSetPreferredFrameRate()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	bool Stream::TellHardwareToSetPreferredFrameRate(Float64 /*preferredFramerate*/)
	{
		return true;
	}
}}
