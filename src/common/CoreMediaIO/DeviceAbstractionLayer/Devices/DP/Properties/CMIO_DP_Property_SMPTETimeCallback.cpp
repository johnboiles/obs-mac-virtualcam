/*
	    File: CMIO_DP_Property_SMPTETimeCallback.cpp
	Abstract: Implements the kCMIODevicePropertySMPTETimeCallback property which allows a client to provide a callback which can be used to get the current SMPTE timestamp from a device.
	
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
#include "CMIO_DP_Property_SMPTETimeCallback.h"

// Internal Includes
#include "CMIO_DP_Device.h"

// Public Utility Includes
#include "CMIODebugMacros.h"
#include "CMIO_PropertyAddress.h"

// CA Public Utility Includes
#include "CAException.h"

namespace CMIO { namespace DP { namespace Property
{
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// IsActive()
	//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	bool SMPTETimeCallback::IsActive(const CMIOObjectPropertyAddress& /*address*/) const
	{
		return true;
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// IsPropertySettable()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	bool SMPTETimeCallback::IsPropertySettable(const CMIOObjectPropertyAddress& address) const
	{
		bool answer = false;
		
		switch (address.mSelector)
		{
			case kCMIODevicePropertySMPTETimeCallback:
				answer = true;
				break;
		};
		
		return answer;
	}


	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// GetPropertyDataSize()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	UInt32 SMPTETimeCallback::GetPropertyDataSize(const CMIOObjectPropertyAddress& address, UInt32 /*qualifierDataSize*/, const void* /*qualifierData*/) const
	{
		UInt32 answer = 0;
		
		switch (address.mSelector)
		{
			case kCMIODevicePropertySMPTETimeCallback:
				answer = sizeof(CMIODeviceSMPTETimeCallback);
				break;
		};
		
		return answer;
	}
	
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// GetPropertyData()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void SMPTETimeCallback::GetPropertyData(const CMIOObjectPropertyAddress& address, UInt32 /*qualifierDataSize*/, const void* /*qualifierData*/, UInt32 dataSize, UInt32& dataUsed, void* data) const
	{
		switch (address.mSelector)
		{
			case kCMIODevicePropertySMPTETimeCallback:
				ThrowIf(dataSize != sizeof(CMIODeviceSMPTETimeCallback), CAException(kCMIOHardwareBadPropertySizeError), "SMPTETimeCallback::GetData: wrong data size for kCMIODevicePropertySMPTETimeCallback");
				*(static_cast<CMIODeviceSMPTETimeCallback*>(data)) = mSMPTECallback;
				dataUsed = sizeof(CMIODeviceSMPTETimeCallback);
				break;
		};
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// SetPropertyData()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void SMPTETimeCallback::SetPropertyData(const CMIOObjectPropertyAddress& address, UInt32 /*qualifierDataSize*/, const void* /*qualifierData*/, UInt32 dataSize, const void* data)
	{
		switch (address.mSelector)
		{
			case kCMIODevicePropertySMPTETimeCallback:
				ThrowIf(dataSize != sizeof(CMIODeviceSMPTETimeCallback), CAException(kCMIOHardwareBadPropertySizeError), "SMPTETimeCallback::SetData: wrong data size for kCMIODevicePropertySMPTETimeCallback");
				mSMPTECallback = *(static_cast<const CMIODeviceSMPTETimeCallback*>(data));
				PropertyAddress address1(kCMIODevicePropertySMPTETimeCallback);
				mOwningDevice.PropertiesChanged(1, &address1);
				break;
		};
	}
	
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// GetNumberAddressesImplemented()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	UInt32 SMPTETimeCallback::GetNumberAddressesImplemented() const
	{
		return 1;
	}
	
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// GetImplementedAddressByIndex()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void SMPTETimeCallback::GetImplementedAddressByIndex(UInt32 index, CMIOObjectPropertyAddress& address) const
	{
		switch (index)
		{
			case 0:
				address.mSelector = kCMIODevicePropertySMPTETimeCallback;
				address.mScope = kCMIOObjectPropertyScopeWildcard;
				address.mElement = kCMIOObjectPropertyElementWildcard;
				break;
		};
	}
	
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// DoCallback()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	OSStatus SMPTETimeCallback::DoCallback(UInt64* frameNumber, Boolean* isDropFrame, UInt32* tolerance) const
	{
		if (NULL == mSMPTECallback.mGetSMPTETimeProc)
			return kCMIOHardwareIllegalOperationError;
			
		return (mSMPTECallback.mGetSMPTETimeProc)(mSMPTECallback.mRefCon, frameNumber, isDropFrame, tolerance);
	}
}}}