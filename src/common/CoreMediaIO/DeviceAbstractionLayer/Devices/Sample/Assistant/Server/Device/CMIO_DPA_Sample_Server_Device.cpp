/*
	    File: CMIO_DPA_Sample_Server_Device.cpp
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
//	Includes
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

// Self Include
#include "CMIO_DPA_Sample_Server_Device.h"

// Internal Includes
#include "CMIO_DPA_Sample_Server_Assistant.h"
#include "CMIO_KEXT_Sample_ControlIDs.h"
#include "CMIO_DP_ControlDictionary.h"
#include "CMIO_DP_IOV_ControlDictionary.h"
#include "CMIO_DP_Sample_ControlIDs.h"

// Public Utility Includes
#include "CMIODebugMacros.h"
#include "CMIO_BitField.h"
#include "CMIO_IOKA_Iterator.h"
#include "CMIO_PTA_NotificationPortThread.h"

// CA Public Utility Includes
#include "CAAutoDisposer.h"
#include "CACFData.h"
#include "CACFNumber.h"
#include "CACFString.h"
#include "CAException.h"
#include "CAHostTimeBase.h"
#include "CAProcess.h"

// System Includes
#include <CoreAudio/HostTime.h>
#include <IOKit/audio/IOAudioTypes.h>
#include <IOKit/IOMessage.h>
#include <CoreMediaIO/CMIOHardware.h>
#include <mach/mach.h>

// Standard Library Includes
#include <algorithm>

namespace
{
	#pragma mark Property Addresses
	// These property addresses are specified in DEVICE RELATIVE terms (i.e., {selector, kCMIODevicePropertyScopeXXXX, element # }
	//	
	//		kHogModeAddress
	//		This wouldn't be needed if the device can never be accessed by a non-DAL mechanism (e.g., a QuickTime video digitizer), since it value would always be -1, so the plugIn could
	//		always report that.
	//		It is only here to provide the ground work in the event the item non-DAL items can grab exclusive access to the device.  If so, then further fleshing out would be needed to
	//		become aware of when such an event happens so that the property could be set to something other than -1.  (The PID of the Assistant would be an adequate stand in should the
	//		actual PID not be determinable.)
	//
	//		kExcludeNonDALAccessAddress
	//		Similarly, this is not necessary if this device can't be accessed by a non-DAL mechanism since the plugIn could handle everything then.  It is only here to provide the groundwork
	//		in the event non-DAL items can access the device.  If so, then further fleshing out would be needed to lock out other items.

	// Properties at the device level
	const CMIO::PropertyAddress kInputStreamConfigurationAddress		= CMIO::PropertyAddress(kCMIODevicePropertyStreamConfiguration, kCMIODevicePropertyScopeInput);
	const CMIO::PropertyAddress kOutputStreamConfigurationAddress	= CMIO::PropertyAddress(kCMIODevicePropertyStreamConfiguration, kCMIODevicePropertyScopeOutput);
	const CMIO::PropertyAddress kHogModeAddress						= CMIO::PropertyAddress(kCMIODevicePropertyHogMode);
	const CMIO::PropertyAddress kDeviceMasterAddress					= CMIO::PropertyAddress(kCMIODevicePropertyDeviceMaster);
	const CMIO::PropertyAddress kExcludeNonDALAccessAddress			= CMIO::PropertyAddress(kCMIODevicePropertyExcludeNonDALAccess);
	const CMIO::PropertyAddress kDeviceIsRunningSomewhereAddress		= CMIO::PropertyAddress(kCMIODevicePropertyDeviceIsRunningSomewhere);
	const CMIO::PropertyAddress kClientSyncDiscontinuityAddress		= CMIO::PropertyAddress(kCMIODevicePropertyClientSyncDiscontinuity);
}

namespace CMIO { namespace DPA { namespace Sample { namespace Server
{
	#pragma mark Static members
	
	//
	// mGUIDGenerator
	//	Strictly speaking, the kCMIODevicePropertyDeviceUID is supposed to be "persistent across boots."  The approach taken in this sample code doesn't satisfy that requirement under
	//	certain corner cases, such as a 2nd card being added and the discovery order during subsequent boots being different from the previous boots.  Ideally, it would be possible to
	//	extract a unique ID from the IORegistry as is done with FireWire devices.  (This method of GUID generation is also inapproriate for devices that can be hot plugged.)
	UInt64 Device::mGUIDGenerator = 0;
	
	#pragma mark -
	#pragma mark Device
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// Device()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	Device::Device(IOKA::Object& registryEntry, PTA::NotificationPortThread& notificationPortThread) :
		mRegistryEntry(registryEntry),
        mIOVAPlugIn(IOVA::AllocatePlugIn(mRegistryEntry)),
        mIOVADevice(IOVA::AllocateDevice(mIOVAPlugIn)),
		mStateMutex("CMIO::DPA::Sample::Device state mutex"),
		mDeviceGUID(mGUIDGenerator++),
		mInputStreams(),
		mOutputStreams(),
		mCapabilitiesDiscovered(false),
		mProperties(),
		mPropertyStateNotifiers(),
		mPropertyStateNotifiersMutex("CMIO::DPA::Sample::Server::Device property state notififiers mutex"),
		mHogModeOwner(-1),
		mDeviceMaster(-1),
		mDeviceMasterClient(MACH_PORT_NULL),
		mExcludeNonDALAccess(false),
		mExcludeNonDALAccessClient(MACH_PORT_NULL),
		mForceDiscontinuity(false),
		mControlsList(true),
		mControls(),
		mControlStateNotifiers(),
		mControlStateNotifiersMutex("CMIO::DPA::Sample::Server::Device control state notififiers mutex"),
		mNotificationThread(notificationPortThread),
		mPowerNotificationPort(GetNotificationThread().GetRunLoop(), kCFRunLoopDefaultMode, reinterpret_cast<IOServiceInterestCallback>(PowerNotification), this),
		mSleeping(false),
		mRestartStreamOnWake(false)
	{
		// Grab the mutex for the device's state
		CAMutex::Locker locker(mStateMutex);
		
		// Get the registry path for the device
		IOReturn err = IORegistryEntryGetPath(mRegistryEntry, kIOServicePlane, mRegistryPath);
		ThrowIfKernelError(err, CAException(err), "CMIO::DPA::Sample::Server::Device:: IORegistryEntryGetPath() failed")
		
		// Open the underlying IOVideoDevice
		mIOVADevice.Open();
		
		// Add the IOVDevice's notifcation port run loop source to the notificaton thread
		mIOVADevice.AddToRunLoop(GetNotificationThread().GetRunLoop());
		
		// Specify the callback for device notifications
		mIOVADevice.SetNotificationCallback(reinterpret_cast<IOVideoDeviceNotificationCallback>(IOVDeviceNotification), this);
		
		// Discover what streams the device has
		DiscoverStreams();

		// Discover the device's cababilities
		DiscoverCapabilities();
	}
	
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// ~Device()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	Device::~Device()
	{
		// Grab the mutex for the device's state
		CAMutex::Locker locker(mStateMutex);
		
		// Delete all the input stream
		while (not mInputStreams.empty())
		{
			delete (*mInputStreams.begin()).second;
			mInputStreams.erase(mInputStreams.begin());
		}
				
		// Delete all the ouput stream
		while (not mOutputStreams.empty())
		{
			delete (*mOutputStreams.begin()).second;
			mOutputStreams.erase(mOutputStreams.begin());
		}
		
		// Remove the IOVDevice's notifcation port run loop source from the notificaton thread
		mIOVADevice.RemoveFromRunLoop(GetNotificationThread().GetRunLoop());
		
		// Close the underlying IOVideoDevice (a safe NOP if it is not open)
		mIOVADevice.Close();
		
		// Erase any ports that were to be used for control state change notification
		for (ClientNotifiers::iterator i = mControlStateNotifiers.begin() ; i != mControlStateNotifiers.end() ; ++i)
			(void) mach_port_destroy(mach_task_self(), (*i).second);

		// Erase any ports that were to be used for property state change notification
		for (ClientNotifiers::iterator i = mPropertyStateNotifiers.begin() ; i != mPropertyStateNotifiers.end() ; ++i)
			(void) mach_port_destroy(mach_task_self(), (*i).second);
	}

	#pragma mark -
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// ClientDied()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void Device::ClientDied(Client client)
	{
		// Grab the mutex for the device's state
		CAMutex::Locker locker(mStateMutex);

		{
			// Grab the mutex for the control state notifiers
			CAMutex::Locker locker(mControlStateNotifiersMutex);
			
			// Remove the client from any control change notifications it might have signed up for
			while (true)
			{
				ClientNotifiers::iterator i = mControlStateNotifiers.find(client);
				if (i == mControlStateNotifiers.end())
					break;
				
				// Destroy the port used to send notifications to the (now dead) client
				(void) mach_port_destroy(mach_task_self(), (*i).second);
				
				// Erase this entry from the multimap
				mControlStateNotifiers.erase(i);
			}
		}
		
		{
			// Grab the mutex for the property state notifiers
			CAMutex::Locker locker(mPropertyStateNotifiersMutex);
			
			// Remove the client from any property change notifications it might have signed up for
			while (true)
			{
				ClientNotifiers::iterator i = mPropertyStateNotifiers.find(client);
				if (i == mPropertyStateNotifiers.end())
					break;
				
				// Destroy the port used to send notifications to the (now dead) client
				(void) mach_port_destroy(mach_task_self(), (*i).second);
				
				// Erase this entry from the multimap
				mPropertyStateNotifiers.erase(i);
			}			
		}
		
		// Assume no property state change messages will have to be sent
		bool sendPropertyStatesChangedMessage = false;
		

		// Release device mastership if this client had owned it
		if (client == mDeviceMasterClient)
		{
			// Set the shadow time for when the property changed
			mProperties[kDeviceMasterAddress].mShadowTime = CAHostTimeBase::GetTheCurrentTime();
			
			// Mark the device master as being free
			mDeviceMaster = -1;
			mDeviceMasterClient = MACH_PORT_NULL;
			
			// Remember to send out notifications that properties have changed
			sendPropertyStatesChangedMessage = true;
		}
		
		// Allow non-DAL access if this client had excluded it
		if (client == mExcludeNonDALAccessClient)
		{
			// DoWhatIsNeededToEnableNonDALAccess()
			
			// Set the shadow time for when the property changed
			mProperties[kExcludeNonDALAccessAddress].mShadowTime = CAHostTimeBase::GetTheCurrentTime();
			
			// Allow non-DAL access to the device
			mExcludeNonDALAccess = 0;
			mExcludeNonDALAccessClient = MACH_PORT_NULL;
			
			// Remember to send out notifications that properties have changed
			sendPropertyStatesChangedMessage = true;
		}

		if (sendPropertyStatesChangedMessage)
			SendPropertyStatesChangedMessage();
	}

	#pragma mark -
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// StartStream()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void Device::StartStream(Client client, mach_port_t messagePort, CMIOObjectPropertyScope scope, CMIOObjectPropertyElement element, UInt32 initialDiscontinuityFlags)
	{
//		DebugMessage("Device::StartStream");
		// Grab the mutex for the device's state
		CAMutex::Locker locker(mStateMutex);

		// See if kCMIODevicePropertyIsRunningSomewhere change notifications will need to be sent out
		bool sendChangedNotifications = not AnyStreamRunning();

		// Start it
		GetStreamByScopeAndElement(scope, element).Start(client, messagePort, initialDiscontinuityFlags);

		// Inform the clients that the property states have changed if needed
		if (sendChangedNotifications)
		{
			// The kCMIODevicePropertyIsRunningSomewhere property has changed, so send out notifications
			mProperties[kDeviceIsRunningSomewhereAddress].mShadowTime = CAHostTimeBase::GetTheCurrentTime();
			SendPropertyStatesChangedMessage();
		}
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// StopStream()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void Device::StopStream(Client client, CMIOObjectPropertyScope scope, CMIOObjectPropertyElement element)
	{
//		DebugMessage("Device::StopStream");
		// Grab the mutex for the overall device's state
		CAMutex::Locker locker(mStateMutex);
		
		// Find the stream which is being stopped
		Stream& stream = GetStreamByScopeAndElement(scope, element);
		
		// Stop it
		stream.Stop(client);

		// Inform the clients that the property states have changed if needed
		if (not AnyStreamRunning())
		{
			// The kCMIODevicePropertyIsRunningSomewhere property has changed, so send out notifications
			mProperties[kDeviceIsRunningSomewhereAddress].mShadowTime = CAHostTimeBase::GetTheCurrentTime();
			SendPropertyStatesChangedMessage();
		}		
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	//  DiscoverStreams()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void Device::DiscoverStreams()
	{
		// Grab the mutex for the Device's state
		CAMutex::Locker locker(mStateMutex);

		// Fetch the input streams list
		CACFArray inputStreamList(static_cast<CFArrayRef>(IORegistryEntryCreateCFProperty(mRegistryEntry, CFSTR(kIOVideoDeviceKey_InputStreamList), NULL, 0)), true);
		
		// Iterate through the input streams, locate their registry entries, and add them to the mInputStreams map
		UInt32 streamCount = inputStreamList.GetNumberItems();
		for (UInt32 index = 0 ; index < streamCount ; ++index)
		{
			// Get the stream dictionary
			CFDictionaryRef streamDictionary = NULL;
			if (inputStreamList.GetDictionary(index, streamDictionary))
			{
				// Get the streamID from the the dictionary
				UInt32 streamID = CACFNumber(static_cast<CFNumberRef>(CFDictionaryGetValue(streamDictionary, CFSTR(kIOVideoStreamKey_StreamID))), false).GetSInt32();

				// Iterate over all the device's streams and find the registry entry for this stream
				IOKA::Iterator iterator(mRegistryEntry, kIOServicePlane);
				
				while (true)
				{
					IOKA::Object registryEntry(iterator.Next());
					if (not registryEntry.IsValid())
						break;
					
					// Make sure the registry entry conforms to an IOVideoStream
					if (not registryEntry.ConformsTo("IOVideoStream"))
						continue;
				
					// Make sure the streamIDs are the same
					if (streamID != CACFNumber(static_cast<CFNumberRef>(IORegistryEntryCreateCFProperty(registryEntry, CFSTR(kIOVideoStreamKey_StreamID), NULL, 0))).GetSInt32())
						continue;
					
					// Add the stream to the map of input streams the device tracks
					mInputStreams[streamID] = new Stream(this, registryEntry, streamDictionary, kCMIODevicePropertyScopeInput);
					
					// break out of the while loop since the stream was located
					break;
				}
			
			}
		}

		// Fetch the output streams list
		CACFArray outputStreamList(static_cast<CFArrayRef>(IORegistryEntryCreateCFProperty(mRegistryEntry, CFSTR(kIOVideoDeviceKey_OutputStreamList), NULL, 0)), true);

		// Iterate through the output streams, locate their registry entries, and add them to the mOutputStreams map
		streamCount = outputStreamList.GetNumberItems();
		for (UInt32 index = 0 ; index < streamCount ; ++index)
		{
			// Get the stream dictionary
			CFDictionaryRef streamDictionary = NULL;
			if (outputStreamList.GetDictionary(index, streamDictionary))
			{
				// Get the streamID from the the dictionary
				UInt32 streamID = CACFNumber(static_cast<CFNumberRef>(CFDictionaryGetValue(streamDictionary, CFSTR(kIOVideoStreamKey_StreamID))), false).GetSInt32();

				// Iterate over all the device's streams and find the registry entry for this stream
				IOKA::Iterator iterator(mRegistryEntry, kIOServicePlane);
				
				while (true)
				{
					IOKA::Object registryEntry(iterator.Next());
					if (not registryEntry.IsValid())
						break;
					
					// Make sure the registry entry conforms to an IOVideoStream
					if (not registryEntry.ConformsTo("IOVideoStream"))
						continue;
				
					// Make sure the streamIDs are the same
					if (streamID != CACFNumber(static_cast<CFNumberRef>(IORegistryEntryCreateCFProperty(registryEntry, CFSTR(kIOVideoStreamKey_StreamID), NULL, 0))).GetSInt32())
						continue;
					
					// Add the stream to the map of output streams the device tracks
					mOutputStreams[streamID] = new Stream(this, registryEntry, streamDictionary, kCMIODevicePropertyScopeOutput);
					
					// break out of the while loop since the stream was located
					break;
				}
			}
		}
	}


	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	//  RemoveStreams()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void Device::RemoveStreams()
	{
		// Grab the mutex for the Device's state
		CAMutex::Locker locker(mStateMutex);

		// Delete all the input stream
		while (not mInputStreams.empty())
		{
			delete (*mInputStreams.begin()).second;
			mInputStreams.erase(mInputStreams.begin());
		}
				
		// Delete all the ouput stream
		while (not mOutputStreams.empty())
		{
			delete (*mOutputStreams.begin()).second;
			mOutputStreams.erase(mOutputStreams.begin());
		}
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// GetStreamByScopeAndElement()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	Stream& Device::GetStreamByScopeAndElement(CMIOObjectPropertyScope scope, CMIOObjectPropertyElement element)
	{
		// Figure out in input or output stream is being started
		StreamMap& streams = (scope == kCMIODevicePropertyScopeInput) ? mInputStreams : mOutputStreams;
		
		// Find the stream which corresponds to the given element
		for (StreamMap::iterator i = streams.begin() ; i != streams.end() ; ++i)
		{
			if (element < ((*i).second->GetStartingDeviceChannelNumber() + (*i).second->GetCurrentNumberChannels()))
				return *(*i).second;
		};
		
		// No stream was found so throw an execption
		ThrowIf(true, CAException(kCMIOHardwareBadStreamError), "Device::GetStreamByElement: unable to locate stream by element");
	}
	
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// GetStreamByStreamID()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	Stream* Device::GetStreamByStreamID(UInt32 streamID)
	{
		// See if the it is an input stream
		StreamMap::iterator i = mInputStreams.find(streamID);
		if (i != mInputStreams.end())
			return (*i).second;

		i = mOutputStreams.find(streamID);
		if (i != mOutputStreams.end())
			return (*i).second;
		
		// No stream was found so return NULL
		return NULL;
	}
	
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// AnyStreamRunning()
	//	Iterate off all the streams and report 'true' if any are running.
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	bool Device::AnyStreamRunning() const
	{
		// Check the input streams
		StreamMap::const_iterator i = std::find_if(mInputStreams.begin(), mInputStreams.end(), Device::StreamIsActive());
		if (i != mInputStreams.end())
			return true;
			
		// Check the output streams
		i = std::find_if(mOutputStreams.begin(), mOutputStreams.end(), Device::StreamIsActive());
		return (i != mOutputStreams.end()) ? true : false;

	}
	
	#pragma mark -
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// StartDeckThreads()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void Device::StartDeckThreads(Client client, CMIOObjectPropertyScope scope, CMIOObjectPropertyElement element)
	{
		// Grab the mutex for the device's state
		CAMutex::Locker locker(mStateMutex);
		
		GetStreamByScopeAndElement(scope, element).StartDeckThreads(client);
	}
	
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// StopDeckThreads()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void Device::StopDeckThreads(Client client, CMIOObjectPropertyScope scope, CMIOObjectPropertyElement element)
	{
		// Grab the mutex for the device's state
		CAMutex::Locker locker(mStateMutex);
		
		GetStreamByScopeAndElement(scope, element).StopDeckThreads(client);
	}
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// DeckPlay()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void Device::DeckPlay(Client client, CMIOObjectPropertyScope scope, CMIOObjectPropertyElement element)
	{
		// Make sure the device is allowed to be changed by this client
		ThrowIf(not ClientIsDeviceMasterOrIsFree(client), CAException(kCMIODevicePermissionsError), "Device::DeckPlay: client not allowed to control deck");

		// Grab the mutex for the device's state
		CAMutex::Locker locker(mStateMutex);
				
		GetStreamByScopeAndElement(scope, element).DeckPlay();
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// DeckStop()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void Device::DeckStop(Client client, CMIOObjectPropertyScope scope, CMIOObjectPropertyElement element)
	{
		// Make sure the device is allowed to be changed by this client
		ThrowIf(not ClientIsDeviceMasterOrIsFree(client), CAException(kCMIODevicePermissionsError), "Device::DeckStop: client not allowed to control deck");

		// Grab the mutex for the device's state
		CAMutex::Locker locker(mStateMutex);
		
		GetStreamByScopeAndElement(scope, element).DeckStop();
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// DeckJog()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void Device::DeckJog(Client client, CMIOObjectPropertyScope scope, CMIOObjectPropertyElement element, SInt32 speed)
	{
		// Make sure the device is allowed to be changed by this client
		ThrowIf(not ClientIsDeviceMasterOrIsFree(client), CAException(kCMIODevicePermissionsError), "Device::DeckJog: client not allowed to control deck");

		// Grab the mutex for the device's state
		CAMutex::Locker locker(mStateMutex);
		
		GetStreamByScopeAndElement(scope, element).DeckJog(speed);
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// DeckCueTo()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void Device::DeckCueTo(Client client, CMIOObjectPropertyScope scope, CMIOObjectPropertyElement element, Float64 requestedTimecode, Boolean playOnCue)
	{
		// Make sure the device is allowed to be changed by this client
		ThrowIf(not ClientIsDeviceMasterOrIsFree(client), CAException(kCMIODevicePermissionsError), "Device::DeckCueTo: client not allowed to control deck");		

		// Grab the mutex for the device's state
		CAMutex::Locker locker(mStateMutex);
		
		GetStreamByScopeAndElement(scope, element).DeckCueTo(requestedTimecode, playOnCue);
	}
	
	#pragma mark -
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	//  DiscoverCapabilities()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void Device::DiscoverCapabilities()
	{
		// Grab the mutex for the Device's state
		CAMutex::Locker locker(mStateMutex);
		
		// Just leave if the capabilities have already been discovered
		if (mCapabilitiesDiscovered)
			return;
			
		InitializeProperties();
		InitializeControls();
				
		mCapabilitiesDiscovered = true;
	}
	
	#pragma mark -
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// GetProperties()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void Device::GetProperties(Client client, mach_port_t messagePort, UInt64 time, const PropertyAddress& matchAddress, PropertyAddress** addresses, mach_msg_type_number_t* length)
	{
		// Grab the mutex for the Device's state
		CAMutex::Locker locker(mStateMutex);
		
		// Add the <client, messagePort> pair to the multimap of clients to notify for future state changes to properties
		if (MACH_PORT_NULL != messagePort)
		{
			// Grab the mutex for the property state notifiers
			CAMutex::Locker notiferLocker(mPropertyStateNotifiersMutex);
			mPropertyStateNotifiers.insert(std::make_pair(client, messagePort));
		}
		// Create a vector of PropertyAddresses to track addresses which satisfy the match criteria
		PropertyAddressList matches;

		// Assume no addresses will satisfy the match criteria
		*length = 0;
		
		// Iterate over the device properties and check for matches
		for (Properties::iterator i = mProperties.begin() ; i != mProperties.end() ; std::advance(i, 1))
		{
			// Skip the address if its shadow time is less than the indicated time or the addresses are not congruent 
			if (((*i).second.mShadowTime < time) or (not PropertyAddress::IsCongruentAddress(matchAddress, (*i).first)))
				continue;

			// Add the address to the matches
			matches.AppendItem((*i).first);
		}

		// Iterate over the input streams and check for matches
		for (StreamMap::iterator i = mInputStreams.begin() ; i != mInputStreams.end() ; ++i)
			(*i).second->GetProperties(time, matchAddress, matches); 
				
		// Iterate over the output streams and check for matches
		for (StreamMap::iterator i = mOutputStreams.begin() ; i != mOutputStreams.end() ; ++i)
			(*i).second->GetProperties(time, matchAddress, matches); 
				
		// Make sure there are matches
		if (0 == matches.GetNumberItems())
			return;
		
		// Allocate the memory for returning the property addresses
		kern_return_t err =  vm_allocate(mach_task_self(), reinterpret_cast<vm_address_t*>(addresses), sizeof(PropertyAddress) * matches.GetNumberItems(), true);
		ThrowIfKernelError(err, CAException(err), "Device::GetProperties couldn't allocate storage for PropertyAddress[]");

		// Fill in the property addresses which have changed since the inidicated time
		for (UInt32 i = 0 ; i != matches.GetNumberItems() ; ++i)
		{
			matches.GetItemByIndex(i, (*addresses)[*length]);
			*length += 1;
		}
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// GetPropertyState()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void Device::GetPropertyState(const PropertyAddress& address, UInt8* qualifier, mach_msg_type_number_t qualifierLength,  UInt8** data, mach_msg_type_number_t* length)
	{
		// Grab the mutex for the Device's state
		CAMutex::Locker locker(mStateMutex);

		// Assume no addresses will satisfy the match criteria
		*length = 0;
		vm_size_t size = 0;
		
		if (PropertyAddress::IsSameAddress(address, kInputStreamConfigurationAddress))
		{
			// This answer is a CMIODeviceStreamConfiguration
			// This device has a single input stream with one channel, so creating the resultant CMIODeviceStreamConfiguration is straight forward
			UInt32 inputStreamCount = mInputStreams.size();
			size = sizeof(UInt32) + (sizeof(UInt32) * inputStreamCount);
			ThrowIfKernelError(vm_allocate(mach_task_self(), reinterpret_cast<vm_address_t*>(data), size, true), CAException(-1), "Device::GetPropertyState: allocation failed for kInputStreamConfigurationAddress");

			// Fill in the CMIODeviceStreamConfiguration
			CMIODeviceStreamConfiguration** streamConfiguration = reinterpret_cast<CMIODeviceStreamConfiguration**>(data);
			(**streamConfiguration).mNumberStreams = inputStreamCount;
			
			for (UInt32 i = 0 ; i < inputStreamCount ; ++i)
			{
				// Like everywhere else throughout the DAL, only a single channel per stream is currently supported due to just not having thought it all out
				(**streamConfiguration).mNumberChannels[i] = 1;
			}
		}
		else if (PropertyAddress::IsSameAddress(address, kOutputStreamConfigurationAddress))
		{
			// This answer is a CMIODeviceStreamConfiguration
			UInt32 outputStreamCount = mOutputStreams.size();
			size = sizeof(UInt32) + (sizeof(UInt32) * outputStreamCount);
			ThrowIfKernelError(vm_allocate(mach_task_self(), reinterpret_cast<vm_address_t*>(data), size, true), CAException(-1), "Device::GetPropertyState: allocation failed for kOutputStreamConfigurationAddress");

			// Fill in the CMIODeviceStreamConfiguration
			CMIODeviceStreamConfiguration** streamConfiguration = reinterpret_cast<CMIODeviceStreamConfiguration**>(data);
			(**streamConfiguration).mNumberStreams = outputStreamCount;
			
			for (UInt32 i = 0 ; i < outputStreamCount ; ++i)
			{
				// Like everywhere else throughout the DAL, only a single channel per stream is currently supported due to just not having thought it all out
				(**streamConfiguration).mNumberChannels[i] = 1;
			}
		}
		else if (PropertyAddress::IsSameAddress(address, kHogModeAddress))
		{
			// This answer is a pid_t
			size = sizeof(pid_t);
			ThrowIfKernelError(vm_allocate(mach_task_self(), reinterpret_cast<vm_address_t*>(data), size, true), CAException(-1), "Device::GetPropertyState: allocation failed for kHogModeAddress");
			
			// Report the answer
			**reinterpret_cast<pid_t**>(data) = mHogModeOwner;
		}
		else if (PropertyAddress::IsSameAddress(address, kDeviceMasterAddress))
		{
			// This answer is a pid_t
			size = sizeof(pid_t);
			ThrowIfKernelError(vm_allocate(mach_task_self(), reinterpret_cast<vm_address_t*>(data), size, true), CAException(-1), "Device::GetPropertyState: allocation failed for kDeviceMasterAddress");
			
			// Report the answer
			**reinterpret_cast<pid_t**>(data) = mDeviceMaster;
		}
		else if (PropertyAddress::IsSameAddress(address, kExcludeNonDALAccessAddress))
		{
			// This answer is a Byte
			size = sizeof(Byte);
			ThrowIfKernelError(vm_allocate(mach_task_self(), reinterpret_cast<vm_address_t*>(data), size, true), CAException(-1), "Device::GetPropertyState: allocation failed for kExcludeNonDALAccessAddress");

			// Report the answer
			**reinterpret_cast<Byte**>(data) = mExcludeNonDALAccess ? 1 : 0;
		}
		else if (PropertyAddress::IsSameAddress(address, kDeviceIsRunningSomewhereAddress))
		{
			// This answer is a Byte
			size = sizeof(Byte);
			ThrowIfKernelError(vm_allocate(mach_task_self(), reinterpret_cast<vm_address_t*>(data), size, true), CAException(-1), "Device::GetPropertyState: allocation failed for kDeviceIsRunningSomewhereAddress");

			// If currently streaming, then the device "is running somewhere"
			**reinterpret_cast<Byte**>(data) = AnyStreamRunning() ? 1 : 0;
		}
		else if (PropertyAddress::IsSameAddress(address, kClientSyncDiscontinuityAddress))
		{
			// This answer is a Byte
			size = sizeof(Byte);
			ThrowIfKernelError(vm_allocate(mach_task_self(), reinterpret_cast<vm_address_t*>(data), size, true), CAException(-1), "Device::GetPropertyState: allocation failed for kClientSyncDiscontinuityAddress");

			// Report the answer
			**reinterpret_cast<Byte**>(data) = mForceDiscontinuity ? 1 : 0;
		}
		else if (kCMIOStreamPropertyFormatDescriptions == address.mSelector)
		{
			// This answer is an array of FrameFormats
			size = GetStreamByScopeAndElement(address.mScope, address.mElement).GetFrameFormats(reinterpret_cast<FrameFormat**>(data));
		}
		else if (kCMIOStreamPropertyFormatDescription == address.mSelector)
		{
			// This answer is a FrameType
			size = sizeof(FrameType);
			ThrowIfKernelError(vm_allocate(mach_task_self(), reinterpret_cast<vm_address_t*>(data), size, true), CAException(-1), "Device::GetPropertyState: allocation failed for kCMIOStreamPropertyFormatDescription");

			// Report the answer
			**reinterpret_cast<FrameType**>(data) = GetStreamByScopeAndElement(address.mScope, address.mElement).GetFrameType();
		}
		else if (kCMIOStreamPropertyFrameRates == address.mSelector)
		{
			// This answer is an array of Float64s
			size =  GetStreamByScopeAndElement(address.mScope, address.mElement).GetFrameRates(reinterpret_cast<FrameType*>(qualifier), reinterpret_cast<Float64**>(data));
		}
		else if (kCMIOStreamPropertyFrameRate == address.mSelector)
		{
			// This answer is a Float64
			size = sizeof(Float64);
			ThrowIfKernelError(vm_allocate(mach_task_self(), reinterpret_cast<vm_address_t*>(data), size, true), CAException(-1), "Device::GetPropertyState: allocation failed for kCMIOStreamPropertyFrameRate");

			// Report the answer
			**reinterpret_cast<Float64**>(data) = GetStreamByScopeAndElement(address.mScope, address.mElement).GetFrameRate();
		}
		else if (kCMIOStreamPropertyNoDataTimeoutInMSec == address.mSelector)
		{
			// This answer is a UInt32
			size = sizeof(UInt32);
			ThrowIfKernelError(vm_allocate(mach_task_self(), reinterpret_cast<vm_address_t*>(data), size, true), CAException(-1), "Device::GetPropertyState: allocation failed for kCMIOStreamPropertyNoDataTimeoutInMSec");

			// Report the answer
			**reinterpret_cast<UInt32**>(data) = GetStreamByScopeAndElement(address.mScope, address.mElement).GetNoDataTimeout();
		}
		else if (kCMIOStreamPropertyNoDataEventCount == address.mSelector)
		{
			// This answer is a UInt32
			vm_size_t size = sizeof(UInt32);
			ThrowIfKernelError(vm_allocate(mach_task_self(), reinterpret_cast<vm_address_t*>(data), size, true), CAException(-1), "Device::GetPropertyState: allocation failed for kCMIOStreamPropertyNoDataEventCount");

			// Report the answer
			**reinterpret_cast<UInt32**>(data) = GetStreamByScopeAndElement(address.mScope, address.mElement).GetNoDataEventCount();
		}
		else if (kCMIOStreamPropertyDeviceSyncTimeoutInMSec == address.mSelector)
		{
			// This answer is a UInt32
			vm_size_t size = sizeof(UInt32);
			ThrowIfKernelError(vm_allocate(mach_task_self(), reinterpret_cast<vm_address_t*>(data), size, true), CAException(-1), "Device::GetPropertyState: allocation failed for kCMIOStreamPropertyDeviceSyncTimeoutInMSec");

			// Report the answer
			**reinterpret_cast<UInt32**>(data) = GetStreamByScopeAndElement(address.mScope, address.mElement).GetDeviceSyncTimeout();
		}
		else if (kCMIOStreamPropertyEndOfData == address.mSelector)
		{
			// This answer is a Byte
			size = sizeof(Byte);
			ThrowIfKernelError(vm_allocate(mach_task_self(), reinterpret_cast<vm_address_t*>(data), size, true), CAException(-1), "Device::GetPropertyState: allocation failed for kCMIOStreamPropertyEndOfData");

			// Report the answer
			**reinterpret_cast<Byte**>(data) = GetStreamByScopeAndElement(address.mScope, address.mElement).GetEndOfData() ? 1 : 0;
		}
		else if (kCMIOStreamPropertyDeck == address.mSelector)
		{
			// This answer is a CMIOStreamDeck
			size = sizeof(CMIOStreamDeck);
			ThrowIfKernelError(vm_allocate(mach_task_self(), reinterpret_cast<vm_address_t*>(data), size, true), CAException(-1), "Device::GetPropertyState: allocation failed for kCMIOStreamPropertyDeck");
			
			// Report the answer
			**reinterpret_cast<CMIOStreamDeck**>(data) = GetStreamByScopeAndElement(address.mScope, address.mElement).GetStreamDeck();
		}
		else if (kCMIOStreamPropertyDeckFrameNumber == address.mSelector)
		{
			// This answer is a Float64
			size = sizeof(Float64);
			ThrowIfKernelError(vm_allocate(mach_task_self(), reinterpret_cast<vm_address_t*>(data), size, true), CAException(-1), "Device::GetPropertyState: allocation failed for kCMIOStreamPropertyDeckFrameNumber");
			
			// Report the answer
			**reinterpret_cast<Float64**>(data) = GetStreamByScopeAndElement(address.mScope, address.mElement).GetTimecode();
		}
		else if (kCMIOStreamPropertyDeckCueing == address.mSelector)
		{
			// This answer is a SInt32
			size = sizeof(SInt32);
			ThrowIfKernelError(vm_allocate(mach_task_self(), reinterpret_cast<vm_address_t*>(data), size, true), CAException(-1), "Device::GetPropertyState: allocation failed for kCMIOStreamPropertyDeckCueing");
			
			// Report the answer
			**reinterpret_cast<SInt32**>(data) = GetStreamByScopeAndElement(address.mScope, address.mElement).GetCueState();
		}
		else
		{
			// No match for the property address
			char selector[5] = CA4CCToCString(address.mSelector);
			char scope[5] = CA4CCToCString(address.mScope);
			DebugMessage("Device::GetPropertyState: unknown property address - %s - %s - %d", selector, scope, address.mElement);
			throw(CAException(kCMIOHardwareUnknownPropertyError));
		}

		// Report the length (in bytes)
		*length = size;
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// SetPropertyState()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void Device::SetPropertyState(Client client, bool sendChangedNotifications, const PropertyAddress& address, UInt8* qualifier, mach_msg_type_number_t qualifierLength, Byte* data, mach_msg_type_number_t length)
	{
		// Make sure the device is allowed to be changed by this client
		ThrowIf(not ClientIsDeviceMasterOrIsFree(client), CAException(kCMIODevicePermissionsError), "Device::SetPropertyState: client not allowed to set properties on device");

		// Grab the mutex for the Device's state
		CAMutex::Locker locker(mStateMutex);

		if (PropertyAddress::IsSameAddress(address, kDeviceMasterAddress))
		{
			ThrowIf(sizeof(pid_t) != length, CAException(kCMIOHardwareBadPropertySizeError), "Device::SetPropertySate: wrong size of kDeviceMasterAddress");
			
			// The device mastership can only be changed if it currenlty has no master (-1) or the requester is the current master (mDeviceMasterClient == client)
			ThrowIf((-1 != mDeviceMaster and mDeviceMasterClient != client), CAException(kCMIODevicePermissionsError), "Device::SetPropertyState: kCMIODevicePropertyDeviceMaster owned by another process");
			mProperties[kDeviceMasterAddress].mShadowTime = CAHostTimeBase::GetTheCurrentTime();
			pid_t deviceMaster = *reinterpret_cast<pid_t*>(data);
			
			if (-1 == deviceMaster)
			{
				// Mastership is being released
				mDeviceMasterClient = MACH_PORT_NULL;
			}
			else
			{
				// Mastership is being taken
				mDeviceMasterClient = client;
			}
			
			mDeviceMaster = deviceMaster;
		}
		else if (PropertyAddress::IsSameAddress(address, kExcludeNonDALAccessAddress))
		{
			ThrowIf(sizeof(Byte) != length, CAException(kCMIOHardwareBadPropertySizeError), "Device::SetPropertySate: wrong size of kExcludeNonDALAccessAddress");
			UInt32 excludeNonDALAccess = *reinterpret_cast<Byte*>(data);

			// Don't do anything if there are no changes
			if (excludeNonDALAccess == mExcludeNonDALAccess)
				return;

			if (0 == excludeNonDALAccess)
			{
				// Non-DAL access is being allowed
				// DoWhatIsNeededToEnableNonDALAccess()
				mExcludeNonDALAccessClient = MACH_PORT_NULL;
			}
			else
			{
				// Non-DAL access is being excluded
				// DoWhatIsNeededToExcludeNonDALAccess()
				mExcludeNonDALAccessClient = client;
			}
			
			mProperties[kExcludeNonDALAccessAddress].mShadowTime = CAHostTimeBase::GetTheCurrentTime();
			mExcludeNonDALAccess = excludeNonDALAccess;
		}
		else if (PropertyAddress::IsSameAddress(address, kClientSyncDiscontinuityAddress))
		{
			ThrowIf(sizeof(Byte) != length, CAException(kCMIOHardwareBadPropertySizeError), "Device::SetPropertySate: wrong size of kClientSyncDiscontinuityAddress");
			bool forceDiscontinuity = *reinterpret_cast<Byte*>(data);

			// Don't do anything if there are no changes
			if (forceDiscontinuity == mForceDiscontinuity)
				return;

			mProperties[kClientSyncDiscontinuityAddress].mShadowTime = CAHostTimeBase::GetTheCurrentTime();
			mForceDiscontinuity = forceDiscontinuity;
		}
		else if (kCMIOStreamPropertyFormatDescription == address.mSelector)
		{
			ThrowIf(sizeof(FrameType) != length, CAException(kCMIOHardwareBadPropertySizeError), "Device::SetPropertySate: wrong size of kCMIOStreamPropertyFormatDescription");
			FrameType frameType = *reinterpret_cast<FrameType*>(data);
			
			// Set the FrameType
			GetStreamByScopeAndElement(address.mScope, address.mElement).SetFrameType(frameType);
		}
		else if (kCMIOStreamPropertyFrameRate == address.mSelector)
		{
			ThrowIf(sizeof(Float64) != length, CAException(kCMIOHardwareBadPropertySizeError), "Device::SetPropertySate: wrong size of kCMIOStreamPropertyFrameRate");
			Float64 frameRate = *reinterpret_cast<Float64*>(data);
			
			// Set the frame rate
			GetStreamByScopeAndElement(address.mScope, address.mElement).SetFrameRate(frameRate);
		}
		else if (kCMIOStreamPropertyNoDataTimeoutInMSec == address.mSelector)
		{
			ThrowIf(sizeof(UInt32) != length, CAException(kCMIOHardwareBadPropertySizeError), "Device::SetPropertySate: wrong size of kCMIOStreamPropertyNoDataTimeoutInMSec");
			UInt32 noDataTimeout = *reinterpret_cast<UInt32*>(data);

			// Set the no data timeout
			GetStreamByScopeAndElement(address.mScope, address.mElement).SetNoDataTimeout(noDataTimeout);
		}
		else if (kCMIOStreamPropertyDeviceSyncTimeoutInMSec == address.mSelector)
		{
			ThrowIf(sizeof(UInt32) != length, CAException(kCMIOHardwareBadPropertySizeError), "Device::SetPropertySate: wrong size of kDeviceSyncTimeoutAddress");
			UInt32 deviceSyncTimeout = *reinterpret_cast<UInt32*>(data);

			// Set the device sync timeout
			GetStreamByScopeAndElement(address.mScope, address.mElement).SetDeviceSyncTimeout(deviceSyncTimeout);
		}
		else if (kCMIOStreamPropertyEndOfData == address.mSelector)
		{
			ThrowIf(sizeof(Byte) != length, CAException(kCMIOHardwareBadPropertySizeError), "Device::SetPropertySate: wrong size of kCMIOStreamPropertyEndOfData");
			bool endOfData = *reinterpret_cast<Byte*>(data);

			// Set the end-of-data marker
			GetStreamByScopeAndElement(address.mScope, address.mElement).SetEndOfData(endOfData);
		}	
		else
		{
			// No match for the property address
			char selector[5] = CA4CCToCString(address.mSelector);
			char scope[5] = CA4CCToCString(address.mScope);
			DebugMessage("Device::SetPropertyState: unknown property address - %s - %s - %d", selector, scope, address.mElement);
			throw(CAException(kCMIOHardwareUnknownPropertyError));
		}

		// Inform the clients that the property states have changed if needed
		if (sendChangedNotifications)
			SendPropertyStatesChangedMessage();
	}

	#pragma mark -
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// SendPropertyStatesChangedMessage()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void Device::SendPropertyStatesChangedMessage()
	{
		// Statically initialize the invariant portions of the message (using 'safe' values for the variable portion).
		// (Note:  for messages with 'move' dispostions such as this one, the kernel will not alter the invariant portion of the message, so this is a safe optimization.)
		static PropertyStatesChangedMessage message =
		{
			{
				MACH_MSGH_BITS_REMOTE(MACH_MSG_TYPE_MOVE_SEND_ONCE),						// mHeader.msgh_bits
				sizeof(PropertyStatesChangedMessage),										// mHeader.msgh_size
				MACH_PORT_NULL,																// mHeader.msgh_remote_port
				MACH_PORT_NULL,																// mHeader.msgh_local_port
				0,																			// mHeader.msgh_reserved
				kPropertyStatesChanged														// mHeader.msgh_id
			}
		};
		
		// Grab the mutex for the property state notifiers
		CAMutex::Locker locker(mPropertyStateNotifiersMutex);
		
		for (ClientNotifiers::iterator i = mPropertyStateNotifiers.begin() ; i != mPropertyStateNotifiers.end() ; ++i)
		{
			// Update the variable portion of the message
			message.mHeader.msgh_remote_port = (*i).second;

			// Send the message
			mach_msg_return_t err = mach_msg(&message.mHeader, MACH_SEND_MSG, message.mHeader.msgh_size, 0, MACH_PORT_NULL, 0, MACH_PORT_NULL);
			if (MACH_MSG_SUCCESS != err)
			{
				// Delivery of the message failed, most likely because the client terminated, so simply destroy the message and continue
				mach_msg_destroy(&message.mHeader);
				DebugMessage("Device::SendPropertyStatesChangedMessage() - Error sending notification to port %d - 0x%08X", (*i).second, err);
			}
		}

		// All the 'send-once' rights are now used up, so erase all the property state notifiers 
		mPropertyStateNotifiers.erase(mPropertyStateNotifiers.begin(), mPropertyStateNotifiers.end());
	}

	#pragma mark -
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// GetControls()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void Device::GetControls(Client client, mach_port_t messagePort, UInt64 time, ControlChanges** controlChanges, mach_msg_type_number_t* length)
	{
		// Grab the mutex for the Device's state
		CAMutex::Locker locker(mStateMutex);
		
		// Add the <client, messagePort> pair to the multimap of clients to notify for future state changes to controls
		if (MACH_PORT_NULL != messagePort)
			mControlStateNotifiers.insert(std::make_pair(client, messagePort));
		
		// Assume no controls have changed since the indicated time
		*length = 0;
		
		std::vector<ControlChanges> changedControls;
		// Iterate over the controls and find out how many have had their value or range changed since the indicated time
		for (Controls::iterator i = mControls.begin() ; i != mControls.end() ; std::advance(i, 1))
		{
			if (time < (*i).second.mValueShadowTime or time < (*i).second.mRangeShadowTime)
			{
				changedControls.push_back(ControlChanges((*i).first, time < (*i).second.mValueShadowTime, time < (*i).second.mRangeShadowTime));
			}
		}
		
		// Make sure there are some changed controls
		if (0 == changedControls.size())
			return;
			
		// Allocate the memory need for returning the control change information
		kern_return_t err =  vm_allocate(mach_task_self(), reinterpret_cast<vm_address_t*>(controlChanges), sizeof(ControlChanges) * changedControls.size(), true);
		ThrowIfKernelError(err, CAException(err), "Device::GetControls couldn't allocate storage for ControlChanges[]");
		
		// Fill in the controls which have changed since the inidicated time
		memcpy(*controlChanges, &(*changedControls.begin()), sizeof(ControlChanges) * changedControls.size());

		// Report the number of changed controls
		*length = changedControls.size();
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	//  SetControl()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void Device::SetControl(Client client, UInt32 controlID, UInt32 value, UInt32* newValue)
	{
		// Make sure the request is for a known control
		ThrowIf(mControls.end() == mControls.find(controlID), CAException(kCMIOHardwareUnsupportedOperationError), "Device::SetControl: unknown control ID");

		// Make sure the device is allowed to be changed by this client
		ThrowIf(not ClientIsDeviceMasterOrIsFree(client), CAException(kCMIODevicePermissionsError), "Device::SetControl: client not allowed to set controls on device");

		// Grab the mutex for the Device's state
		CAMutex::Locker locker(mStateMutex);
		
		switch (controlID)
		{
			case KEXT::Sample::kDirectionControlID:
				{
					RemoveStreams();
					mIOVADevice.SetControl(controlID, value, newValue);
					DiscoverStreams();
				}
				break;
			
			case KEXT::Sample::kInputSourceSelectorControlID:
				{
					mIOVADevice.SetControl(controlID, value, newValue);
				}
				break;
				
			case CMIO::DP::Sample::kProtocolSelectorControlID:
				{
					if (mControlsList.IsValid())
					{
						CFMutableDictionaryRef theControlDictionary = CMIO::DP::ControlDictionary::GetControlByID(mControlsList, controlID);
						if (NULL != theControlDictionary)
						{
							UInt32	currentSelection;
							//	set the new value in the registry
							CACFDictionary theCAControlDictionary(theControlDictionary, false);
							currentSelection = CMIO::DP::ControlDictionary::GetSelectorControlValue(theCAControlDictionary);
							if (currentSelection != value)
							{
								CMIO::DP::ControlDictionary::SetSelectorControlValue(theCAControlDictionary, value);
								
								//	set the new value as the return value
								if (NULL != newValue)
								{
									*newValue = value;
								}
								mControls[controlID].mValueShadowTime = CAHostTimeBase::GetTheCurrentTime();
								mControls[controlID].mRangeShadowTime = CAHostTimeBase::GetTheCurrentTime();
							}
						}
					}
				}
				break;
		}
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// RS422Command()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	IOReturn Device::RS422Command(const UInt8 *command, UInt32 commandLength, UInt8 *response, UInt32 *responseLength)
	{
		DebugMessage("Device::RS422Command: command Length = %d cmd %x %x %x %x", commandLength, *command++, *command++, *command++, *command++);
		*responseLength = 4;
		*response++ = 1;
		*response++ = 2;
		*response++ = 3;
		*response++ = 4;
		return noErr; 
	}
	
	#pragma mark -
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// SendControlStatesChangedMessage()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void Device::SendControlStatesChangedMessage()
	{
		// Statically initialize the invariant portions of the message (using 'safe' values for the variable portion).
		// (Note:  for messages with 'move' dispostions such as this one, the kernel will not alter the invariant portion of the message, so this is a safe optimization.)
		static ControlStatesChangedMessage message =
		{
			{
				MACH_MSGH_BITS_REMOTE(MACH_MSG_TYPE_MOVE_SEND_ONCE),						// mHeader.msgh_bits
				sizeof(ControlStatesChangedMessage),										// mHeader.msgh_size
				MACH_PORT_NULL,																// mHeader.msgh_remote_port
				MACH_PORT_NULL,																// mHeader.msgh_local_port
				0,																			// mHeader.msgh_reserved
				kControlStatesChanged														// mHeader.msgh_id
			}
		};
		
		for (ClientNotifiers::iterator i = mControlStateNotifiers.begin() ; i != mControlStateNotifiers.end() ; ++i)
		{
			// Update the variable portion of the message
			message.mHeader.msgh_remote_port = (*i).second;

			// Send the message
			mach_msg_return_t err = mach_msg(&message.mHeader, MACH_SEND_MSG, message.mHeader.msgh_size, 0, MACH_PORT_NULL, 0, MACH_PORT_NULL);
			if (MACH_MSG_SUCCESS != err)
			{
				// Delivery of the message failed, most likely because the client terminated, so simply destroy the message and continue
				mach_msg_destroy(&message.mHeader);
				DebugMessage("Device::SendControlStatesChangedMessage() - Error sending notification to port %d - 0x%08X", (*i).second, err);
			}
		}

		// All the 'send-once' rights are now used up, so erase everything multimap 
		mControlStateNotifiers.erase(mControlStateNotifiers.begin(), mControlStateNotifiers.end());
	}

	#pragma mark -
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// InitializeControls()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void Device::InitializeControls()
	{
		{
			// Get the kext control list from the IORegistry
			CACFArray controlList = CACFArray(static_cast<CFArrayRef>(IORegistryEntryCreateCFProperty(mRegistryEntry, CFSTR(kIOVideoDeviceKey_ControlList), NULL, 0)), true);

			// Don't do anything if the list is not valid
			if (not controlList.IsValid())
				return;

			// Use "now" as the shadow time for when the controls were last changed
			UInt64 shadowTime = CAHostTimeBase::GetTheCurrentTime();

			// Iterate over the controls
			UInt32 controlCount = controlList.GetNumberItems();
			for (UInt32 index = 0 ; index < controlCount ; ++index)
			{
				// Get the control dictionary:DP::ControlDictionary::S
				CFDictionaryRef controlDictionary = NULL;
				if (controlList.GetDictionary(index, controlDictionary))
				{
					// Extract the control ID
					UInt32 controlID = DP::IOV::ControlDictionary::GetControlID(controlDictionary);
					
					// Make an entry for it in the map of Controls
					mControls[controlID].mValueShadowTime = shadowTime;
					mControls[controlID].mRangeShadowTime = shadowTime;
				}
			}
		}
		
		//Create the plugin based controls
		{
			CACFArray			theSourceSelectorMap = NULL;
			CFDictionaryRef		theSelectorItem = NULL;
			CFDictionaryRef		theSelectorControl = NULL;
			theSourceSelectorMap = CACFArray(3, false);
			//		CFArray::withCapacity(3);
			if (!theSourceSelectorMap.IsValid())
			{
				DebugMessage("Device::InitializeControls: couldn't allocate the source selector map array");
			}
			
			theSelectorItem = CMIO::DP::ControlDictionary::CreateSelectorControlSelectorMapItem((UInt32)CMIO::DP::Sample::kProtocolSample, CACFString(CFSTR("Sample")));
			theSourceSelectorMap.AppendDictionary(theSelectorItem);
			theSelectorItem = CMIO::DP::ControlDictionary::CreateSelectorControlSelectorMapItem((UInt32)CMIO::DP::Sample::kProtocolSampleBasic, CACFString(CFSTR("Sample Basic")));
			theSourceSelectorMap.AppendDictionary(theSelectorItem);
			theSelectorItem = CMIO::DP::ControlDictionary::CreateSelectorControlSelectorMapItem((UInt32)CMIO::DP::Sample::kProtocolSampleAdvanced, CACFString(CFSTR("Sample Advanced")));
			theSourceSelectorMap.AppendDictionary(theSelectorItem);
			
			//	create a custom play through boolean control
			theSelectorControl = CMIO::DP::ControlDictionary::CreateSelectorControl(CMIO::DP::Sample::kProtocolSelectorControlID, kCMIOSelectorControlClassID, kCMIOSelectorControlClassID, kCMIOObjectPropertyScopeGlobal, kCMIOObjectPropertyElementMaster, 1, theSourceSelectorMap, CACFString(CFSTR("Protocol")), false, 0);
			if (NULL == theSelectorControl)
			{
				DebugMessage("Device::InitializeControls: couldn't allocate the theSelectorControl control");
			}
			
			mControlsList.AppendDictionary(theSelectorControl); 
			
			// Use "now" as the shadow time for when the controls were last changed
			UInt64 shadowTime = CAHostTimeBase::GetTheCurrentTime();
			
			// Iterate over the controls
			UInt32 controlCount = mControlsList.GetNumberItems();
			for (UInt32 index = 0 ; index < controlCount ; ++index)
			{
				// Get the control dictionary
				CFDictionaryRef controlDictionary = NULL;
				if (mControlsList.GetDictionary(index, controlDictionary))
				{
					CFShow(controlDictionary);
					// Extract the control ID
					CACFDictionary caControlDictionary(controlDictionary, false);
					UInt32 controlID = CMIO::DP::ControlDictionary::GetControlID(caControlDictionary);
					
					// Make an entry for it in the map of Controls
					// Make an entry for it in the map of Controls
					mControls[controlID].mValueShadowTime = shadowTime;
					mControls[controlID].mRangeShadowTime = shadowTime;
				}
			}
		}
		
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// InitializeProperties()
	//	The map-subscripting operator causes pair(PropertyAddress, PropertyShadow()) to be inserted into the map if it currently lacks the key, so this used to specify which properties are
	//	present.  Subsequently, GetProperties() will only report properties which have been inserted here (or in a stream)
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void Device::InitializeProperties()
	{
		UInt64 shadowTime = CAHostTimeBase::GetTheCurrentTime();

		// Properties at the device level
		mProperties[kInputStreamConfigurationAddress].mShadowTime = shadowTime;
		mProperties[kOutputStreamConfigurationAddress].mShadowTime = shadowTime;
		mProperties[kHogModeAddress].mShadowTime = shadowTime;
		mProperties[kDeviceMasterAddress].mShadowTime = shadowTime;
		mProperties[kExcludeNonDALAccessAddress].mShadowTime = shadowTime;
		mProperties[kDeviceIsRunningSomewhereAddress].mShadowTime = shadowTime;
	}

	#pragma mark -
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// Sleep()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void Device::Sleep()
	{
		// This shoud stop all the streams that are currently running
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// Wake()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void Device::Wake()
	{
		// This should start all the streams were stopped by sleep
	}

	#pragma mark -
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// GetControlList()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void Device::GetControlList(UInt8** data, mach_msg_type_number_t* length)
	{
		// Grab the mutex for the Device's state
		CAMutex::Locker locker(mStateMutex);
		
		// Assume no addresses will satisfy the match criteria
		*length = 0;
		vm_size_t size = 0;
		
		if (mControlsList.IsValid())
		{
		
			#if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_6)
				CACFData xmlData(CFPropertyListCreateData(kCFAllocatorDefault, mControlsList.AsPropertyList(), kCFPropertyListXMLFormat_v1_0, 0, NULL));
			#else
				CACFData xmlData(CFPropertyListCreateXMLData(kCFAllocatorDefault, mControlsList.AsPropertyList()));
			#endif

			if (xmlData.IsValid())
			{
				size = xmlData.GetSize();
				ThrowIfKernelError(vm_allocate(mach_task_self(), reinterpret_cast<vm_address_t*>(data), size, true), CAException(-1), "Device::GetControlList: allocation failed for returning xmlData");
				CFDataGetBytes(xmlData.GetCFData(), CFRangeMake(0, size), *data);
			}
		}
		
		// Report the length (in bytes)
		*length = size;
	}
	
	
	#pragma mark -
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// IOVDeviceNotification()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void Device::IOVDeviceNotification(IOVideoDeviceRef /*deviceRef*/, Device& device, const IOVideoDeviceNotificationMessage& message)
	{
		// Grab the mutex for the device's state
		CAMutex::Locker locker(device.mStateMutex);
		
		// Assume this notification is not about a control change
		bool sendControlStatesChangedMessage = false;
		
		// Record the shadow time in the event a control as been altered
		UInt64 shadowTime = CAHostTimeBase::GetTheCurrentTime();

		#if Log_HardareNotifications
			DebugMessage("Device::IOVDeviceNotification: received %lu messages", message.mNumberNotifications);
		#endif
		
		// A message can contain many notifications, so iterate through them
		for (UInt32 notificationindex = 0 ; notificationindex < message.mNumberNotifications ; ++notificationindex)
		{
			#if Log_HardareNotifications
				char notificationIDString[] = CA4CCToCString(message.mNotifications[notificationindex].mNotificationID);
				DebugMessage("Device::IOVDeviceNotification: Handling notification %lu: Object ID: %lu Notification ID: '%s' (%lu, %lu, %qd, %qd)", notificationindex, message.mNotifications[notificationindex].mObjectID, notificationIDString, message.mNotifications[notificationindex].mNotificationArgument1, message.mNotifications[notificationindex].mNotificationArgument2, message.mNotifications[notificationindex].mNotificationArgument3, message.mNotifications[notificationindex].mNotificationArgument4);
			#endif
			
			// Figure out what object this notification is for
			if (0 == message.mNotifications[notificationindex].mObjectID)
			{
				#if Log_HardareNotifications
					DebugMessage("Device::IOVDeviceNotification: Notification %lu is a device notification", notificationIndex);
				#endif
				
				// This is a device level notification
				device.DeviceNotification(message.mNotifications[notificationindex]);
			}
			else
			{
				// Check to see if this object is a stream
				Stream* stream = device.GetStreamByStreamID(message.mNotifications[notificationindex].mObjectID);
				if (NULL != stream)
				{
					device.StreamNotification(message.mNotifications[notificationindex], *stream);
				}
				else
				{
					// Check to see if this object is a control
					Controls::const_iterator i = device.mControls.find(message.mNotifications[notificationindex].mObjectID);
					if (i != device.mControls.end())
					{
						device.ControlNotification(message.mNotifications[notificationindex], shadowTime);
						
						// Since some control was altered, remember that a "controls changed" message will have to be sent
						sendControlStatesChangedMessage = true;
					}
				}
			}
		}
		
		if (sendControlStatesChangedMessage)
			device.SendControlStatesChangedMessage();
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// DeviceNotification()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void Device::DeviceNotification(const IOVideoDeviceNotification& notification)
	{
		switch (notification.mNotificationID)
		{
			default:
				DebugMessage("Device::HandleDeviceNotification: unusual notification type");
				break;
		};
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// StreamNotification()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void Device::StreamNotification(const IOVideoDeviceNotification& /*notification*/, Stream& /*stream*/)
	{
	}
	
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// ControlNotification()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void Device::ControlNotification(const IOVideoDeviceNotification& notification, UInt64 shadowTime)
	{
		switch (notification.mNotificationID)
		{
			case kIOVideoDeviceNotificationID_ControlValueChanged:
				mControls[notification.mObjectID].mValueShadowTime = shadowTime;
				break;
			
			case kIOVideoDeviceNotificationID_ControlRangeChanged:
				mControls[notification.mObjectID].mRangeShadowTime = shadowTime;
				break;
						
			default:
				DebugMessage("Device::ControlNotification: unusual notification");
				break;
		};
	}
	
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// PowerNotification()
	//	Which notifications are handled (and how they are handled) can vary based on the device.
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void Device::PowerNotification(Device& device, io_service_t unused, natural_t messageType, void* message)
	{
		// Catch all exceptions since this is invoked via a call back and the exception cannot leave this routine 
		try
		{
			switch (messageType)
			{
				case kIOMessageCanSystemSleep:
					(void) IOAllowPowerChange(device.mPowerNotificationPort.GetRootPowerDomain(), reinterpret_cast<long>(message));
					break;
					
				case kIOMessageSystemWillSleep:
					device.Sleep();
					(void) IOAllowPowerChange(device.mPowerNotificationPort.GetRootPowerDomain(), reinterpret_cast<long>(message));
					break;
					
				case kIOMessageSystemHasPoweredOn:
					device.Wake();
					break;
					
				default:
					break;
			}
		}
		catch (...)
		{
		}
	}
}}}}
