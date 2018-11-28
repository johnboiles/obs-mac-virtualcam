/*
	    File: CMIO_DP_Sample_Device.cpp
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
// includes
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

// Self Include
#include "CMIO_DP_Sample_Device.h"

// Internal Includes
#include "CMIO_DP_Sample_PlugIn.h"
#include "CMIO_DP_Sample_Stream.h"
#include "CMIO_DP_IOV_Control.h"

// DAL PlugIn Base Includes
#include "CMIO_DP_DeviceSettings.h"

// Properties
#include "CMIO_DP_Property_ClientSyncDiscontinuity.h"
#include "CMIO_DP_Property_HogMode.h"

// Public Utility Includes
#include "CMIODebugMacros.h"
#include "CMIO_Buffer.h"

// CA Public Utility Includes
#include "CACFMachPort.h"
#include "CAException.h"
#include "CAHostTimeBase.h"
#include "CALogMacros.h"

// System Includes
#include <CoreMediaIO/CMIOHardware.h>
#include <IOKit/IOMessage.h>

#define Log_HardwareStartStop 0

#pragma mark -
namespace CMIO { namespace DP { namespace Sample
{
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// Device()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	Device::Device(PlugIn& plugIn, CMIODeviceID deviceID, mach_port_t assistantPort, UInt64 guid, const io_string_t registryPath) :
		DP::Device(deviceID, plugIn),
		mDeviceGUID(guid),
		mRegistryEntry(IORegistryEntryFromPath(kIOMasterPortDefault, registryPath)), 
		mAssistantPort(assistantPort),
        mDeviceUID(CFStringCreateWithFormat(0, 0, CFSTR("%#16llx-SampleVideo"), guid)),
		mDeviceName(CFSTR("Sample"), false),						// ••• A proper name and device manufacturer should also be reported
		mDeviceManufacturerName(CFSTR("Apple"), false),				// ••• Need to determine a the proper manufacturer provide the manufacturer
		mPropertyCacheTime(0),
		mHogMode(NULL),
		mClientSyncDiscontinuity(NULL),
		mSMPTETimeCallback(NULL),
		mDeviceIsRunningSomewhere(DPA::Sample::GetDeviceIsRunningSomewhere(GetAssistantPort(), GetDeviceGUID())),
		mControlCacheTime(0),
		mEventPort(reinterpret_cast<CFMachPortCallBack>(Event), this)
	{
		// Make sure the registry entry is valid
		ThrowIf(not mRegistryEntry.IsValid(), CAException(kIOReturnBadArgument), "CMIO::DP::Sample::Device: invalid registry entry");
		
		// Set DP::Device's mExcludeNonDALAccess based on whether or not non-DAL processes are being excluded
		mExcludeNonDALAccess = DPA::Sample::GetExcludeNonDALAccess(GetAssistantPort(), GetDeviceGUID());
	}
	
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// ~Device()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	Device::~Device()
	{
	}
	
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// Initialize()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void Device::Initialize()
	{
		// italize the super class
		DP::Device::Initialize();

		// Take and hold the state mutex
		CAMutex::Locker stateMutex(GetStateMutex());

		try
		{
			// Create the streams
			CreateStreams(kCMIODevicePropertyScopeInput);
			CreateStreams(kCMIODevicePropertyScopeOutput);
		
			// Create the properties
			CreateProperties();

			// Create the controls
			CreateRegistryControls();
			
			CreatePluginControls();
			
			// Add  event port's run loop source to the DAL's sources
			CFRunLoopAddSource(CFRunLoopGetMain(), GetEventPort().GetRunLoopSource(), kCFRunLoopCommonModes);
		}
		catch (...)
		{
			// Something went wrong, so tear everything down (these are all safe NOPs if the items in question had never been created)
			CFRunLoopRemoveSource(CFRunLoopGetMain(), GetEventPort().GetRunLoopSource(), kCFRunLoopCommonModes);

			ReleaseProperties();
			ReleaseControls(true);
			ReleaseStreams(true);

			// Teardown the super class
			DP::Device::Teardown();
			
			throw;
		}
	}
	
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// Teardown()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void Device::Teardown()
	{
		// Remove the event loop run loop source from the DAL's sources
		CFRunLoopRemoveSource(CFRunLoopGetMain(), GetEventPort().GetRunLoopSource(), kCFRunLoopCommonModes);
			
		// Release the properties, controls, and streams
		ReleaseProperties();
		ReleaseControls();
		ReleaseStreams();	
		
		// Teardown the super class
		DP::Device::Teardown();
	}
	
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// Finalize()
	//	Finalize() is called in place of Teardown() when being lazy about cleaning up. The idea is to do as little work as possible.
	//	••• This needs to be revisted to make sure enough is actually being done here.
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void Device::Finalize()
	{
		// Finalize the input streams
		UInt32 numberStreams = GetNumberStreams(kCMIODevicePropertyScopeInput);
		for (UInt32 streamIndex = 0; streamIndex != numberStreams; ++streamIndex)
		{
			Stream* stream = static_cast<Stream*>(GetStreamByIndex(kCMIODevicePropertyScopeInput, streamIndex));
			stream->Finalize();
		}
		
		// Finalize the output streams
		numberStreams = GetNumberStreams(kCMIODevicePropertyScopeOutput);
		for (UInt32 streamIndex = 0; streamIndex != numberStreams; ++streamIndex)
		{
			Stream* stream = static_cast<Stream*>(GetStreamByIndex(kCMIODevicePropertyScopeOutput, streamIndex));
			stream->Finalize();
		}
	}

	#pragma mark -
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// CopyModelUID()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	CFStringRef	Device::CopyDeviceName() const
	{
		return mDeviceName.CopyCFString();
	}
	
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// CopyModelUID()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	CFStringRef	Device::CopyModelUID() const
	{
		// ••• At some point in the future, if the model of the device can be determined n a unique ID should be returned		
		return NULL;
	}
	
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// HogModeIsOwnedBySelfOrIsFree()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	bool Device::HogModeIsOwnedBySelfOrIsFree() const
	{
		return  mHogMode->IsFree();
	}
	
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// SetDeviceMaster()
	//	This gets called from the CMIO::DP::Property::DeviceMaster::SetPropertyData().
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void Device::SetDeviceMaster(pid_t masterPID)
	{
		// Instruct the Assistant to try and set the DeviceMaster
		DPA::Sample::SetDeviceMaster(GetAssistantPort(), GetDeviceGUID(), masterPID);
	}
	
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// SetExcludeNonDALAccess()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void Device::SetExcludeNonDALAccess(bool excludeNonDALAccess)
	{
		// No need to do anything if there are no changes
		if (mExcludeNonDALAccess == excludeNonDALAccess)
			return;
			
		// Attempt to exclude / allow non-DAL access
		DPA::Sample::SetExcludeNonDALAccess(GetAssistantPort(), GetDeviceGUID(), excludeNonDALAccess);
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// SetForceDiscontinuity()
	//	This gets called from the CMIO::DP::Property::ClientSyncDiscontinuity::SetPropertyData().
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void Device::SetForceDiscontinuity(Boolean forceDiscontinuity)
	{
		DPA::Sample::SetClientSyncDiscontinuity(GetAssistantPort(), GetDeviceGUID(), forceDiscontinuity);
	}

	#pragma mark -
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// HasProperty()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	bool Device::HasProperty(const CMIOObjectPropertyAddress& address) const
	{
		bool answer = false;
		
		// Take and hold the state mutex
		CAMutex::Locker stateMutex(const_cast<Device*>(this)->GetStateMutex());
		
		switch (address.mSelector)
		{
			case kCMIODevicePropertyLinkedCoreAudioDeviceUID:
			case kCMIODevicePropertyLinkedAndSyncedCoreAudioDeviceUID:
				answer = true;
				break;
			
			default:
				answer = DP::Device::HasProperty(address);
				break;
		};
		
		return answer;
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// IsPropertySettable()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	bool Device::IsPropertySettable(const CMIOObjectPropertyAddress& address) const
	{
		bool answer = false;
		
		// Take and hold the state mutex
		CAMutex::Locker stateMutex(const_cast<Device*>(this)->GetStateMutex());
		
		switch (address.mSelector)
		{
			case kCMIODevicePropertyLinkedCoreAudioDeviceUID:
			case kCMIODevicePropertyLinkedAndSyncedCoreAudioDeviceUID:
				answer = false;
				break;
			
			default:
				answer = DP::Device::IsPropertySettable(address);
				break;
		};
		
		return answer;
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// GetPropertyDataSize()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	UInt32 Device::GetPropertyDataSize(const CMIOObjectPropertyAddress& address, UInt32 qualifierDataSize, const void* qualifierData) const
	{
		UInt32 answer = 0;
		
		// Take and hold the state mutex
		CAMutex::Locker stateMutex(const_cast<Device*>(this)->GetStateMutex());
		
		switch (address.mSelector)
		{
			case kCMIODevicePropertyLinkedCoreAudioDeviceUID:
			case kCMIODevicePropertyLinkedAndSyncedCoreAudioDeviceUID:
				answer = sizeof(CFStringRef);
				break;
				
			default:
				answer = DP::Device::GetPropertyDataSize(address, qualifierDataSize, qualifierData);
				break;
		};
		
		return answer;
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// GetPropertyData()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void Device::GetPropertyData(const CMIOObjectPropertyAddress& address, UInt32 qualifierDataSize, const void* qualifierData, UInt32 dataSize, UInt32& dataUsed, void* data) const
	{
		// Take and hold the state mutex
		CAMutex::Locker stateMutex(const_cast<Device*>(this)->GetStateMutex());
		
		switch (address.mSelector)
		{
			case kCMIODevicePropertyHogMode:
				ThrowIf(dataSize != GetPropertyDataSize(address, qualifierDataSize, qualifierData), CAException(kCMIOHardwareBadPropertySizeError), "CMIO::DP::Sample::Device::GetPropertyData: wrong data size for kCMIODevicePropertyHogMode");
				mHogMode->GetPropertyData(address, qualifierDataSize, qualifierData, dataSize, dataUsed, data);
				break;
				
			case kCMIODevicePropertyCanProcessRS422Command:
				ThrowIf(dataSize != GetPropertyDataSize(address, qualifierDataSize, qualifierData), CAException(kCMIOHardwareBadPropertySizeError), "CMIO::DP::Device::GetPropertyData: wrong data size for kCMIODevicePropertyCanProcessRS422Command");
				*static_cast<Boolean*>(data) = true;
				dataUsed = sizeof(Boolean);
				break;
				
			case kCMIODevicePropertyLinkedCoreAudioDeviceUID:
			case kCMIODevicePropertyLinkedAndSyncedCoreAudioDeviceUID:
				if (kCMIODevicePropertyScopeInput == address.mScope)
				{
					ThrowIf(dataSize != GetPropertyDataSize(address, qualifierDataSize, qualifierData), CAException(kCMIOHardwareBadPropertySizeError), "CMIO::DP::Sample::Device::GetPropertyData: wrong data size for kCMIO_DPA_Sample_StreamPropertyLinkedAndSyncedCoreAudioDeviceUID");
					// The UID of the linked audio device is simply the GUID formatted as shown below
					*(static_cast<CFStringRef*>(data)) = CFStringCreateWithCString(kCFAllocatorDefault, "Audio_BipBop_1", kCFStringEncodingASCII);
					dataUsed = sizeof(CFStringRef);
				}
				else
				{
					*(static_cast<CFStringRef*>(data)) = NULL;
					dataUsed = 0;
				}
				break;
				
			default:
				DP::Device::GetPropertyData(address, qualifierDataSize, qualifierData, dataSize, dataUsed, data);
				break;
		};
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// SetPropertyData()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void Device::SetPropertyData(const CMIOObjectPropertyAddress& address, UInt32 qualifierDataSize, const void* qualifierData, UInt32 dataSize, const void* data)
	{
		// Take and hold the state mutex
		CAMutex::Locker stateMutex(GetStateMutex());
		
		switch (address.mSelector)
		{
			default:
				DP::Device::SetPropertyData(address, qualifierDataSize, qualifierData, dataSize, data);
				break;
		};
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// PropertyListenerAdded()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void Device::PropertyListenerAdded(const CMIOObjectPropertyAddress& address)
	{
		Object::PropertyListenerAdded(address);
	}

	#pragma mark -
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// CreateProperties()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void Device::CreateProperties() 
	{
		// Remember when the properties were last cached
		mPropertyCacheTime = CAHostTimeBase::GetTheCurrentTime();
		
		// Set DP::Device's mDeviceMaster based on whether or not a process has acquired mastership
		mDeviceMaster->SetMaster(DPA::Sample::GetDeviceMaster(GetAssistantPort(), GetDeviceGUID()), false);

		// Use CMIO::DP::Property::HogMode (Implements the kCMIODevicePropertyHogMode property with a non-settable value of -1)
		mHogMode = new DP::Property::HogMode(*this, DPA::Sample::GetHogMode(GetAssistantPort(), GetDeviceGUID()));
		mHogMode->Initialize();
		AddProperty(mHogMode);

		// Use CMIO::DP::Property::ClientSyncDiscontinuity property to allow the client to flush the device's internal state
		mClientSyncDiscontinuity = new DP::Property::ClientSyncDiscontinuity(*this);
		mClientSyncDiscontinuity->Initialize();
		AddProperty(mClientSyncDiscontinuity);

		// Use the CMIO::DP::Property::SMPTETimeCallback property to get SMPTE timecode from the client
		mSMPTETimeCallback = new DP::Property::SMPTETimeCallback(*this);
		mSMPTETimeCallback->Initialize();
		AddProperty(mSMPTETimeCallback);

		// Call UpdatePropertyStates() to make sure everything is synced with the Assistant and to provide it the initial "send-once" right for future kPropertyStatesChanged messages
		UpdatePropertyStates();
	}	

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// ReleaseProperties()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void Device::ReleaseProperties() 
	{
		if (NULL != mHogMode)
		{
			RemoveProperty(mHogMode);
			mHogMode->Teardown();
			delete mHogMode;
			mHogMode = NULL;
		}

		if (NULL != mClientSyncDiscontinuity)
		{
			RemoveProperty(mClientSyncDiscontinuity);
			delete mClientSyncDiscontinuity;
			mClientSyncDiscontinuity = NULL;
		}

		if (NULL != mSMPTETimeCallback)
		{
			RemoveProperty(mSMPTETimeCallback);
			delete mSMPTETimeCallback;
			mSMPTETimeCallback = NULL;
		}
	} 

	#pragma mark -
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// GetControlByControlID()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	DP::Control* Device::GetControlByControlID(UInt32 controlID) const
	{
		// Iterate through the control list
		for(ControlList::const_iterator iterator = mControlList.begin() ; iterator != mControlList.end() ; std::advance(iterator, 1))
		{
			// Get the control object
			DP::Control* control = *iterator;
			
			// Get the control ID
			UInt32 theControlID = 0;
			switch (control->GetBaseClassID())
			{
				case kCMIOBooleanControlClassID:
					theControlID = ((IOV::BooleanControl*)control)->GetControlID();
					break;
					
				case kCMIOSelectorControlClassID:
					theControlID = ((IOV::SelectorControl*)control)->GetControlID();
					break;
			};
			
			// See if it's the one being looked for
			if (0 != theControlID and controlID == theControlID)
			{
				// It is
				return control;
			}
		}
		
		return NULL;
	}
	
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// SetControlValue()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void Device::SetControlValue(UInt32 controlID, UInt32 value, UInt32* newValue)
	{
		// Grab the muxtex for the device's state
		CAMutex::Locker locker(GetStateMutex());

		// Attempt to set the control
		DPA::Sample::SetControl(GetAssistantPort(), GetDeviceGUID(), controlID, value, newValue);
		
		// Update some internal state if the value changed synchronously
		if (value != *newValue)
		{
			
		}
		
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// CopyControlDictionaryByControlID()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	CFDictionaryRef	Device::CopyControlDictionaryByControlID(UInt32 controlID) const
	{
		CFDictionaryRef answer = NULL;
		
		// Get the control list from the IORegistry
		CACFArray controlList(static_cast<CFArrayRef>(IORegistryEntryCreateCFProperty(mRegistryEntry, CFSTR(kIOVideoDeviceKey_ControlList), NULL, 0)), true);
		
		// See if the IORegistry entry held the control dictionary
		if (controlList.IsValid())
		{
			// Iterate through the controls
			UInt32 controlCount = controlList.GetNumberItems();
			for (UInt32 index = 0; (NULL == answer) and (index < controlCount); ++index)
			{
				// Get the control dictionary
				CFDictionaryRef controls = NULL;
				if (controlList.GetDictionary(index, controls))
				{
					// Check to see if it's the one we're looking for
					if (controlID == IOV::Control::GetControlID(controls))
					{
						// It is
						answer = controls;
						CFRetain(answer);
					}
				}
			}
		}
		
		// If the control dictionary wasn't found in the IORegistry, ask the Assistant
		if (NULL == answer)
		{
			// Copy the control list from the Assistant
			CACFArray controlList(DPA::Sample::CopyControlList(GetAssistantPort(), GetDeviceGUID()), true);
			
			// Iterate through the controls
			UInt32 controlCount = controlList.GetNumberItems();
			for (UInt32 index = 0; (NULL == answer) and (index < controlCount); ++index)
			{
				// Get the control dictionary
				CFDictionaryRef controls = NULL;
				if (controlList.GetDictionary(index, controls))
				{
					// Check to see if it's the one we're looking for
					if (controlID == IOV::Control::GetControlID(controls))
					{
						// It is
						answer = controls;
						CFRetain(answer);
					}
				}
			}
		}
		
		return answer;
	}

	#pragma mark -
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// CreateRegistryControls()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void Device::CreateRegistryControls() 
	{
		// Create a vector of CMIOObjectIDs to hold the objects being created
		std::vector<CMIOObjectID> controlIDs;

		// Get the control list from the IORegistry
		CACFArray controlList = CACFArray(static_cast<CFArrayRef>(IORegistryEntryCreateCFProperty(mRegistryEntry, CFSTR(kIOVideoDeviceKey_ControlList), NULL, 0)), true);

		// Don't do anything if the list is not valid
		if (not controlList.IsValid())
			return;

		// Iterate over the controls
		UInt32 controlCount = controlList.GetNumberItems();
		for (UInt32 index = 0 ; index < controlCount ; ++index)
		{
			// Get the control dictionary
			CFDictionaryRef controlDictionary = NULL;
			if (controlList.GetDictionary(index, controlDictionary))
			{
				// Figure out what class the control is
				CMIOClassID classID = IOV::ControlDictionary::GetClassID(controlDictionary);
				
				// Create the control
				CMIOObjectID controlID;
				OSStatus err = CMIOObjectCreate(GetPlugIn().GetInterface(), GetObjectID(), classID, &controlID);
				if (0 == err)
				{
					try
					{
						// Create the control
						CFRetain(controlDictionary);
						DP::Control* control = IOV::Control::CreateControl(controlID, classID, controlDictionary, GetPlugIn(), *this);
						
						if (NULL != control)
						{
							// Add it to the list
							AddControl(control);
						
							// Store the new stream ID
							controlIDs.push_back(controlID);
						}
					}
					catch (...)
					{
						// Silently continue on to the next control
						DebugMessage("CMIO::DP::Sample::Device::CreateControls: failure createing control for %d", classID);
						continue;
					}
				}
			}
		}
		
		// Call UpdateControlStates() to make sure everything is synced with the Assistant and to provide it the initial "send-once" right for future kControlStatesChanged messages
		UpdateControlStates(false);

		// Tell the DAL about the new controls
		if (controlIDs.size() > 0)
		{
			// Set the object state mutexes
			for (std::vector<CMIOObjectID>::iterator iterator = controlIDs.begin(); iterator != controlIDs.end(); std::advance(iterator, 1))
			{
				Object* object = Object::GetObjectByID(*iterator);
				if (NULL != object)
				{
					Object::SetObjectStateMutexForID(*iterator, object->GetObjectStateMutex());
				}
			}
	
			OSStatus err = CMIOObjectsPublishedAndDied(GetPlugIn().GetInterface(), GetObjectID(), controlIDs.size(), &(controlIDs.front()), 0, NULL);
			ThrowIfError(err, CAException(err), "CMIO::DP::Sample::Device::CreateControls: couldn't tell the DAL about the controls");
		}
	}

	#pragma mark -
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// CreateControls()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void Device::CreatePluginControls() 
	{
		// Remember when the controls were last cached
		mControlCacheTime = CAHostTimeBase::GetTheCurrentTime();
		
		// Create a vector of CMIOObjectIDs to hold the objects being created
		std::vector<CMIOObjectID> controlIDs;

		// Copy the control list from the Assistant
		CACFArray controlList(DPA::Sample::CopyControlList(GetAssistantPort(), GetDeviceGUID()), true);
		
		// Iterate over the controls
		UInt32 controlCount = controlList.GetNumberItems();
		for (UInt32 index = 0 ; index < controlCount ; ++index)
		{
			// Get the control dictionary
			CFDictionaryRef controlDictionary = NULL;
			if (controlList.GetDictionary(index, controlDictionary))
			{
				
				// Figure out what class the control is
				CMIOClassID classID = IOV::ControlDictionary::GetClassID(controlDictionary);
				
				// Create the control
				CMIOObjectID controlID;
				OSStatus err = CMIOObjectCreate(GetPlugIn().GetInterface(), GetObjectID(), classID, &controlID);
				if (0 == err)
				{
					try
					{
						// Create the control
						CFRetain(controlDictionary);
						DP::Control* control = IOV::Control::CreateControl(controlID, classID, controlDictionary, GetPlugIn(), *this);
						
						if (NULL != control)
						{
							// Add it to the list
							AddControl(control);
							
							// Store the new stream ID
							controlIDs.push_back(controlID);
						}
					}
					catch (...)
					{
						// Silently continue on to the next control
						DebugMessage("CMIO::DP::Sample::Device::CreateControls: failure createing control for %d", classID);
						continue;
					}
				}
				
			}
		}

		// Tell the DAL about the new controls
		if (controlIDs.size() > 0)
		{
			// Set the object state mutexes
			for (std::vector<CMIOObjectID>::iterator i = controlIDs.begin() ; i != controlIDs.end() ; std::advance(i, 1))
			{
				Object* object = Object::GetObjectByID(*i);
				if (NULL != object)
				{
					Object::SetObjectStateMutexForID(*i, object->GetObjectStateMutex());
				}
			}
			
			OSStatus err = CMIOObjectsPublishedAndDied(GetPlugIn().GetInterface(), GetObjectID(), controlIDs.size(), &(controlIDs.front()), 0, NULL);
			ThrowIfError(err, CAException(err), "CMIO::DP::Sample::Device::CreateControls: couldn't tell the DAL about the controls");
		}
	}
	
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// ReleaseControls()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void Device::ReleaseControls(bool reportDeath) 
	{
		// Grab the muxtex for the device's state
		CAMutex::Locker locker(GetStateMutex());
	
		ControlList::iterator iterator = mControlList.begin();
		while (iterator != mControlList.end())
		{
			DP::Control* control = *iterator;
			CMIOObjectID objectID = control->GetObjectID();

			if (reportDeath)
				(void) CMIOObjectsPublishedAndDied(GetPlugIn().GetInterface(), GetObjectID(), 0, 0, 1, &objectID);
			
			control->Teardown();
			delete control;
			std::advance(iterator, 1);
		}
	}

	#pragma mark -
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// Event()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void Device::Event(CFMachPortRef port, mach_msg_header_t* header, CFIndex size, Device& device) 
	{
		// Don't let any exceptions	leave this callback
		try
		{	
			// Take and hold the state mutex
			CAMutex::Locker stateMutex(device.GetStateMutex());
		
			switch (header->msgh_id)
			{
				case DPA::Sample::kControlStatesChanged:
					{
						// Update the controls state
						device.UpdateControlStates(true);
					}
					break;

				case DPA::Sample::kPropertyStatesChanged:
					{
						// Update the properties state
						device.UpdatePropertyStates();
					}
					break;
			}
		}
		catch (...)
		{
		}
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// UpdatePropertyStates()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void Device::UpdatePropertyStates() 
	{
		// Grab the muxtex for the device's state
		CAMutex::Locker locker(GetStateMutex());

		// Get the current time to use to remember when the properties were cached
		UInt64 currentTime = CAHostTimeBase::GetTheCurrentTime();
					
		// Get the state of all the properties that have changed since the last time their values were cached
		DPA::Sample::AutoFreeUnboundedArray<PropertyAddress> addresses;
		PropertyAddress matchAddress(kCMIOObjectPropertySelectorWildcard, kCMIOObjectPropertyScopeWildcard, kCMIOObjectPropertyElementWildcard);
		DPA::Sample::GetProperties(GetAssistantPort(), GetDeviceGUID(), GetEventPort().GetMachPort(), mPropertyCacheTime, matchAddress, addresses);

		// Iterate over the array of addresses and update the properties that have changed 
		for (UInt32 i = 0 ; i < addresses.GetLength() ; ++i)
		{
			try
			{
				if (PropertyAddress::IsSameAddress(addresses[i], PropertyAddress(kCMIODevicePropertyDeviceIsRunningSomewhere)))
				{
					if (mDeviceIsRunningSomewhere != DPA::Sample::GetDeviceIsRunningSomewhere(GetAssistantPort(), GetDeviceGUID()))
					{
						mDeviceIsRunningSomewhere = not mDeviceIsRunningSomewhere;
						PropertyAddress address(kCMIODevicePropertyDeviceIsRunningSomewhere);
						PropertiesChanged(1, &address);
					}
				}
				else if (PropertyAddress::IsSameAddress(addresses[i], PropertyAddress(kCMIODevicePropertyHogMode)))
				{
					mHogMode->SetHogMode(DPA::Sample::GetHogMode(GetAssistantPort(), GetDeviceGUID()));
				}
				else if (PropertyAddress::IsSameAddress(addresses[i], PropertyAddress(kCMIODevicePropertyDeviceMaster)))
				{
					mDeviceMaster->SetMaster(DPA::Sample::GetDeviceMaster(GetAssistantPort(), GetDeviceGUID()));
				}
				else if (PropertyAddress::IsSameAddress(addresses[i], PropertyAddress(kCMIODevicePropertyExcludeNonDALAccess)))
				{
					if (mExcludeNonDALAccess != DPA::Sample::GetExcludeNonDALAccess(GetAssistantPort(), GetDeviceGUID()))
					{
						mExcludeNonDALAccess = not mExcludeNonDALAccess;
						PropertyAddress address(kCMIODevicePropertyExcludeNonDALAccess);
						PropertiesChanged(1, &address);
					}
				}
				else if (PropertyAddress::IsSameAddress(addresses[i], PropertyAddress(kCMIODevicePropertyClientSyncDiscontinuity)))
				{
					mClientSyncDiscontinuity->SetForceDiscontinuity(DPA::Sample::GetClientSyncDiscontinuity(GetAssistantPort(), GetDeviceGUID()));
				}
				else
				{
					// All the device level properties that are tracked have been addressed above, so just continue unless this is a input or output property
					if (addresses[i].mScope != kCMIODevicePropertyScopeInput and addresses[i].mScope != kCMIODevicePropertyScopeOutput)
						continue;

					// One of the stream properties to update
					Stream* stream = static_cast<Stream*>(GetStreamByPropertyAddress(addresses[i], false));
					ThrowIfNULL(stream, CAException(kCMIOHardwareUnknownPropertyError), "CMIO::DP::Sample::Device::UpdatePropertyStates: no stream for givin address");
					stream->UpdatePropertyState(addresses[i]);
				}
			}
			catch (...)
			{
				// Something went wrong...silently continue on to the next property
				char selector[5] = CA4CCToCString(addresses[i].mSelector);
				char scope[5] = CA4CCToCString(addresses[i].mScope);
				DebugMessage("CMIO::DP::Sample::Device::UpdatePropertyStates: failure updating property - %s - %s - %d", selector, scope, addresses[i].mElement);
				continue;
			}
		}

		// Remember when the properties were cached
		mPropertyCacheTime = currentTime;
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// UpdateControlStates()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void Device::UpdateControlStates(bool sendChangeNotifications) 
	{
		// Grab the muxtex for the device's state
		CAMutex::Locker locker(GetStateMutex());

		// Get the current time to use to remember when the controls were cached
		UInt64 currentTime = CAHostTimeBase::GetTheCurrentTime();
					
		// Get the state of all the control that have changed since the last time their values were cached
		DPA::Sample::AutoFreeUnboundedArray<DPA::Sample::ControlChanges> controlChanges;
		DPA::Sample::GetControls(GetAssistantPort(), GetDeviceGUID(), GetEventPort().GetMachPort(), mControlCacheTime, controlChanges);
		
		// Iterate of over all the controls that have had a state change
		for (UInt32 i = 0 ; i < controlChanges.GetLength() ; ++i)
		{
			// Find the control which has been updated
			DP::Control* control = GetControlByControlID(controlChanges[i].mControlID);
			if (NULL == control)
				continue;
			
			// AVC devices only have kCMIODirectionControlClassID & kCMIODataSourceControlClassID (masquerading a yet-to-be defined protocol ID) conntrols, just special case them
			// and be lazy on the switch statement evaluation.
			switch (control->GetBaseClassID())
			{
				case kCMIOBooleanControlClassID:
					static_cast<IOV::BooleanControl&>(*control).UpdateValue(static_cast<IOV::BooleanControl&>(*control).GetNoncachedValue(), sendChangeNotifications);
					break;
					
				case kCMIOSelectorControlClassID:
					// Also, the SampleAssistant doesn't provide enough to REALLY say what property of the selector (currentItem, availableItems, itemName) changed, but the only thing
					// that can actually change is the 'currentItem' so hard code appropriately.
					static_cast<IOV::SelectorControl&>(*control).UpdateCurrentItem(sendChangeNotifications);
					break;
			};
		}
		
		// Remember when the conrols were cached
		mControlCacheTime = currentTime;
	}

	#pragma mark -
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// IsSafeToExecuteCommand()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	bool Device::IsSafeToExecuteCommand(DP::Command* command)
	{
		return true;
	}
	
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// StartCommandExecution()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	bool Device::StartCommandExecution(void** savedCommandState)
	{
		*savedCommandState = 0;
		return true;
	}
	
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// FinishCommandExecution()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void Device::FinishCommandExecution(void* savedCommandState)
	{
	}
	
	#pragma mark -
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// ProcessRS422Command()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void Device::ProcessRS422Command(CMIODeviceRS422Command* ioRS422Command) 
	{
		DebugMessage("CMIO::DP::Sample::Device::ProcessRS422Command");
		DPA::Sample::ProcessRS422Command(GetAssistantPort(), GetDeviceGUID(), *ioRS422Command);
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// StreamDirectionChanged()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void Device::StreamDirectionChanged(CMIOObjectPropertyScope newScope) 
	{
		ReleaseStreams(true);
		CreateStreams(newScope);
	}
	
	#pragma mark -
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// Unplug()
	//	This is voked in the event a kIOMessageServiceIsRequestingClose message is received and the underlying AVSA::Device is open.
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void Device::Unplug()
	{
		// Don't do anything if the device is no longer alive
		if (not IsAlive())
			return;
		
		// Suspend all the streams
		SuspendAllStreams();

		// Send the IsAlive notifications
		SetIsAlive(false);
		PropertyAddress address(kCMIODevicePropertyDeviceIsAlive);
		PropertiesChanged(1, &address);

		// Stop all the streams since clients have had a chance to handle the "IsAlive" change
		StopAllStreams();
	}

	#pragma mark -
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// StartStream()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void Device::StartStream(CMIOStreamID streamID)
	{
		// See if the streamID is an input stream
		Stream* stream = static_cast<Stream*>(GetStreamByID(kCMIODevicePropertyScopeInput, streamID));
		
		// If it wasn't an input stream, see if it an output stream
		if (NULL == stream)
		{
			stream = static_cast<Stream*>(GetStreamByID(kCMIODevicePropertyScopeOutput, streamID));
			ThrowIfNULL(stream, CAException(kCMIOHardwareBadStreamError), "CMIO::DP::Sample::Device::StartStream: streamID does not correspond to any of the device's streams");
		}
			
		// Simply return if the stream is already active
		if (stream->Streaming())
			return;
					
		// Start the stream
		stream->Start();
		
		// Indicate that the device is "running" and send out the property changed notifications
		DeviceStarted();
		PropertyAddress address(kCMIODevicePropertyDeviceIsRunning);
		PropertiesChanged(1, &address);
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// StopStream()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void Device::StopStream(CMIOStreamID streamID)
	{
		// See if the streamID is an input stream
		Stream* stream = static_cast<Stream*>(GetStreamByID(kCMIODevicePropertyScopeInput, streamID));
		
		// If it wasn't an input stream, see if it an output stream
		if (NULL == stream)
		{
			stream = static_cast<Stream*>(GetStreamByID(kCMIODevicePropertyScopeOutput, streamID));
			ThrowIfNULL(stream, CAException(kCMIOHardwareBadStreamError), "CMIO::DP::Sample::Device::StartStream: streamID does not correspond to any of the device's streams");
		}
			
		// Simply return if the stream is not active
		if (not stream->Streaming())
			return;
			
		// Stop the stream
		stream->Stop();
		
		// If any input streams are still active, simply return
		UInt32 numberStreams = GetNumberStreams(kCMIODevicePropertyScopeInput);
		for (UInt32 streamIndex = 0; streamIndex != numberStreams; ++streamIndex)
		{
			if (GetStreamByIndex(kCMIODevicePropertyScopeInput, streamIndex)->Streaming())
				return;
		}

		// If any output streams are still active, simply return
		numberStreams = GetNumberStreams(kCMIODevicePropertyScopeOutput);
		for (UInt32 streamIndex = 0; streamIndex != numberStreams; ++streamIndex)
		{
			if (GetStreamByIndex(kCMIODevicePropertyScopeOutput, streamIndex)->Streaming())
				return;
		}

		// Indicate that the device is not "running" and send out the property changed notifications
		DeviceStopped();
		PropertyAddress address(kCMIODevicePropertyDeviceIsRunning);
		PropertiesChanged(1, &address);
	}
	
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// StopAllStreams()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void Device::StopAllStreams()
	{
		// Stop all the input streams
		UInt32 numberStreams = GetNumberStreams(kCMIODevicePropertyScopeInput);
		for (UInt32 streamIndex = 0; streamIndex != numberStreams; ++streamIndex)
		{
			Stream* stream = static_cast<Stream*>(GetStreamByIndex(kCMIODevicePropertyScopeInput, streamIndex));
			stream->Stop();
		}

		// Stop all the output streams
		numberStreams = GetNumberStreams(kCMIODevicePropertyScopeOutput);
		for (UInt32 streamIndex = 0; streamIndex != numberStreams; ++streamIndex)
		{
			Stream* stream = static_cast<Stream*>(GetStreamByIndex(kCMIODevicePropertyScopeOutput, streamIndex));
			stream->Stop();
		}

		// Indicate that the device is not "running" and send out the property changed notifications
		DeviceStopped();
		PropertyAddress address(kCMIODevicePropertyDeviceIsRunning);
		PropertiesChanged(1, &address);
	}
	
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// SuspendAllStreams()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void Device::SuspendAllStreams()
	{
		// Suspend all the input streams
		UInt32 numberStreams = GetNumberStreams(kCMIODevicePropertyScopeInput);
		for (UInt32 streamIndex = 0; streamIndex != numberStreams; ++streamIndex)
		{
			Stream* stream = static_cast<Stream*>(GetStreamByIndex(kCMIODevicePropertyScopeInput, streamIndex));
			stream->Unplug();
		}

		// Suspend all the output streams
		numberStreams = GetNumberStreams(kCMIODevicePropertyScopeOutput);
		for (UInt32 streamIndex = 0; streamIndex != numberStreams; ++streamIndex)
		{
			Stream* stream = static_cast<Stream*>(GetStreamByIndex(kCMIODevicePropertyScopeOutput, streamIndex));
			stream->Unplug();
		}
	}
	
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// CreateStreams()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void Device::CreateStreams(CMIOObjectPropertyScope scope) 
	{
		// Get the stream configuration from the Assistant in 
		DPA::Sample::AutoFreeUnboundedArray<UInt32> configuration;
		DPA::Sample::GetStreamConfiguration(GetAssistantPort(), GetDeviceGUID(), scope, configuration);
		
		// The first element of the configuration array is the number of streams present
		UInt32 streamCount = configuration[0];
		
		// The first stream in this scope starts at channel 1
		UInt32 startingDeviceChannelNumber = 1;

		// Create a vector of CMIOStreamIDs to hold the streams being created
		std::vector<CMIOStreamID> streamIDs;

		for (UInt32 i = 0 ; i < streamCount ; ++i)
		{
			// Create the CMIOStream object 
			CMIOObjectID streamID;
			OSStatus err = CMIOObjectCreate(GetPlugIn().GetInterface(), GetObjectID(), kCMIOStreamClassID, &streamID);
			ThrowIfError(err, CAException(err), "CMIO::DP::Sample::Device::CreateStreams: couldn't instantiate the CMIOStream object");
			
			// Create the stream
			Stream* stream = new Stream(streamID, GetPlugIn(), *this, scope, startingDeviceChannelNumber);

			try
			{
				stream->Initialize();
				
				// Add to the list of streams in this device
				AddStream(stream);
				
				// Set the object state mutex
				Object::SetObjectStateMutexForID(streamID, stream->GetObjectStateMutex());

				// Store the new stream ID
				streamIDs.push_back(streamID);
			}
			catch (...)
			{
				// Remove it from the stream list (which is always safe to attempt) and delete it
				RemoveStream(stream);
				delete stream;
				throw;
			}
			
			// Bump the startingDeviceChannelNumber by the number of channels this stream has (as indicated by the 'i + 1' element of the configuration array
			startingDeviceChannelNumber += configuration[i + 1];
		}

		// Tell the DAL about the new streams
		if (streamIDs.size() > 0)
		{
			OSStatus err = CMIOObjectsPublishedAndDied(GetPlugIn().GetInterface(), GetObjectID(), streamIDs.size(), &(streamIDs.front()), 0, NULL);
			ThrowIfError(err, CAException(err), "CMIO::DP::Sample::Device::CreateStreams: couldn't tell the DAL about the streams");
		}
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// ReleaseStreams()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void Device::ReleaseStreams(bool reportDeath) 
	{
		// Release the input streams
		while (GetNumberStreams(kCMIODevicePropertyScopeInput) > 0)
		{
			// Get the stream
			Stream* stream = static_cast<Stream*>(GetStreamByIndex(kCMIODevicePropertyScopeInput, 0));
			CMIOObjectID objectID = stream->GetObjectID();
		
			if (reportDeath)
				(void) CMIOObjectsPublishedAndDied(GetPlugIn().GetInterface(), GetObjectID(), 0, 0, 1, &objectID);

			// Remove it from the lists
			RemoveStream(stream);
			
			// Toss it
			stream->Teardown();
			delete stream;
		}

		// Release the output streams
		while (GetNumberStreams(kCMIODevicePropertyScopeOutput) > 0)
		{
			// Get the stream
			Stream* stream = static_cast<Stream*>(GetStreamByIndex(kCMIODevicePropertyScopeOutput, 0));
			CMIOObjectID objectID = stream->GetObjectID();
		
			if (reportDeath)
				(void) CMIOObjectsPublishedAndDied(GetPlugIn().GetInterface(), GetObjectID(), 0, 0, 1, &objectID);
			
			// Remove it from the lists
			RemoveStream(stream);
			
			// Toss it
			stream->Teardown();
			delete stream;
		}
	}
}}}
