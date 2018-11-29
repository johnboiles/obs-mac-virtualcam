/*
	    File: CMIO_DP_Sample_IOBackedDevice.cpp
	Abstract: n/a
	 Version: 1.2
	
*/

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// includes
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

// Self Include
#include "CMIO_DP_Sample_IOBackedDevice.h"

// Internal Includes
#include "CMIO_DP_Sample_PlugIn.h"
#include "CMIO_DP_IOV_Control.h"

// CA Public Utility Includes
#include "CACFArray.h"

// System Includes
#include <IOKit/video/IOVideoTypes.h>

#pragma mark -
namespace CMIO { namespace DP { namespace Sample
{
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// IOBackedDevice()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    IOBackedDevice::IOBackedDevice(PlugIn& plugIn, CMIODeviceID deviceID, mach_port_t assistantPort, UInt64 guid, const io_string_t registryPath):
        Device(plugIn, deviceID, assistantPort, guid),
        mRegistryEntry(IORegistryEntryFromPath(kIOMasterPortDefault, registryPath))
	{
		// Make sure the registry entry is valid
		ThrowIf(not mRegistryEntry.IsValid(), CAException(kIOReturnBadArgument), "CMIO::DP::Sample::Device: invalid registry entry");
	}
	
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// CopyControlDictionaryByControlID()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	CFDictionaryRef IOBackedDevice::CopyControlDictionaryByControlID(UInt32 controlID) const
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
            answer = Device::CopyControlDictionaryByControlID(controlID);
		}
		
		return answer;
	}

	#pragma mark -
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// CreateRegistryControls()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void IOBackedDevice::CreateRegistryControls()
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
	
			OSStatus err = CMIOObjectsPublishedAndDied(GetPlugIn().GetInterface(), GetObjectID(), (UInt32)controlIDs.size(), &(controlIDs.front()), 0, NULL);
			ThrowIfError(err, CAException(err), "CMIO::DP::Sample::Device::CreateControls: couldn't tell the DAL about the controls");
		}
	}

}}}
