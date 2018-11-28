/*
	    File: CMIO_DP_Device.cpp
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
#include "CMIO_DP_Device.h"
#include "CMIO_DP_DeviceCommands.h"

// Internal Includes
#include "CMIO_DP_Control.h"
#include "CMIO_DP_PlugIn.h"
#include "CMIO_DP_Stream.h"

// Public Utility Includes
#include "CMIODebugMacros.h"

// CA Public Utility Includes
#include "CAAutoDisposer.h"
#include "CACFString.h"
#include "CAException.h"

// System Include
#include <IOKit/audio/IOAudioTypes.h>
#include <sys/types.h>

namespace CMIO { namespace DP
{
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// Device()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	Device::Device(CMIODeviceID deviceID, PlugIn& plugIn) :
		Object(deviceID, kCMIODeviceClassID, plugIn),
		mDeviceMaster(NULL),
		mIsAlive(true),
		mExcludeNonDALAccess(false),
		mDeviceIsRunning(false),
		mSuspended(false),
		mSuspendedInputStreams(),
		mSuspendedOutputStreams(),
		mTakeDeviceMasterOnResumption(false),
		mExcludeNonDALAccessOnResumption(false),
		mInputStreamList(),
		mOutputStreamList()
	{
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// ~Device()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	Device::~Device()
	{
	}
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// GetObjectStateMutex()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	CAMutex* Device::GetObjectStateMutex() 
	{ 
		return GetPlugIn().GetObjectStateMutex(); 
	}
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// GetStateMutex()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	CAMutex& Device::GetStateMutex() 
	{ 
		return GetPlugIn().GetStateMutex(); 
	}
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// Initialize()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void Device::Initialize()
	{
		Object::Initialize();

		// Use the CMIO::DP::Property::DeviceMaster property to allow clients to become the device's master
		mDeviceMaster = new Property::DeviceMaster(*this, -1);
		mDeviceMaster->Initialize();
		AddProperty(mDeviceMaster);
		
		// Grab the name of the device for debugging purposes
		#if CMIO_Debug
			CACFString deviceName(CopyDeviceName());
			if (deviceName.IsValid())
			{
				UInt32 stringSize = 255;
				deviceName.GetCString(mDebugDeviceName, stringSize);
			}
			else
			{
				snprintf(mDebugDeviceName, 8, "%s", "Unknown");
			}
		#endif
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// Teardown()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void Device::Teardown()
	{
		if (NULL != mDeviceMaster)
		{
			RemoveProperty(mDeviceMaster);
			delete mDeviceMaster;
			mDeviceMaster = NULL;
		}

		Object::Teardown();
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// CopyDeviceName()
	//	This routine should return a CFStringRef that contains a string that is the human readable and localized name of the device.  Note that the caller will CFRelease the returned object,
	//	so be sure that the object's ref count is correct.
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	CFStringRef Device::CopyDeviceName() const
	{
		return NULL;
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// CopyDeviceManufacturerName()
	//	This routine should return a CFStringRef that contains a string that is the human readable and localized name of the device's manufacturer.  Note that the caller will CFRelease the
	//	returned object, so be sure that the object's ref count is correct.
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	CFStringRef Device::CopyDeviceManufacturerName() const
	{
		return NULL;
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// CopyElementFullName()
	//	This routine returns a human readable, localized name of the given element this routine shouldn't throw an exception. Just return NULL if the value doesn't exist.
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	CFStringRef Device::CopyElementFullName(const CMIOObjectPropertyAddress& /*address*/) const
	{
		return NULL;
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// CopyElementCategoryName()
	//	This routine returns a human readable, localized name of the category of the given element this routine shouldn't throw an exception. Just return NULL if the value doesn't exist.
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	CFStringRef Device::CopyElementCategoryName(const CMIOObjectPropertyAddress& /*address*/) const
	{
		return NULL;
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// CopyElementNumberName()
	//	This routine returns a human readable, localized name of the number of the given element this routine shouldn't throw an exception. Just return NULL if the value doesn't exist.
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	CFStringRef Device::CopyElementNumberName(const CMIOObjectPropertyAddress& /*address*/) const
	{
		return NULL;
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// CopyDeviceUID()
	//	This routine should return a CFStringRef that contains a string that is unique to this instance of the device. This string must be persistant so that it is the same between process
	//	launches and boots. Note that the caller will CFRelease the returned object, so be sure that the object's ref count is correct.
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	CFStringRef Device::CopyDeviceUID() const
	{
		return NULL;
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// CopyModelUID()
	//	This routine should return a CFString that contains a persistent identifier for the model of an CMIODevice. The identifier is unique such that the identifier from two
	//	CMIODevices are equal if and only if the two CMIODevices are the exact same model from the same manufacturer. Further, the identifier has to be the same no matter on what
	//	machine the CMIODevice appears.
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	CFStringRef Device::CopyModelUID() const
	{
		return NULL;
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// GetTransportType()
	//	This routine returns how the device is connected to the system.  The constants used here are defined in <IOKit/audio/IOAudioTypes.h>
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	UInt32 Device::GetTransportType() const
	{
		return 0;
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// IsConstantRateClock()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	bool Device::IsConstantRateClock() const
	{
		// By default, use the transport to determine this
		UInt32 transportType = GetTransportType();
		return transportType == kIOAudioDeviceTransportTypeUSB;
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// CanBeDefaultDevice()
	//	This routine returns whether or not the device is allowed to be selected as the default device.
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	bool Device::CanBeDefaultDevice(CMIOObjectPropertyScope /*scope*/, bool /*isSystem*/) const
	{
		return true;
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// Lock()
	//	This routine should do whatever is necessary to grab exclusive access to the device.
	//	Nothing needs to be done if the device will only be accessed through the DAL and not other software modules (such a s QuickTime video digitizers).
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void Device::Lock()
	{
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// Unlock()
	//	This routine should do whatever is necessary to release exclusive access to the device.
	//	Nothing needs to be done if the device will only be accessed through the DAL and not other software modules (such a s QuickTime video digitizers).
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void Device::Unlock()
	{
	}

	bool Device::HogModeIsOwnedBySelf() const
	{
		return false;
	}

	bool Device::HogModeIsOwnedBySelfOrIsFree() const
	{
		return true;
	}

	void Device::HogModeStateChanged()
	{
		//Do_StopAllIOProcs();
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// SetForceDiscontinuity()
	//	This gets called from the CMIO::DP::Property::ClientSyncDiscontinuity::SetPropertyData(), so override if anything is needed to be done to force a discontinuity in the stream.
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void Device::SetForceDiscontinuity(Boolean /*forceDiscontinuity*/)
	{
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// GetDeviceMaster()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	pid_t Device::GetDeviceMaster() const
	{
		return mDeviceMaster->GetMaster();
	}
	
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// DeviceMasterIsOwnedBySelf()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	bool Device::DeviceMasterIsOwnedBySelf() const
	{
		return mDeviceMaster->CurrentProcessIsMaster();
	}
	
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// DeviceMasterIsOwnedBySelfOrIsFree()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	bool Device::DeviceMasterIsOwnedBySelfOrIsFree() const
	{
		return mDeviceMaster->CurrentProcessIsMasterOrIsFree();
	}
	
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// SetDeviceMaster()
	//	This gets called from the CMIO::DP::Property::DeviceMaster::SetPropertyData(), so override if anything is needed to be done to assume mastership.
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void Device::SetDeviceMaster(pid_t /*masterPID*/)
	{
	}

	void Device::Show() const
	{
		// Make a string for the class name
		const char* className = NULL;
		
		switch (mClassID)
		{
			case kCMIODeviceClassID:
			default:
				className = "CMIO Device";
				break;
		}
		
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
		printf("CMIOObjectID:\t\t\t0x%lX\n\tClass:\t\t\t\t%s\n\tName:\t\t\t\t%s\n\tInput Channels:\t\t%lu\n\tOutput Channels:\t%lu\n", (long unsigned int)mObjectID, className, name, (long unsigned int)GetTotalNumberChannels(kCMIODevicePropertyScopeInput), (long unsigned int)GetTotalNumberChannels(kCMIODevicePropertyScopeOutput));
	}

	bool Device::HasProperty(const CMIOObjectPropertyAddress& address) const
	{
		// Initialize the return value
		bool answer = false;
		
		// Initialize some commonly used variables
		CFStringRef cfstring = NULL;
		
		// Take and hold the state mutex
		CAMutex::Locker stateMutex(const_cast<Device*>(this)->GetStateMutex());
		
		switch (address.mSelector)
		{
			case kCMIOObjectPropertyName:
				cfstring = CopyDeviceName();
				if (cfstring != NULL)
				{
					answer = true;
					CFRelease(cfstring);
				}
				break;
				
			case kCMIOObjectPropertyManufacturer:
				cfstring = CopyDeviceManufacturerName();
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
				
			case kCMIODevicePropertyPlugIn:
				answer = true;
				break;
				
			case kCMIODevicePropertyDeviceUID:
				cfstring = CopyDeviceUID();
				if (cfstring != NULL)
				{
					answer = true;
					CFRelease(cfstring);
				}
				break;
				
			case kCMIODevicePropertyModelUID:
				cfstring = CopyModelUID();
				if (cfstring != NULL)
				{
					answer = true;
					CFRelease(cfstring);
				}
				break;
				
			case kCMIODevicePropertyTransportType:
				answer = true;
				break;
				
			case kCMIODevicePropertyDeviceIsAlive:
				answer = true;
				break;
				
			case kCMIODevicePropertyDeviceHasChanged:
				answer = true;
				break;
				
			case kCMIODevicePropertyDeviceIsRunning:
				answer = true;
				break;
				
			case kCMIODevicePropertyDeviceIsRunningSomewhere:
				answer = true;
				break;
				
			case kCMIODevicePropertyDeviceCanBeDefaultDevice:
				answer = (address.mScope == kCMIODevicePropertyScopeInput) || (address.mScope == kCMIODevicePropertyScopeOutput);
				break;
				
			case kCMIODevicePropertyHogMode:
				answer = true;
				break;
				
			case kCMIODevicePropertyLatency:
				answer = ((address.mScope == kCMIODevicePropertyScopeInput) && HasInputStreams()) || ((address.mScope == kCMIODevicePropertyScopeOutput) && HasOutputStreams());
				break;
				
			case kCMIODevicePropertyStreams:
				answer = (address.mScope == kCMIODevicePropertyScopeInput) || (address.mScope == kCMIODevicePropertyScopeOutput);
				break;
				
			case kCMIODevicePropertyStreamConfiguration:
				answer = (address.mScope == kCMIODevicePropertyScopeInput) || (address.mScope == kCMIODevicePropertyScopeOutput);
				break;
				
			case kCMIODevicePropertyExcludeNonDALAccess:
				// This property is only present if the hog mode property is NOT settable
				answer = not IsPropertySettable(PropertyAddress(kCMIODevicePropertyHogMode));
				break;
				
			case kCMIODevicePropertyCanProcessAVCCommand:
				answer = true;
				break;
				
			case kCMIODevicePropertyCanProcessRS422Command:
				answer = true;
				break;
				
			case kCMIODevicePropertyLinkedCoreAudioDeviceUID:
				answer = false;
				break;
			
			default:
				answer = Object::HasProperty(address);
				break;
		};
		
		return answer;
	}

	bool Device::IsPropertySettable(const CMIOObjectPropertyAddress& address) const
	{
		bool answer = false;
		
		// Take and hold the state mutex
		CAMutex::Locker stateMutex(const_cast<Device*>(this)->GetStateMutex());
		
		switch (address.mSelector)
		{
			case kCMIOObjectPropertyName:
				answer = false;
				break;
				
			case kCMIOObjectPropertyManufacturer:
				answer = false;
				break;
				
			case kCMIOObjectPropertyElementCategoryName:
				answer = false;
				break;
				
			case kCMIOObjectPropertyElementNumberName:
				answer = false;
				break;
				
			case kCMIODevicePropertyPlugIn:
				answer = false;
				break;
				
			case kCMIODevicePropertyDeviceUID:
				answer = false;
				break;
				
			case kCMIODevicePropertyModelUID:
				answer = false;
				break;
				
			case kCMIODevicePropertyTransportType:
				answer = false;
				break;
				
			case kCMIODevicePropertyDeviceIsAlive:
				answer = false;
				break;
				
			case kCMIODevicePropertyDeviceHasChanged:
				answer = false;
				break;
				
			case kCMIODevicePropertyDeviceIsRunning:
				answer = false;
				break;
				
			case kCMIODevicePropertyDeviceIsRunningSomewhere:
				answer = false;
				break;
				
			case kCMIODevicePropertyDeviceCanBeDefaultDevice:
				answer = false;
				break;
				
			case kCMIODevicePropertyHogMode:
				answer = false;
				break;
				
			case kCMIODevicePropertyLatency:
				answer = false;
				break;
				
			case kCMIODevicePropertyStreams:
				answer = false;
				break;
				
			case kCMIODevicePropertyStreamConfiguration:
				answer = false;
				break;
				
			case kCMIODevicePropertyExcludeNonDALAccess:
				answer = true;
				break;
				
			case kCMIODevicePropertyCanProcessAVCCommand:
				answer = false;
				break;
				
			case kCMIODevicePropertyCanProcessRS422Command:
				answer = false;
				break;
				
			case kCMIODevicePropertyLinkedCoreAudioDeviceUID:
				answer = false;
				break;
				
			default:
				answer = Object::IsPropertySettable(address);
				break;
		};
		
		return answer;
	}

	UInt32 Device::GetPropertyDataSize(const CMIOObjectPropertyAddress& address, UInt32 qualifierDataSize, const void* qualifierData) const
	{
		UInt32 answer = 0;
		
		// Note that the DAL's API calls HasProperty before calling GetPropertyDataSize. This means that it can be assumed that address is valid for the property involved.
					
		// Take and hold the state mutex
		CAMutex::Locker stateMutex(const_cast<Device*>(this)->GetStateMutex());
		
		switch (address.mSelector)
		{
			case kCMIOObjectPropertyName:
				answer = sizeof(CFStringRef);
				break;
				
			case kCMIOObjectPropertyManufacturer:
				answer = sizeof(CFStringRef);
				break;
				
			case kCMIOObjectPropertyElementCategoryName:
				answer = sizeof(CFStringRef);
				break;
				
			case kCMIOObjectPropertyElementNumberName:
				answer = sizeof(CFStringRef);
				break;
				
			case kCMIODevicePropertyPlugIn:
				answer = sizeof(CMIOObjectID);
				break;
				
			case kCMIODevicePropertyDeviceUID:
				answer = sizeof(CFStringRef);
				break;
				
			case kCMIODevicePropertyModelUID:
				answer = sizeof(CFStringRef);
				break;
				
			case kCMIODevicePropertyTransportType:
				answer = sizeof(UInt32);
				break;
				
			case kCMIODevicePropertyDeviceIsAlive:
				answer = sizeof(UInt32);
				break;
				
			case kCMIODevicePropertyDeviceHasChanged:
				answer = sizeof(UInt32);
				break;
				
			case kCMIODevicePropertyDeviceIsRunning:
				answer = sizeof(UInt32);
				break;
				
			case kCMIODevicePropertyDeviceIsRunningSomewhere:
				answer = sizeof(UInt32);
				break;
				
			case kCMIODevicePropertyDeviceCanBeDefaultDevice:
				answer = sizeof(UInt32);
				break;
				
			case kCMIODevicePropertyHogMode:
				answer = sizeof(pid_t);
				break;
				
			case kCMIODevicePropertyLatency:
				answer = sizeof(UInt32);
				break;
				
			case kCMIODevicePropertyStreams:
				answer = sizeof(CMIOStreamID) * GetNumberStreams(address.mScope);
				break;
				
			case kCMIODevicePropertyStreamConfiguration:
				answer = sizeof(UInt32) + (sizeof(UInt32) * GetNumberStreams(address.mScope));
				break;
				
			case kCMIODevicePropertyExcludeNonDALAccess:
				answer = sizeof(UInt32);
				break;
				
			case kCMIODevicePropertyCanProcessAVCCommand:
				answer = sizeof(Boolean);
				break;
				
			case kCMIODevicePropertyCanProcessRS422Command:
				answer = sizeof(Boolean);
				break;
				
			case kCMIODevicePropertyLinkedCoreAudioDeviceUID:
				answer = sizeof(CFStringRef);
				break;
				
			default:
				answer = Object::GetPropertyDataSize(address, qualifierDataSize, qualifierData);
				break;
		};
		
		return answer;
	}

	void Device::GetPropertyData(const CMIOObjectPropertyAddress& address, UInt32 qualifierDataSize, const void* qualifierData, UInt32 dataSize, UInt32& dataUsed, void* data) const
	{
		Stream* stream = NULL;
		UInt32 index = 0;
		
		// Note that the DAL's API calls HasProperty before calling GetPropertyDataSize. This means that it can be assumed that address is valid for the property involved.
					
		// Take and hold the state mutex
		CAMutex::Locker stateMutex(const_cast<Device*>(this)->GetStateMutex());
		
		switch (address.mSelector)
		{
			case kCMIOObjectPropertyName:
				ThrowIf(dataSize != GetPropertyDataSize(address, qualifierDataSize, qualifierData), CAException(kCMIOHardwareBadPropertySizeError), "CMIO::DP::Device::GetPropertyData: wrong data size for kCMIOObjectPropertyName");
				*static_cast<CFStringRef*>(data) = CopyDeviceName();
				dataUsed = sizeof(CFStringRef);
				break;
				
			case kCMIOObjectPropertyManufacturer:
				ThrowIf(dataSize != GetPropertyDataSize(address, qualifierDataSize, qualifierData), CAException(kCMIOHardwareBadPropertySizeError), "CMIO::DP::Device::GetPropertyData: wrong data size for kCMIOObjectPropertyManufacturer");
				*static_cast<CFStringRef*>(data) = CopyDeviceManufacturerName();
				dataUsed = sizeof(CFStringRef);
				break;
				
			case kCMIOObjectPropertyElementCategoryName:
				ThrowIf(dataSize != GetPropertyDataSize(address, qualifierDataSize, qualifierData), CAException(kCMIOHardwareBadPropertySizeError), "CMIO::DP::Device::GetPropertyData: wrong data size for kCMIOObjectPropertyElementCategoryName");
				*static_cast<CFStringRef*>(data) = CopyElementCategoryName(address);
				dataUsed = sizeof(CFStringRef);
				break;
				
			case kCMIOObjectPropertyElementNumberName:
				ThrowIf(dataSize != GetPropertyDataSize(address, qualifierDataSize, qualifierData), CAException(kCMIOHardwareBadPropertySizeError), "CMIO::DP::Device::GetPropertyData: wrong data size for kCMIOObjectPropertyElementNumberName");
				*static_cast<CFStringRef*>(data) = CopyElementNumberName(address);
				dataUsed = sizeof(CFStringRef);
				break;
				
			case kCMIODevicePropertyPlugIn:
				ThrowIf(dataSize != GetPropertyDataSize(address, qualifierDataSize, qualifierData), CAException(kCMIOHardwareBadPropertySizeError), "CMIO::DP::Device::GetPropertyData: wrong data size for kCMIODevicePropertyPlugIn");
				*static_cast<CMIOObjectID*>(data) = GetPlugIn().GetObjectID();
				dataUsed = sizeof(CMIOObjectID);
				break;
				
			case kCMIODevicePropertyDeviceUID:
				ThrowIf(dataSize != GetPropertyDataSize(address, qualifierDataSize, qualifierData), CAException(kCMIOHardwareBadPropertySizeError), "CMIO::DP::Device::GetPropertyData: wrong data size for kCMIODevicePropertyDeviceUID");
				*static_cast<CFStringRef*>(data) = CopyDeviceUID();
				dataUsed = sizeof(CFStringRef);
				break;
				
			case kCMIODevicePropertyModelUID:
				ThrowIf(dataSize != GetPropertyDataSize(address, qualifierDataSize, qualifierData), CAException(kCMIOHardwareBadPropertySizeError), "CMIO::DP::Device::GetPropertyData: wrong data size for kCMIODevicePropertyModelUID");
				*static_cast<CFStringRef*>(data) = CopyModelUID();
				dataUsed = sizeof(CFStringRef);
				break;
				
			case kCMIODevicePropertyTransportType:
				ThrowIf(dataSize != GetPropertyDataSize(address, qualifierDataSize, qualifierData), CAException(kCMIOHardwareBadPropertySizeError), "CMIO::DP::Device::GetPropertyData: wrong data size for kCMIODevicePropertyTransportType");
				*static_cast<UInt32*>(data) = GetTransportType();
				dataUsed = sizeof(UInt32);
				break;
				
			case kCMIODevicePropertyDeviceIsAlive:
				ThrowIf(dataSize != GetPropertyDataSize(address, qualifierDataSize, qualifierData), CAException(kCMIOHardwareBadPropertySizeError), "CMIO::DP::Device::GetPropertyData: wrong data size for kCMIODevicePropertyDeviceIsAlive");
				*static_cast<UInt32*>(data) = IsAlive();
				dataUsed = sizeof(UInt32);
				break;
				
			case kCMIODevicePropertyDeviceHasChanged:
				ThrowIf(dataSize != GetPropertyDataSize(address, qualifierDataSize, qualifierData), CAException(kCMIOHardwareBadPropertySizeError), "CMIO::DP::Device::GetPropertyData: wrong data size for kCMIODevicePropertyDeviceHasChanged");
				*static_cast<UInt32*>(data) = 0;
				dataUsed = sizeof(UInt32);
				break;
				
			case kCMIODevicePropertyDeviceIsRunning:
				ThrowIf(dataSize != GetPropertyDataSize(address, qualifierDataSize, qualifierData), CAException(kCMIOHardwareBadPropertySizeError), "CMIO::DP::Device::GetPropertyData: wrong data size for kCMIODevicePropertyDeviceIsRunning");
				*static_cast<UInt32*>(data) = IsDeviceRunning() ? 1 : 0;
				dataUsed = sizeof(UInt32);
				break;
				
			case kCMIODevicePropertyDeviceIsRunningSomewhere:
				ThrowIf(dataSize != GetPropertyDataSize(address, qualifierDataSize, qualifierData), CAException(kCMIOHardwareBadPropertySizeError), "CMIO::DP::Device::GetPropertyData: wrong data size for kCMIODevicePropertyDeviceIsRunningSomewhere");
				*static_cast<UInt32*>(data) = IsDeviceRunningSomewhere() ? 1 : 0;
				dataUsed = sizeof(UInt32);
				break;
				
			case kCMIODevicePropertyDeviceCanBeDefaultDevice:
				ThrowIf(dataSize != GetPropertyDataSize(address, qualifierDataSize, qualifierData), CAException(kCMIOHardwareBadPropertySizeError), "CMIO::DP::Device::GetPropertyData: wrong data size for kCMIODevicePropertyDeviceCanBeDefaultDevice");
				*static_cast<UInt32*>(data) = CanBeDefaultDevice(address.mScope, false) ? 1 : 0;
				dataUsed = sizeof(UInt32);
				break;
				
			case kCMIODevicePropertyHogMode:
				ThrowIf(dataSize != GetPropertyDataSize(address, qualifierDataSize, qualifierData), CAException(kCMIOHardwareBadPropertySizeError), "CMIO::DP::Device::GetPropertyData: wrong data size for kCMIODevicePropertyHogMode");
				*static_cast<pid_t*>(data) = -1;
				dataUsed = sizeof(pid_t);
				break;
				
			case kCMIODevicePropertyLatency:
				ThrowIf(dataSize != GetPropertyDataSize(address, qualifierDataSize, qualifierData), CAException(kCMIOHardwareBadPropertySizeError), "CMIO::DP::Device::GetPropertyData: wrong data size for kCMIODevicePropertyLatency");
				*static_cast<UInt32*>(data) = GetLatency(address.mScope);
				dataUsed = sizeof(UInt32);
				break;
				
			case kCMIODevicePropertyStreams:
				{
					CMIOStreamID* streamList = static_cast<CMIOStreamID*>(data);
					UInt32 numberStreams = std::min((UInt32)(dataSize / sizeof(CMIOStreamID)), GetNumberStreams(address.mScope));
					for(index = 0; index < numberStreams ; ++index)
					{
						stream = GetStreamByIndex(address.mScope, index);
						streamList[index] = stream->GetObjectID();
					}
					dataUsed = numberStreams * sizeof(CMIOStreamID);
				}
				break;
				
			case kCMIODevicePropertyStreamConfiguration:
				{
					ThrowIf(dataSize != GetPropertyDataSize(address, qualifierDataSize, qualifierData), CAException(kCMIOHardwareBadPropertySizeError), "CMIO::DP::Device::GetPropertyData: wrong data size for kCMIODevicePropertyStreamConfiguration");
					CMIODeviceStreamConfiguration* streamConfiguration = static_cast<CMIODeviceStreamConfiguration*>(data);
					streamConfiguration->mNumberStreams = GetNumberStreams(address.mScope);
					for(index = 0; index < streamConfiguration->mNumberStreams; ++index)
					{
						stream = GetStreamByIndex(address.mScope, index);
						streamConfiguration->mNumberChannels[index] = stream->GetCurrentNumberChannels();
					}
					dataUsed = dataSize;
				}
				break;
				
			case kCMIODevicePropertyExcludeNonDALAccess:
				ThrowIf(dataSize != GetPropertyDataSize(address, qualifierDataSize, qualifierData), CAException(kCMIOHardwareBadPropertySizeError), "CMIO::DP::Device::GetPropertyData: wrong data size for kCMIODevicePropertyExcludeNonDALAccess");
				*static_cast<UInt32*>(data) = GetExcludeNonDALAccess() ? 1 : 0;
				dataUsed = sizeof(UInt32);
				break;
				
			case kCMIODevicePropertyCanProcessAVCCommand:
				ThrowIf(dataSize != GetPropertyDataSize(address, qualifierDataSize, qualifierData), CAException(kCMIOHardwareBadPropertySizeError), "CMIO::DP::Device::GetPropertyData: wrong data size for kCMIODevicePropertyCanProcessAVCCommand");
				*static_cast<Boolean*>(data) = false;
				dataUsed = sizeof(Boolean);
				break;
				
			case kCMIODevicePropertyCanProcessRS422Command:
				ThrowIf(dataSize != GetPropertyDataSize(address, qualifierDataSize, qualifierData), CAException(kCMIOHardwareBadPropertySizeError), "CMIO::DP::Device::GetPropertyData: wrong data size for kCMIODevicePropertyCanProcessRS422Command");
				*static_cast<Boolean*>(data) = false;
				dataUsed = sizeof(Boolean);
				break;
				
			default:
				Object::GetPropertyData(address, qualifierDataSize, qualifierData, dataSize, dataUsed, data);
				break;
		};
	}

	void Device::SetPropertyData(const CMIOObjectPropertyAddress& address, UInt32 qualifierDataSize, const void* qualifierData, UInt32 dataSize, const void* data)
	{
		// Note that the DAL's API calls HasProperty before calling GetPropertyDataSize. This means that it can be assumed that address is valid for the property involved.
					
		// Take and hold the state mutex
		CAMutex::Locker stateMutex(GetStateMutex());
		
		switch (address.mSelector)
		{
			case kCMIODevicePropertyExcludeNonDALAccess:
				{
					ThrowIf(dataSize != GetPropertyDataSize(address, qualifierDataSize, qualifierData), CAException(kCMIOHardwareBadPropertySizeError), "CMIO::DP::Device::GetPropertyData: wrong data size for kCMIODevicePropertyDeviceIsRunning");
					SetExcludeNonDALAccess(*static_cast<const UInt32*>(data) != 0);
					PropertyAddress changedAddress(kCMIODevicePropertyExcludeNonDALAccess);
					PropertiesChanged(1, &changedAddress);
				}
				break;
				
			default:
				Object::SetPropertyData(address, qualifierDataSize, qualifierData, dataSize, data);
				break;
		};
	}

	void Device::ExecuteCommand(Command* command)
	{
		// See if we can execute the command immediately
		if (IsSafeToExecuteCommand(command))
		{
			OSStatus error = 0;
			
			// We can, so do so and toss the object
			void* savedCommandState;
			if (StartCommandExecution(&savedCommandState))
			{
				try
				{
					command->Execute(this);
				}
				catch(const CAException exception)
				{
					error = exception.GetError();
				}
				catch(...)
				{
					error = kCMIOHardwareUnspecifiedError;
				}
				FinishCommandExecution(savedCommandState);
			}
			delete command;
			if (error != 0)
			{
				throw CAException(error);
			}
		}
		else
		{
			// We can't, so put it in the list for later execution
			mCommandList.push_back(command);
		}
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// ExecuteAllCommands()
	//	This routine must always be called at a time when it is safe to execute commands. This routine should empty the list regardless of whether all the commands have been executed.
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void Device::ExecuteAllCommands()
	{
		
		CommandList::iterator iterator = mCommandList.begin();
		while (iterator != mCommandList.end())
		{
			Command* command = *iterator;
			
			try
			{
				command->Execute(this);
			}
			catch(...)
			{
			}
			delete command;
			
			std::advance(iterator, 1);
		}
		
		mCommandList.clear();
	}

	void Device::ClearAllCommands()
	{
		CommandList::iterator iterator = mCommandList.begin();
		while (iterator != mCommandList.end())
		{
			Command* command = *iterator;
			
			delete command;
			
			std::advance(iterator, 1);
		}
		
		mCommandList.clear();
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// IsSafeToExecuteCommand()
	//	Override this routine to determine whether or not the given command can be executed immediately in the current context.
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	bool Device::IsSafeToExecuteCommand(Command* /*command*/)
	{
		return true;
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// StartCommandExecution()
	//	Called prior to exectuting each command subclasses should override in order to lock any required locks and save any state in the provided pointer returns whether or not commands 
	//	should be executed
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	bool Device::StartCommandExecution(void** /*savedCommandState*/)
	{
		return true;
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// FinishCommandExecution()
	//	Called prior to exectuting each command subclasses should override in order to unlock any required locks using the state provided
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void Device::FinishCommandExecution(void* /*savedCommandState*/)
	{
	}

	UInt32 Device::GetLatency(CMIOObjectPropertyScope /*scope*/) const
	{
		return 0;
	}

	bool Device::IsDeviceRunningSomewhere() const
	{
		return IsDeviceRunning();
	}

	Float64 Device::GetCurrentNominalSampleRate() const
	{
		// All streams have to have the same sample rate, so ask the first stream
		Float64 answer = 0.0;
		Stream* stream = NULL;
		
		if (HasInputStreams())
		{
			stream = GetStreamByIndex(kCMIODevicePropertyScopeInput, 0);
		}
		else if (HasOutputStreams())
		{
			stream = GetStreamByIndex(kCMIODevicePropertyScopeOutput, 0);
		}
		
		if (stream != NULL)
		{
//			answer = stream->GetCurrentNominalSampleRate();
		}
		
		return answer;
	}

	Float64 Device::GetCurrentActualSampleRate() const
	{
		//	This routine should return the measured sample rate of the device when IO is running.
		return GetCurrentNominalSampleRate();
	}
	
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// Suspend()
	//	Called when receiving a user logout.
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void Device::Suspend() 
	{
		try
		{
			// Lock the state mutex
			CAMutex::Locker stateMutex(GetStateMutex());
			
			// Don't do anything if already suspended
			if (mSuspended) 
				return;
			
			mSuspended = true;

			// Empty the suspended streams list
			mSuspendedInputStreams.erase(mSuspendedInputStreams.begin(), mSuspendedInputStreams.end());
			mSuspendedOutputStreams.erase(mSuspendedOutputStreams.begin(), mSuspendedOutputStreams.end());
			
			// Store the IDs of any streaming input streams so they can be resumed 
			for (StreamList::const_iterator iterator = mInputStreamList.begin(); iterator != mInputStreamList.end(); ++iterator)
			{
				if ((**iterator).Streaming())
					mSuspendedInputStreams.insert(mSuspendedInputStreams.begin(), (**iterator).GetObjectID());
			}
			
			// Store the IDs of any streaming output streams so they can be resumed 
			for (StreamList::const_iterator iterator = mOutputStreamList.begin(); iterator != mOutputStreamList.end(); ++iterator)
			{
				if ((**iterator).Streaming())
					mSuspendedOutputStreams.insert(mSuspendedInputStreams.begin(), (**iterator).GetObjectID());
			}
			
			// Stop all the streams 
			StopAllStreams();
			
			// Release the device master if needed
			mTakeDeviceMasterOnResumption = DeviceMasterIsOwnedBySelf();
			if (mTakeDeviceMasterOnResumption)
				SetDeviceMaster(-1);

			// Restore non-DAL access if needed 
			mExcludeNonDALAccessOnResumption = GetExcludeNonDALAccess();
			if (mExcludeNonDALAccessOnResumption)
				SetExcludeNonDALAccess(false);
		}
		catch(...)
		{
			// An error occured
			DebugMessageLevel(2, "CMIO::DP::Device:Suspend: exception");
		}
	}
	
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// Resume()
	//	Called when receiving a user login.
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void Device::Resume() 
	{
		{
			// Lock the state mutex
			CAMutex::Locker stateMutex(GetStateMutex());
			
			if (not mSuspended) 
				return;
			
			mSuspended = false;
		}
		
		// Wait for up to three seconds until HogModeIsOwnedBySelfOrIsFree()
		for (UInt32 tryHogMode = 0; (tryHogMode < 6 and not HogModeIsOwnedBySelfOrIsFree()); ++tryHogMode) 
			usleep(500000);
		
		// Do not attempt to resume if the hog mode is not yet free
		if (not HogModeIsOwnedBySelfOrIsFree())
			return;
		
		try
		{
			// Lock the state mutex
			CAMutex::Locker stateMutex(GetStateMutex());

			// Restart any suspended input streams
			for (StreamIDList::const_iterator iterator = mSuspendedInputStreams.begin(); iterator != mSuspendedInputStreams.end(); ++iterator)
			{
				// Ensure the stream still exist
				if (GetStreamByID(kCMIODevicePropertyScopeInput, *iterator))
					StartStream(*iterator);
			}
			
			// Restart any suspended output streams
			for (StreamIDList::const_iterator iterator = mSuspendedOutputStreams.begin(); iterator != mSuspendedOutputStreams.end(); ++iterator)
			{
				// Ensure the sream still exist
				if (GetStreamByID(kCMIODevicePropertyScopeOutput, *iterator))
					StartStream(*iterator);
			}
			
			// Exclude non-DAL access if needed
			if (mExcludeNonDALAccessOnResumption)
				SetExcludeNonDALAccess(true);
			
			// Become the device master if needed
			if (mTakeDeviceMasterOnResumption)
				SetDeviceMaster(CAProcess::GetPID());
		}
		catch(...)
		{
			// Something went wrong, so stop all the streams
			DebugMessageLevel(2, "CMIO::DP::Device:Resume: something re");
			StopAllStreams();
		}
	}

	Stream* Device::GetStreamByID(CMIOObjectPropertyScope scope, CMIOStreamID id) const
	{
		// Initialize the return value
		Stream* answer = NULL;
		
		// Figure which stream list is involved
		const StreamList& streamList = (kCMIODevicePropertyScopeInput == scope) ? mInputStreamList : mOutputStreamList;
		
		// Iterate through the streams
		StreamList::const_iterator iterator = streamList.begin();
		while ((iterator != streamList.end()) && (answer == NULL))
		{
			// Get the stream
			Stream* stream = *iterator;
			
			// Figure out if the device channel falls in it's range
			if (stream->GetObjectID() == id)
			{
				// It does, set the return value
				answer = stream;
			}
			else
			{
				// It doesn't, get the next item
				std::advance(iterator, 1);
			}
		}
		
		return answer;
	}

	Stream* Device::GetStreamByIndex(CMIOObjectPropertyScope scope, UInt32 index) const
	{
		// Initialize the return value
		Stream* answer = NULL;
		
		// Figure which stream list is involved
		const StreamList& streamList = (kCMIODevicePropertyScopeInput == scope) ? mInputStreamList : mOutputStreamList;
		
		// Make sure the index is in the proper range
		if (index < streamList.size())
		{
			// Get the stream
			answer = streamList.at(index);
		}
		
		return answer;
	}

	Stream* Device::GetStreamByDeviceChannel(CMIOObjectPropertyScope scope, UInt32 deviceChannel) const
	{
		// Initialize the return value
		Stream* answer = NULL;
		
		// Figure which stream list is involved
		const StreamList& streamList = (kCMIODevicePropertyScopeInput == scope) ? mInputStreamList : mOutputStreamList;
		
		// Iterate through the streams
		StreamList::const_iterator iterator = streamList.begin();
		while ((iterator != streamList.end()) && (answer == NULL))
		{
			// Get the stream
			Stream* stream = *iterator;
			
			// Figure out if the device channel falls in it's range
			if (deviceChannel < (stream->GetStartingDeviceChannelNumber() + stream->GetCurrentNumberChannels()))
			{
				// It does, set the return value
				answer = stream;
			}
			else
			{
				// It doesn't, get the next item
				std::advance(iterator, 1);
			}
		}
		
		return answer;
	}

	Stream* Device::GetStreamByPropertyAddress(const CMIOObjectPropertyAddress& address, bool tryRealHard) const
	{
		// Initialize the return value
		Stream* answer = NULL;
		
		// Figure which stream list is involved
		const StreamList* streamList = NULL;
		switch (address.mScope)
		{
			case kCMIODevicePropertyScopeInput:
				streamList = &mInputStreamList;
				break;
				
			case kCMIODevicePropertyScopeOutput:
				streamList = &mOutputStreamList;
				break;
			
			case kCMIOObjectPropertyScopeGlobal:
				if (mOutputStreamList.size() > 0)
				{
					streamList = &mOutputStreamList;
				}
				else if (mInputStreamList.size() > 0)
				{
					streamList = &mInputStreamList;
				}
				break;
		};
		
		// Iterate through the streams
		if (streamList != NULL)
		{
			StreamList::const_iterator iterator = streamList->begin();
			while ((iterator != streamList->end()) && (answer == NULL))
			{
				// Get the stream
				Stream* stream = *iterator;
				
				// Figure out if the device channel falls in it's range
				if (address.mElement < (stream->GetStartingDeviceChannelNumber() + stream->GetCurrentNumberChannels()))
				{
					// It does, set the return value
					answer = stream;
				}
				else
				{
					// It doesn't, get the next item
					std::advance(iterator, 1);
				}
			}
		}
		
		// If we still haven't found a stream and we've been told to try hard, just pick one.
		if (tryRealHard && (answer == NULL))
		{
			if (HasOutputStreams())
			{
				answer = GetStreamByIndex(kCMIODevicePropertyScopeOutput, 0);
			}
			else if (HasInputStreams())
			{
				answer = GetStreamByIndex(kCMIODevicePropertyScopeInput, 0);
			}
		}
		
		return answer;
	}

	UInt32 Device::GetTotalNumberChannels(CMIOObjectPropertyScope scope) const
	{
		// Initialize the return value
		UInt32 answer = 0;
		
		// Figure which stream list is involved
		const StreamList& streamList = (kCMIODevicePropertyScopeInput == scope) ? mInputStreamList : mOutputStreamList;
		
		// Iterate through the streams
		StreamList::const_iterator iterator = streamList.begin();
		while (iterator != streamList.end())
		{
			// Get the stream
			Stream* stream = *iterator;
			
			// Add it's number of channels
			answer += stream->GetCurrentNumberChannels();
			
			// Get the next one
			std::advance(iterator, 1);
		}
		
		return answer;
	}

	void Device::AddStream(Stream* stream)
	{
		// Figure which stream list is involved
		StreamList& streamList = stream->IsInput() ? mInputStreamList : mOutputStreamList;
		
		// Find the first stream with a starting channel number greater than the one being added
		UInt32 startingChannelNumber = stream->GetStartingDeviceChannelNumber();
		StreamList::iterator iterator = streamList.begin();
		while ((iterator != streamList.end()) && ((*iterator)->GetStartingDeviceChannelNumber() < startingChannelNumber))
		{
			std::advance(iterator, 1);
		}
		
		// Shove the new stream in front of it
		streamList.insert(iterator, stream);
	}

	void Device::RemoveStream(Stream* stream)
	{
		// Figure which stream list is involved
		StreamList& streamList = stream->IsInput() ? mInputStreamList : mOutputStreamList;
		
		// Find the stream in the list
		StreamList::iterator iterator = std::find(streamList.begin(), streamList.end(), stream);
		if (iterator != streamList.end())
		{
			// And remove it
			streamList.erase(iterator);
		}
	}

	Control* Device::GetControlByClassID(CMIOClassID controlClassID, CMIOObjectPropertyScope controlScope, CMIOObjectPropertyElement controlElement) const
	{
		Control* answer = NULL;
		
		// Look for it in the list
		ControlList::const_iterator controlIterator = mControlList.begin();
		while ((answer == NULL) && (controlIterator != mControlList.end()))
		{
			// Get the control
			Control* currentControl = *controlIterator;
			
			// Get the info about it
			CMIOClassID currentControlClassID = currentControl->GetClassID();
			CMIOObjectPropertyScope currentControlScope = currentControl->GetPropertyScope();
			CMIOObjectPropertyElement currentControlElement = currentControl->GetPropertyElement();
			
			// See if it's the one we want
			if (controlElement != 0)
			{
				if ((currentControlClassID == controlClassID) && (currentControlScope == controlScope) && (currentControlElement == controlElement))
				{
					// It is
					answer = currentControl;
				}
				else
				{
					// Go to the next one
					std::advance(controlIterator, 1);
				}
			}
			else
			{
				// Master clock control are always global regardless of their scope
				if ((currentControlClassID == controlClassID) && (currentControlElement == controlElement))
				{
					// It is
					answer = currentControl;
				}
				else
				{
					// Go to the next one
					std::advance(controlIterator, 1);
				}
			}
		}
		
		return answer;
	}

	void Device::AddControl(Control* control)
	{
		mControlList.push_back(control);
	}

	void Device::RemoveControl(Control* control)
	{
		ControlList::iterator iterator = std::find(mControlList.begin(), mControlList.end(), control);
		if (iterator != mControlList.end())
		{
			mControlList.erase(iterator);
		}
	}

	void Device::ClearControlMarks()
	{
		ControlList::iterator iterator = mControlList.begin();
		while (iterator != mControlList.end())
		{
			(*iterator)->SetMark(false);
			std::advance(iterator, 1);
		}
	}
	void	Device::SetControlValue(UInt32 inControlID, UInt32 inValue, UInt32* outNewValue)
	{
		// must be overriden
	}
	CFDictionaryRef Device::CopyControlDictionaryByControlID(UInt32 inControlID) const
	{
		// must be overriden
		return NULL;
	}

}}
