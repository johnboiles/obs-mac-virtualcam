/*
	    File: CMIO_DP_Property_DeviceMaster.cpp
	Abstract: Implements kCMIODevicePropertyDeviceMaster property.
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
#include "CMIO_DP_Property_DeviceMaster.h"

// Internal Includes
#include "CMIO_DP_Device.h"

// Public Utility Includes
#include "CMIODebugMacros.h"
#include "CMIO_PropertyAddress.h"

// CA Public Utility Includes
#include "CAException.h"

// System Includes
//#include <CoreAudio/AudioHardware.h>

namespace CMIO { namespace DP { namespace Property
{
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// IsActive()
	//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	bool DeviceMaster::IsActive(const CMIOObjectPropertyAddress& /*address*/) const
	{
		return true;
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// IsPropertySettable()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	bool DeviceMaster::IsPropertySettable(const CMIOObjectPropertyAddress& address) const
	{
		bool answer = false;
		
		switch (address.mSelector)
		{
			case kCMIODevicePropertyDeviceMaster:
				answer = true;
				break;
		};
		
		return answer;
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// GetPropertyDataSize()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	UInt32 DeviceMaster::GetPropertyDataSize(const CMIOObjectPropertyAddress& address, UInt32 /*qualifierDataSize*/, const void* /*qualifierData*/) const
	{
		UInt32 answer = 0;
		
		switch (address.mSelector)
		{
			case kCMIODevicePropertyDeviceMaster:
				answer = sizeof(pid_t);
				break;
		};
		
		return answer;
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// GetPropertyData()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void DeviceMaster::GetPropertyData(const CMIOObjectPropertyAddress& address, UInt32 /*qualifierDataSize*/, const void* /*qualifierData*/, UInt32 dataSize, UInt32& dataUsed, void* data) const
	{
		switch (address.mSelector)
		{
			case kCMIODevicePropertyDeviceMaster:
				ThrowIf(dataSize != sizeof(pid_t), CAException(kCMIOHardwareBadPropertySizeError), "CMIO::DP::DeviceMaster::GetPropertyData: wrong data size for kCMIODevicePropertyDeviceMaster");
				*(static_cast<pid_t*>(data)) = mMasterPID;
				dataUsed = sizeof(pid_t);
				break;
		};
	}
	
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// SetPropertyData()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void DeviceMaster::SetPropertyData(const CMIOObjectPropertyAddress& address, UInt32 /*qualifierDataSize*/, const void* /*qualifierData*/, UInt32 dataSize, const void* data)
	{
		switch (address.mSelector)
		{
			case kCMIODevicePropertyDeviceMaster:
				ThrowIf(dataSize != sizeof(pid_t), CAException(kCMIOHardwareBadPropertySizeError), "CMIO::DP::DeviceMaster::SetPropertyData: wrong data size for kCMIODevicePropertyDeviceMaster");
				pid_t masterPID = *(static_cast<const pid_t*>(data));
				
				if (masterPID != mMasterPID)
				{
					// The master can only be changed if this process is the current master or if it is free
					ThrowIf(not CurrentProcessIsMasterOrIsFree(), CAException(kCMIODevicePermissionsError), "CMIO::DP::IIDC::DeviceMaster::SetPropertyData: kCMIODevicePropertyDeviceMaster cannot be changed if not CurrentProcessIsMasterOrIsFree()"); 
					
					// Only values of -1 and the processes' own PID are valid
					ThrowIf((-1 != masterPID) and (CAProcess::GetPID() != masterPID), CAException(kCMIOHardwareIllegalOperationError), "CMIO::DP::IIDC::DeviceMaster::SetPropertyData: invalid pid_t for kCMIODevicePropertyDeviceMaster");

					// Attempt to set the device master
					GetOwningDevice().SetDeviceMaster(masterPID);
					mMasterPID = masterPID;
					
					// Signal that device master changed
					PropertyAddress changedAddress(kCMIODevicePropertyDeviceMaster);
					GetOwningDevice().PropertiesChanged(1, &changedAddress);
				}
				break;
		};
	}
	
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// GetNumberAddressesImplemented()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	UInt32 DeviceMaster::GetNumberAddressesImplemented() const
	{
		return 1;
	}
	
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// GetImplementedAddressByIndex()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void DeviceMaster::GetImplementedAddressByIndex(UInt32 index, CMIOObjectPropertyAddress& address) const
	{
		switch (index)
		{
			case 0:
				address.mSelector = kCMIODevicePropertyDeviceMaster;
				address.mScope = kCMIOObjectPropertyScopeGlobal;
				address.mElement = kCMIOObjectPropertyElementMaster;
				break;
		};
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// SetMaster()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void DeviceMaster::SetMaster(pid_t masterPID, bool sendChangeNotifications) 
	{
		// Send out property change notifications if the master is different
		if (masterPID != mMasterPID)
		{
			mMasterPID = masterPID;
			
			if (sendChangeNotifications)
			{
				PropertyAddress changedAddress(kCMIODevicePropertyDeviceMaster);
				mOwningDevice.PropertiesChanged(1, &changedAddress);
			}
		}
	}
}}}
