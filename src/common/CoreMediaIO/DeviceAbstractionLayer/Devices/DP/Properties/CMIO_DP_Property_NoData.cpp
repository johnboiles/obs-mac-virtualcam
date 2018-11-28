/*
	    File: CMIO_DP_Property_NoData.cpp
	Abstract: Implements the kCMIOStreamPropertyNoDataTimeoutInMSec, kCMIOStreamPropertyDeviceSyncTimeoutInMSec, and kCMIOStreamPropertyNoDataEventCount properties.
				These deal with a device detecting that no data is coming in (such as a DV device on a section of blank tape).
	
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
#include "CMIO_DP_Property_NoData.h"

// Internal Includes
#include "CMIO_DP_Stream.h"

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
	bool NoData::IsActive(const CMIOObjectPropertyAddress& /*address*/) const
	{
		return true;
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// IsPropertySettable()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	bool NoData::IsPropertySettable(const CMIOObjectPropertyAddress& address) const
	{
		bool answer = false;
		
		switch (address.mSelector)
		{
			case kCMIOStreamPropertyNoDataTimeoutInMSec:
			case kCMIOStreamPropertyDeviceSyncTimeoutInMSec:
				answer = true;
				break;
			
			case kCMIOStreamPropertyNoDataEventCount:
				answer = false;
				break;
		};
		
		return answer;
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// GetPropertyDataSize()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	UInt32 NoData::GetPropertyDataSize(const CMIOObjectPropertyAddress& address, UInt32 /*qualifierDataSize*/, const void* /*qualifierData*/) const
	{
		UInt32 answer = 0;
		
		switch (address.mSelector)
		{
			case kCMIOStreamPropertyNoDataTimeoutInMSec:
			case kCMIOStreamPropertyDeviceSyncTimeoutInMSec:
			case kCMIOStreamPropertyNoDataEventCount:
				answer = sizeof(UInt32);
				break;
		};
		
		return answer;
	}
	
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// GetPropertyData()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void NoData::GetPropertyData(const CMIOObjectPropertyAddress& address, UInt32 /*qualifierDataSize*/, const void* /*qualifierData*/, UInt32 dataSize, UInt32& dataUsed, void* data) const
	{
		switch (address.mSelector)
		{
			case kCMIOStreamPropertyNoDataTimeoutInMSec:
				ThrowIf(dataSize != sizeof(UInt32), CAException(kCMIOHardwareBadPropertySizeError), "CMIO::DP::NoData::GetPropertyData: wrong data size for kCMIOStreamPropertyNoDataTimeoutInMSec");
				*(static_cast<UInt32*>(data)) = mTimeout;
				dataUsed = sizeof(UInt32);
				break;
			
			case kCMIOStreamPropertyDeviceSyncTimeoutInMSec:
				ThrowIf(dataSize != sizeof(UInt32), CAException(kCMIOHardwareBadPropertySizeError), "CMIO::DP::NoData::GetPropertyData: wrong data size for kCMIOStreamPropertyDeviceSyncTimeoutInMSec");
				*(static_cast<UInt32*>(data)) = mDeviceSyncTimeout;
				dataUsed = sizeof(UInt32);
				break;
			
			case kCMIOStreamPropertyNoDataEventCount:
				ThrowIf(dataSize != sizeof(UInt32), CAException(kCMIOHardwareBadPropertySizeError), "CMIO::DP::NoData::GetPropertyData: wrong data size for kCMIOStreamPropertyNoDataEventCount");
				*(static_cast<UInt32*>(data)) = mEventCount;
				dataUsed = sizeof(UInt32);
				break;
		};
	}
	
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// SetPropertyData()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void NoData::SetPropertyData(const CMIOObjectPropertyAddress& address, UInt32 /*qualifierDataSize*/, const void* /*qualifierData*/, UInt32 dataSize, const void* data)
	{
		switch (address.mSelector)
		{
			case kCMIOStreamPropertyNoDataTimeoutInMSec:
			{
				ThrowIf(dataSize != sizeof(UInt32), CAException(kCMIOHardwareBadPropertySizeError), "CMIO::DP::NoData::SetPropertyData: wrong data size for kCMIOStreamPropertyNoDataTimeoutInMSec");
				UInt32 timeout = *(static_cast<const UInt32*>(data));
				
				if (timeout != mTimeout)
				{
					// Instruct the device that the force discontinuity state is changing
					GetOwningStream().SetNoDataTimeout(timeout);
					mTimeout = timeout;
					
					// Notify that the property changed
					PropertyAddress changedAddress(kCMIOStreamPropertyNoDataTimeoutInMSec);
					GetOwningStream().PropertiesChanged(1, &changedAddress);
				}
				break;
			}
			
			case kCMIOStreamPropertyDeviceSyncTimeoutInMSec:
			{
				ThrowIf(dataSize != sizeof(UInt32), CAException(kCMIOHardwareBadPropertySizeError), "CMIO::DP::NoData::SetPropertyData: wrong data size for kCMIOStreamPropertyDeviceSyncTimeoutInMSec");
				UInt32 deviceSyncTimeout = *(static_cast<const UInt32*>(data));
				
				if (deviceSyncTimeout != mDeviceSyncTimeout)
				{
					// Instruct the device that the force discontinuity state is changing
					GetOwningStream().SetDeviceSyncTimeout(deviceSyncTimeout);
					mDeviceSyncTimeout = deviceSyncTimeout;
					
					// Notify that the property changed
					PropertyAddress changedAddress(kCMIOStreamPropertyDeviceSyncTimeoutInMSec);
					GetOwningStream().PropertiesChanged(1, &changedAddress);
				}
				break;
			}
		};
	}
	
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// GetNumberAddressesImplemented()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	UInt32 NoData::GetNumberAddressesImplemented() const
	{
		return 3;
	}
	
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// GetImplementedAddressByIndex()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void NoData::GetImplementedAddressByIndex(UInt32 index, CMIOObjectPropertyAddress& address) const
	{
		switch (index)
		{
			case 0:
				address.mSelector = kCMIOStreamPropertyNoDataTimeoutInMSec;
				address.mScope = kCMIOObjectPropertyScopeWildcard;
				address.mElement = kCMIOObjectPropertyElementWildcard;
				break;
			case 1:
				address.mSelector = kCMIOStreamPropertyDeviceSyncTimeoutInMSec;
				address.mScope = kCMIOObjectPropertyScopeWildcard;
				address.mElement = kCMIOObjectPropertyElementWildcard;
				break;
			case 2:
				address.mSelector = kCMIOStreamPropertyNoDataEventCount;
				address.mScope = kCMIOObjectPropertyScopeWildcard;
				address.mElement = kCMIOObjectPropertyElementWildcard;
				break;
		};
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// BumpNoDataEventCount()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void NoData::BumpNoDataEventCount(bool sendChangeNotifications)
	{
		// Bump the event count
		++mEventCount;
		
		// Send the property changed notification
		if (sendChangeNotifications)
		{
			PropertyAddress address(kCMIOStreamPropertyNoDataEventCount);
			GetOwningStream().PropertiesChanged(1, &address);
		}
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// SetNoDataTimeoutInMSec()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void NoData::SetNoDataTimeoutInMSec(UInt32 timeout, bool sendChangeNotifications) 
	{
		// Send out property change notifications there has been a change
		if (timeout != mTimeout)
		{
			mTimeout = timeout;
			
			if (sendChangeNotifications)
			{
				PropertyAddress changedAddress(kCMIOStreamPropertyNoDataTimeoutInMSec);
				GetOwningStream().PropertiesChanged(1, &changedAddress);
			}
		}
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// SetDeviceSyncTimeoutInMSec()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void NoData::SetDeviceSyncTimeoutInMSec(UInt32 deviceSyncTimeout, bool sendChangeNotifications) 
	{
		// Send out property change notifications there has been a change
		if (deviceSyncTimeout != mDeviceSyncTimeout)
		{
			mDeviceSyncTimeout = deviceSyncTimeout;
			
			if (sendChangeNotifications)
			{
				PropertyAddress changedAddress(kCMIOStreamPropertyNoDataTimeoutInMSec);
				GetOwningStream().PropertiesChanged(1, &changedAddress);
			}
		}
	}
}}}