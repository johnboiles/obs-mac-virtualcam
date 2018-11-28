/*
	    File: CMIO_DP_Sample_PlugIn.cpp
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
#include "CMIO_DP_Sample_Plugin.h"

// Internal Includes
#include "CMIO_DP_Sample_Device.h"

// DAL PlugIn Base Includes
#include "CMIO_DP_DeviceSettings.h"

// Public Utility Includes
#include "CMIODebugMacros.h"
#include "CMIO_DALA_System.h"
#include "CMIO_PropertyAddress.h"

// CA Public Utility Includes
#include "CAAutoDisposer.h"
#include "CAException.h"
#include "CACFString.h"
#include "CAHostTimeBase.h"

// System Includes
#include <IOKit/IOMessage.h>
#include <servers/bootstrap.h>

extern "C"
{
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// AppleCMIODPSampleNewPlugIn()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void* AppleCMIODPSampleNewPlugIn(CFAllocatorRef allocator, CFUUIDRef requestedTypeUUID);
	void* AppleCMIODPSampleNewPlugIn(CFAllocatorRef allocator, CFUUIDRef requestedTypeUUID) 
	{
		if (not CFEqual(requestedTypeUUID, kCMIOHardwarePlugInTypeID))
			return 0;
		
		try
		{
			// Before going any further, make sure the SampleAssistant process is registerred with Mach's bootstrap service.  Normally, this would be done by having an appropriately
			// configured plist in /Library/LaunchDaemons, but if that is done then the process will be owned by root, thus complicating the debugging process.  Therefore, in the event that the
			// plist is missing (as would be the case for most debugging efforts) attempt to register the SampleAssistant now.  It will fail gracefully if allready registered.
			mach_port_t assistantServicePort;		
			name_t assistantServiceName = "com.apple.cmio.DPA.Sample";
			kern_return_t err = bootstrap_look_up(bootstrap_port, assistantServiceName, &assistantServicePort);
			if (BOOTSTRAP_SUCCESS != err)
			{
				// Create an URL to SampleAssistant that resides at "/Library/CoreMediaIO/Plug-Ins/DAL/Sample.plugin/Contents/Resources/SampleAssistant" 
				CACFURL assistantURL(CFURLCreateWithFileSystemPath(NULL, CFSTR("/Library/CoreMediaIO/Plug-Ins/DAL/Sample.plugin/Contents/Resources/SampleAssistant"), kCFURLPOSIXPathStyle, false));
				ThrowIf(not assistantURL.IsValid(), CAException(-1), "AppleCMIODPSampleNewPlugIn: unable to create URL for the SampleAssistant");

				// Get the maximum size of the of the file system representation of the SampleAssistant's absolute path
				CFIndex length = CFStringGetMaximumSizeOfFileSystemRepresentation(CACFString(CFURLCopyFileSystemPath(CACFURL(CFURLCopyAbsoluteURL(assistantURL.GetCFObject())).GetCFObject(), kCFURLPOSIXPathStyle)).GetCFString());

				// Get the file system representation
				CAAutoFree<char> path(length);
				(void) CFURLGetFileSystemRepresentation(assistantURL.GetCFObject(), true, reinterpret_cast<UInt8*>(path.get()), length);

				mach_port_t assistantServerPort;
				err = bootstrap_create_server(bootstrap_port, path, getuid(), true, &assistantServerPort);
				ThrowIf(BOOTSTRAP_SUCCESS != err, CAException(err), "AppleCMIODPSampleNewPlugIn: couldn't create server");
				
				err = bootstrap_check_in(assistantServerPort, assistantServiceName, &assistantServicePort);

				// The server port is no longer needed so get rid of it
				(void) mach_port_deallocate(mach_task_self(), assistantServerPort);

				// Make sure the call to bootstrap_create_service() succeeded
				ThrowIf(BOOTSTRAP_SUCCESS != err, CAException(err), "AppleCMIODPSampleNewPlugIn: couldn't create SampleAssistant service port");
			}

			// The service port is not longer needed so get rid of it
			(void) mach_port_deallocate(mach_task_self(), assistantServicePort);


			CMIO::DP::Sample::PlugIn* plugIn = new CMIO::DP::Sample::PlugIn(requestedTypeUUID);
			plugIn->Retain();
			return plugIn->GetInterface();
		}
		catch (...)
		{
			return NULL;
		}
	}
}

namespace CMIO { namespace DP { namespace Sample
{
	#pragma mark -
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// PlugIn()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	PlugIn::PlugIn(CFUUIDRef factoryUUID) :
		DP::PlugIn(factoryUUID),
		mAssistantPort(),
		mDeviceEventPort(NULL),
		mDeviceEventDispatchSource(NULL),
		mAssistantCrashAnchorTime(CAHostTimeBase::GetCurrentTimeInNanos()),
		mAssistantCrashCount(0)
	{
	}
	
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// ~PlugIn()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	PlugIn::~PlugIn()
	{
		// Cancel the dispatch source for the device event notifications
		if (NULL != mDeviceEventDispatchSource)
		{
			dispatch_source_cancel(mDeviceEventDispatchSource);
			dispatch_release(mDeviceEventDispatchSource);
			mDeviceEventDispatchSource = NULL;
		}
	}
	
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// Initialize()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void PlugIn::InitializeWithObjectID(CMIOObjectID objectID)
	{
		// Grab the muxtex for the plugIn's state.
		CAMutex::Locker locker(GetStateMutex());

		// Initialize the super class
		DP::PlugIn::InitializeWithObjectID(objectID);

		// Set the object state mutex
		Object::SetObjectStateMutexForID(objectID, GetObjectStateMutex());

		// Create the Mach port for device event notifications
		mDeviceEventPort = new CACFMachPort(reinterpret_cast<CFMachPortCallBack>(DeviceEvent), this);

 		// Create a dispatch source that listens to the device event port
		mDeviceEventDispatchSource = dispatch_source_create(DISPATCH_SOURCE_TYPE_MACH_RECV, mDeviceEventPort->GetMachPort(), 0, dispatch_get_main_queue());
		if (NULL == mDeviceEventDispatchSource)
		{
			// Delete the device event port (since normally that would be handled by the the dispatch source's cancel handler)
			delete mDeviceEventPort;
			mDeviceEventPort = NULL;
			DebugMessage("CMIO::DP::Sample::PlugIn::InitializeWithObjectID: failed to create the mach port event source");
			throw CAException(kCMIOHardwareUnspecifiedError);
		}
		
		// Install an event handler
		dispatch_source_set_event_handler(mDeviceEventDispatchSource, ^{ DeviceEvent(*this); });
		
		// Note that the mach port cannot be freed prior to the the cancel handler running due to a race condition. See the note in the comments dispatch_source_set_cancel_handler in <dispatch/source.h>.
		dispatch_source_set_cancel_handler(mDeviceEventDispatchSource, ^{ delete mDeviceEventPort; mDeviceEventPort = NULL; });

		// Resume the event source so that it can start handling messages and also so that the source can be released
		dispatch_resume(mDeviceEventDispatchSource);

		// Get the Mach port on which messages will be sent to the SampleAssistant
		mAssistantPort.Reset(DPA::Sample::GetPort());

		// Get the state of all the devices currently connected
		UpdateDeviceStates();
	}
	
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// Teardown()
	//	This is invoked when the DAL is torn down by CMIOHardwareUnload().  If the process quits or crashes, the DAL just vanishes.
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void PlugIn::Teardown()
	{
		// Grab the muxtex for the plugIn's state
		CAMutex::Locker locker(GetStateMutex());
		
		// Cancel the dispatch source for the device event notifications
		if (NULL != mDeviceEventDispatchSource)
		{
			dispatch_source_cancel(mDeviceEventDispatchSource);
			dispatch_release(mDeviceEventDispatchSource);
			mDeviceEventDispatchSource = NULL;
		}

		// Do the full teardown if this is outside of the process being torn down or this is the master process
		if (not DALA::System::IsInitingOrExiting() or DALA::System::IsMaster())
		{
			// Teardown all the devices currently being managed
			while (0 != GetNumberDevices())
				DeviceRemoved(*static_cast<Device*>(GetDeviceByIndex(0)));
			
			// Teardown the super class
			DP::PlugIn::Teardown();
		}
		else
		{
			// Iterate over the devices and suspend and finalize them
			for (UInt32 i = 0 ; i < GetNumberDevices() ; ++i)
			{
				Device* device = static_cast<Device*>(GetDeviceByIndex(i));
				
				// Suspend the device
				device->Unplug();
				
				// Finalize (rather than teardown) the device
				device->Finalize();
			}
			
			// Teardown the super class
			DP::PlugIn::Teardown();
			
			// And leave the rest to die with the process...
		}
	}
	
	#pragma mark -
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// HasProperty()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	bool PlugIn::HasProperty(const CMIOObjectPropertyAddress& address) const
	{
		// Initialize the return value
		bool answer = false;
		
		switch (address.mSelector)
		{
			case kCMIOObjectPropertyName:
				answer = true;
				break;
				
			default:
				answer = DP::PlugIn::HasProperty(address);
				break;
		};
		
		return answer;
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// IsPropertySettable()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	bool PlugIn::IsPropertySettable(const CMIOObjectPropertyAddress& address) const
	{
		bool answer = false;
		
		switch (address.mSelector)
		{
			case kCMIOObjectPropertyName:
				answer = false;
				break;
				
			default:
				answer = DP::PlugIn::IsPropertySettable(address);
				break;
		};
		
		return answer;
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// GetPropertyDataSize()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	UInt32 PlugIn::GetPropertyDataSize(const CMIOObjectPropertyAddress& address, UInt32 qualifierDataSize, const void* qualifierData) const
	{
		UInt32 answer = 0;
		
		switch (address.mSelector)
		{
			case kCMIOObjectPropertyName:
				answer = sizeof(CFStringRef);
				break;
				
			default:
				answer = DP::PlugIn::GetPropertyDataSize(address, qualifierDataSize, qualifierData);
				break;
		};
		
		return answer;
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// GetPropertyData()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void PlugIn::GetPropertyData(const CMIOObjectPropertyAddress& address, UInt32 qualifierDataSize, const void* qualifierData, UInt32 dataSize, UInt32& dataUsed, void* data) const
	{
		switch (address.mSelector)
		{
			case kCMIOObjectPropertyName:
				ThrowIf(dataSize != GetPropertyDataSize(address, qualifierDataSize, qualifierData), CAException(kCMIOHardwareBadPropertySizeError), "CMIO::DP::Sample::PlugIn::GetPropertyData: wrong data size for kCMIOObjectPropertyName");
				*static_cast<CFStringRef*>(data) = CFSTR("com.apple.cmio.DAL.Sample");
				CFRetain(*static_cast<CFStringRef*>(data));
				dataUsed = sizeof(CFStringRef);
				break;
				
			default:
				DP::PlugIn::GetPropertyData(address, qualifierDataSize, qualifierData, dataSize, dataUsed, data);
				break;
		};
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// SetPropertyData()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void PlugIn::SetPropertyData(const CMIOObjectPropertyAddress& address, UInt32 qualifierDataSize, const void* qualifierData, UInt32 dataSize, const void* data)
	{
		switch (address.mSelector)
		{
			default:
				DP::PlugIn::SetPropertyData(address, qualifierDataSize, qualifierData, dataSize, data);
				break;
		};
	}

	#pragma mark -
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// GetDeviceByGUID()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	Device&	PlugIn::GetDeviceByGUID(UInt64 guid) const
	{
		Device* device = 0;
		bool found = false;

		// Iterate over the plugIn's devices and find the one whose guid matches
		for (UInt32 i = 0 ; i < GetNumberDevices() ; ++i)
		{
			device = static_cast<Device*>(GetDeviceByIndex(i));
			ThrowIfNULL(device, CAException(kCMIOHardwareBadDeviceError), "CMIO::DP::Sample::PlugIn::GetDeviceByGUID: no device for index");
			if (guid == device->GetDeviceGUID())
			{
				found = true;
				break;
			}
		}
		
		// Make sure that a device was actually found
		if (not found)
			throw CAException(kCMIOHardwareBadDeviceError);
		
		return *device;
	}

	#pragma mark -
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// DeviceEvent()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void PlugIn::DeviceEvent(PlugIn& plugIn) 
	{
		// Don't let any exceptions	leave this callback
		try
		{	
			// The port used to field notifications has a message, so have to pull it off the queue
			struct Message
			{
				DPA::Sample::DeviceStatesChangedMessage	mMessage;
				mach_msg_trailer_t						mTrailer;
			};
			
			Message message;
			kern_return_t err = plugIn.mDeviceEventPort->ReceiveMessage(sizeof(Message), &message.mMessage.mHeader, 1000);
			if (noErr != err)
			{
				// Something went wrong trying to receive the message
				DebugMessageIfError(err, "CMIO::DP::Sample::PlugIn::DeviceEvent: failed to receive the message");
				
				if (MACH_RCV_TIMED_OUT == err)
				{
					// If the message timed out, simply try and update the devices state again
					message.mMessage.mHeader.msgh_id = DPA::Sample::kDeviceStatesChanged;
				}
				else
				{
					// Something else went wrong, so act as if the 'case 71' had occurred and re-establish the connection with the Assistant
					message.mMessage.mHeader.msgh_id = 71;
				}
			}

			// Grab the mutex for the plugIn's state
			CAMutex::Locker locker(plugIn.GetStateMutex());
					
			switch (message.mMessage.mHeader.msgh_id)
			{
				case DPA::Sample::kDeviceStatesChanged:
				{
					// Update the state of the of the devices
					plugIn.UpdateDeviceStates();
				}
				break;
				
				case 71:
				{
					// Though not extensively documented, this is the message ID sent from a MIG server that held a 'send-once right' to a message port but died without ever having exercised
					// that right.  The plugIn had given the Assistant a 'send-once' right when it had called DPA::Sample::GetDeviceStates(), so getting this message indicates the Assistant
					// has crashed.

					// Remove all the devices currently being managed
					while (0 != plugIn.GetNumberDevices())
						plugIn.DeviceRemoved(*static_cast<Device*>(plugIn.GetDeviceByIndex(0)));

					// Attempt to re-establish the connection with the Assistant if it hasn't crashed more than 5 times in the last minute.  If that threshhold is exceeded, no reconnection
					// attempt will be made so all attempts to communicate with it will fail until the application using this plugIn is relaunched.
					UInt64 now = CAHostTimeBase::GetCurrentTimeInNanos();
					
					if (now - plugIn.mAssistantCrashAnchorTime > 60000000000ULL or plugIn.mAssistantCrashCount < 5)
					{
						DebugMessage("CMIO::DP::Sample::PlugIn::DeviceEvent: The Assistant has crashed...respawning it");
						
						// Reset mAssistantCrashAnchorTime to now and reset the crash count to 0 if more than 1 minute has passed since the last time a crash was anchored
						if (now - plugIn.mAssistantCrashAnchorTime > 60000000000ULL)
						{
							plugIn.mAssistantCrashAnchorTime = now;
							plugIn.mAssistantCrashCount = 0;
						}
						
						// Bump the crash count
						++plugIn.mAssistantCrashCount;
						
						// Get the Mach port on which messages will be sent to the SampleAssistant
						plugIn.mAssistantPort.Reset(DPA::Sample::GetPort());

						// Get the state of all the devices currently connected
						plugIn.UpdateDeviceStates();
					}
					else
					{
						DebugMessage("CMIO::DP::Sample::PlugIn::DeviceEvent: The Assistant has crashed...not respawning since it has crashed to much");
					}
				}
				break;
			}
		}
		catch (...)
		{
		}
	}

	#pragma mark -
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// UpdateDeviceStates()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void PlugIn::UpdateDeviceStates() 
	{
		// Get the current state of all the devices plugged in from the SampleAssistant
		DPA::Sample::AutoFreeUnboundedArray<DPA::Sample::DeviceState> deviceStates;
		DPA::Sample::GetDeviceStates(mAssistantPort, mDeviceEventPort->GetMachPort(), deviceStates);
		
		// Determine how many devices have been removed (if any).
		// This will be done by iterating over all the devices the PlugIn knows about and making sure they are present in the deviceStates array
		{
			std::vector<Device*> removedDevices;

			for (UInt32 i = 0 ; i < GetNumberDevices() ; ++i)
			{
				Device* device = static_cast<Device*>(GetDeviceByIndex(i));
				bool found = false;
				
				// See if it can be located in the deviceStates array
				for (UInt32 ii = 0; ii < deviceStates.GetLength() ; ++ii)
				{
					if (deviceStates[ii].mGUID == device->GetDeviceGUID())
					{
						found = true;
						break;
					}
				}
				
				// If the device was not found, stick into the vector of unplugged devices
				if (not found)
					removedDevices.push_back(device);
			}
			
			// Remove all the unplugged devices
			for (std::vector<Device*>::iterator i = removedDevices.begin() ; i != removedDevices.end() ; ++i)
				DeviceRemoved(**i);
		}

		// Determine how many new devices are present
		{
			for (UInt32 i = 0; i < deviceStates.GetLength() ; ++i)
			{
				try
				{
					// This will throw an exception if the device is not found
					(void) GetDeviceByGUID(deviceStates[i].mGUID);
				}
				catch (...)
				{
					// No device was found with the indicated GUID, so it is a new device
					DeviceArrived(deviceStates[i].mGUID, deviceStates[i].mRegistryPath);
				}
			}
		}
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// DeviceArrived()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void PlugIn::DeviceArrived(UInt64 guid, const io_string_t registryPath)
	{
		try
		{	
			// Instantiate a new CMIODevice object in the DAL
			CMIODeviceID deviceID = 0;
			OSStatus err = CMIOObjectCreate(GetInterface(), kCMIOObjectSystemObject, kCMIODeviceClassID, &deviceID);
			ThrowIfError(err, CAException(err), "CMIO::DP::Sample::PlugIn::DeviceArrived: couldn't instantiate the CMIODevice object");

			// Make a device object
			Device* device = new Device(*this, deviceID, mAssistantPort, guid, registryPath);

			try
			{
				// Initialize the device
				device->Initialize();
				
				// Restore its settings if necessary
				if (DALA::System::IsMaster())
				{
					DP::DeviceSettings::RestoreFromPrefs(*device, DP::DeviceSettings::sStandardControlsToSave, DP::DeviceSettings::kStandardNumberControlsToSave);
				}
				
				// Set the object state mutex
				Object::SetObjectStateMutexForID(deviceID, device->GetObjectStateMutex());
				
				// Add it to the device list
				AddDevice(*device);

				// Unlock the mutex since CMIOObjectsPublishedAndDied() can callout to property listeners
				CAMutex::Unlocker unlocker(GetStateMutex());

				// Tell the DAL about the device
				err = CMIOObjectsPublishedAndDied(GetInterface(), kCMIOObjectSystemObject, 1, &deviceID, 0, 0);
				ThrowIfError(err, err, "CMIO::DP::Sample::PlugIn::DeviceArrived: couldn't tell the DAL about a new device");
			}
			catch (...)
			{
				// Remove it from the device list (which is always safe to attempt) and delete it
				RemoveDevice(*device);
				delete device;
			}
		}
		catch (...)
		{
		}
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// DeviceRemoved()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void PlugIn::DeviceRemoved(Device& device)
	{
		// Suspend the device
		device.Unplug();

		// Save it's settings if necessary
		if (DALA::System::IsMaster())
			DP::DeviceSettings::SaveToPrefs(device, DP::DeviceSettings::sStandardControlsToSave, DP::DeviceSettings::kStandardNumberControlsToSave);

		{
			// Unlock the mutex since CMIOObjectsPublishedAndDied() can callout to property listeners
			CAMutex::Unlocker unlocker(GetStateMutex());

			// Tell the DAL that the device has gone away
			CMIOObjectID objectID = device.GetObjectID();
			OSStatus err = CMIOObjectsPublishedAndDied(GetInterface(), kCMIOObjectSystemObject, 0, 0, 1, &objectID);
			AssertNoError(err, "CMIO::DP::Sample::PlugIn::Teardown: got an error telling the DAL a device died");
		}
						
		// Remove it from the device list
		RemoveDevice(device);

		// Get rid of the device object
		device.Teardown();
		delete &device;
	}
}}}