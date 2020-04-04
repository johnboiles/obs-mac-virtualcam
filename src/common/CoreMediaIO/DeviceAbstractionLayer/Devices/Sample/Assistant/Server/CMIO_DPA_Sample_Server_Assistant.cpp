/*
	    File: CMIO_DPA_Sample_Server_Assistant.cpp
	Abstract: Server which handles all the IPC between the various Sample DAL PlugIn instances.
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
#include "CMIO_DPA_Sample_Server_Assistant.h"

// Internal Includes
#include "CMIO_DPA_Sample_Server_Device.h"

// Public Utility Includes
#include "CMIODebugMacros.h"

// CA Public Utility Includes
#include "CAAutoDisposer.h"
#include "CACFNumber.h"
#include "CACFObject.h"

// System Includes
#include <CoreAudio/HostTime.h>
#include <mach-o/dyld.h>

// Standard Library Includes
#include <algorithm>

namespace
{
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// CopyPlugInBundle()
	//	This locates the com.apple.cmio.DAL.Sample bundle so localized device names can be extracted from it.
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	CFBundleRef CopyPlugInBundle()
	{
		// Get the path of the Assistant
		uint32_t pathLength = MAXPATHLEN;
		CAAutoFree<char> path(pathLength + 1, true);
		if (0 == _NSGetExecutablePath(path.get(), &pathLength))
		{
			pathLength = ToUInt32(strlen(path.get()));
		}
		else
		{
			// Try again with actual path length
			path.allocBytes(pathLength);
			ThrowIf(noErr != _NSGetExecutablePath(path.get(), &pathLength), CAException(-1), "CopyPlugInBundle: _NSGetExecutablePath failed"); 
		}

		// Get the Assistant's URL
		CACFURL assistantURL(CFURLCreateFromFileSystemRepresentation(NULL, reinterpret_cast<UInt8*>(path.get()), pathLength, false));
		ThrowIf(not assistantURL.IsValid(), CAException(-2), "CopyPlugInBundle: unable to create URL for the Assistant");

		// Create an URL to the DAL plugIn in which the Assistant is embedded by deleting the last 3 path components 
		CACFURL resoursesURL(CFURLCreateCopyDeletingLastPathComponent(NULL, assistantURL.GetCFObject()));
		CACFURL contentsURL(CFURLCreateCopyDeletingLastPathComponent(NULL, resoursesURL.GetCFObject()));
		CACFURL plugInURL(CFURLCreateCopyDeletingLastPathComponent(NULL, contentsURL.GetCFObject()));
		ThrowIf(not plugInURL.IsValid(), CAException(-3), "CopyPlugInBundle: unable to create URL for the Sample.plugin");

		// Get the plugIn's bundle
		CACFBundle bundle(CFBundleCreate(0, plugInURL.GetCFObject()), false);
		ThrowIf(not bundle.IsValid(), CAException(-4), "CopyPlugInBundle: Sample.plugin bundle not valid");
		
		return bundle.GetCFObject();
	}
}

#pragma mark -
namespace CMIO { namespace DPA { namespace Sample { namespace Server
{
	#pragma mark -
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// Assistant()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	Assistant::Assistant() :
		mStateMutex("SampleAssistant state mutex"),
		mPlugInBundle(CopyPlugInBundle()),
		mPortSet(MACH_PORT_NULL),
		mDevices(),
		mClientInfoMap(),
		mDeviceStateNotifiers()
	{
		// Create a port set to hold the service port, and each client's port
		kern_return_t err = mach_port_allocate(mach_task_self(), MACH_PORT_RIGHT_PORT_SET, &mPortSet);
		ThrowIfKernelError(err, CAException(err), "Unable to create port set");
	}
	
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// ~Assistant()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	Assistant::~Assistant()
	{
		// Delete all Devices
		for (Devices::iterator i = mDevices.begin() ; i != mDevices.end() ; ++i)
			delete *i;
	}
	
	#pragma mark -
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// Connect()
	//	This is the first call that clients invoke to establish a connection with the Assistant.  The Assistant will create a unique port for the client, add it to the portset it services,
	//	and a return a send right to the client (via the MIG reply).  The Assistant will request a "no-sender" notification on that newly created port, thus allowing it to get a notification
	//	in the event the client is terminated.
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	kern_return_t Assistant::Connect(mach_port_t servicePort, pid_t clientPID, mach_port_t* client)
	{
		*client = MACH_PORT_NULL;
		
		try
		{
			// Create a port for the new client to send messages to the Assistant
			kern_return_t err = mach_port_allocate(mach_task_self(), MACH_PORT_RIGHT_RECEIVE, client);
			ThrowIfKernelError(err, CAException(err), "CMIO::DPA::Sample::Assistant::Connect: mach_port_allocate() failed"); 
			
			// Request a notification if the client loses all of its senders.
			mach_port_t previous;
			err = mach_port_request_notification(mach_task_self(), *client, MACH_NOTIFY_NO_SENDERS, 1, *client, MACH_MSG_TYPE_MAKE_SEND_ONCE, &previous);
			ThrowIfKernelError(err, CAException(err), "CMIO::DPA::Sample::Assistant::Connect: mach_port_request_notification() failed"); 

			// Add the client port to the Assistant's port set
			err = mach_port_move_member(mach_task_self(), *client, GetPortSet());
			ThrowIfKernelError(err, CAException(err), "CMIO::DPA::Sample::Assistant::Connect: mach_port_move_member() failed"); 

			// Add the <Client, ClientInfo*> pair to the ClientInfoMap
			mClientInfoMap[*client] = new ClientInfo(clientPID);
		}
		catch (...)
		{
			if (MACH_PORT_NULL != *client)
			{
				(void) mach_port_destroy(mach_task_self(), *client);
				*client = MACH_PORT_NULL;
			}

			return -1;
		}

		return KERN_SUCCESS;
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// Disconnect()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	kern_return_t Assistant::Disconnect(Client client)
	{
		// Just forward this on to ClientDied(), since an explicit disconnect can be handled just like an accidetal death
		ClientDied(client);
		return KERN_SUCCESS;
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// ClientDied()
	//	The client which was sending messages to the Assisant on the indicicated port died
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void Assistant::ClientDied(Client client)
	{
		// Grab the mutex for the Assistant's state
		CAMutex::Locker locker(mStateMutex);

		// Remove the receive right on this port since it is no longer needed since the client that was sending on it died.  This will result in the port being destroyed and it being removed
		// from the Assistant's port set.
		(void) mach_port_mod_refs(mach_task_self(), client,  MACH_PORT_RIGHT_RECEIVE, -1);

		// Get the <Client, ClientInfo*> pair for the client
		ClientInfoMap::iterator i = mClientInfoMap.find(client);
		
		// Make sure the client was found
		if (i == mClientInfoMap.end())
			return;

		// Iterate over all the devices whose stream was being watched by the client and stop them
		while (not (*i).second->mStreamSpecifiers.empty())
		{
			StreamSpecifiers::iterator ii = (*i).second->mStreamSpecifiers.begin();
			(*ii).mDevice.StopStream(client, (*ii).mScope, (*ii).mElement);
			mClientInfoMap[client]->mStreamSpecifiers.erase(*ii);
		}
		
		// Deallocate the ClientInfo
		delete (*i).second;

		// Remove the <client, ClientInfo*> pair from the ClientInfoMap
		mClientInfoMap.erase(i);

		// Remove the client from any device added / removed notifications it might have signed up for
		while (true)
		{
			ClientNotifiers::iterator i = mDeviceStateNotifiers.find(client);
			if (i == mDeviceStateNotifiers.end())
				break;
			
			// Destroy the port used to send notificaitions to the (now dead) client
			(void) mach_port_destroy(mach_task_self(), (*i).second);
			
			// Erase this entry from the multimap
			mDeviceStateNotifiers.erase(i);
		}

		// Inform all devices that the client has died so they can revoke any properties (e.g., Device Master) the client might have had
		for (Devices::iterator i = mDevices.begin() ; i != mDevices.end() ; ++i)
			(**i).ClientDied(client);
			
		// If there are still clients connected to the Assistant, simply return
		if (not mClientInfoMap.empty())
			return;
	}

	#pragma mark -
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// GetDeviceStates()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	kern_return_t Assistant::GetDeviceStates(Client client, mach_port_t messagePort, DeviceState** deviceStates, mach_msg_type_number_t* length)
	{
		try
		{
			// Grab the mutex for the Assistant's state
			CAMutex::Locker locker(mStateMutex);
			
			// Add the <client, messagePort> pair to the multimap of clients to notify for future device state changes (added/removed)
			mDeviceStateNotifiers.insert(std::make_pair(client, messagePort));

			// Report the size of the variable length DeviceStates array
			*length = (mach_msg_type_number_t)mDevices.size();

			// Simply return if there are no devices present
			if (mDevices.empty())
				return KERN_SUCCESS;
				
			// Allocate the memory for returning the devices' state
			kern_return_t err =  vm_allocate(mach_task_self(), reinterpret_cast<vm_address_t*>(deviceStates), sizeof(DeviceState) * mDevices.size(), true);
			ThrowIfKernelError(err, CAException(err), "CMIO::DPA::Sample::Server::Assistant::GetDeviceStates couldn't allocate storage for DeviceState[]");
			
			// Populate the devices state array for all the devices currently present
			int index = 0;
			for (Devices::const_iterator i = mDevices.begin() ; i != mDevices.end() ; ++i)
			{
				(*deviceStates)[index].mGUID = (**i).GetDeviceGUID();
                (*deviceStates)[index].mIsIOBacked = false;
                
				++index;
			}
		}
		catch (CAException& exception)
		{
			return exception.GetError();
		}

		return KERN_SUCCESS;
	}

	#pragma mark -
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// GetProperties()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	kern_return_t Assistant::GetProperties(Client client, UInt64 guid, mach_port_t messagePort, UInt64 time, CMIOObjectPropertyAddress matchAddress, PropertyAddress** addresses, mach_msg_type_number_t* length)
	{
		try
		{
			// Grab the mutex for the Assistant's state
			CAMutex::Locker locker(mStateMutex);

			// Get the state of the device's controls
			GetDeviceByGUID(guid).GetProperties(client, messagePort, time, matchAddress, addresses, length);
			
		}
		catch (CAException& exception)
		{
			return exception.GetError();
		}
				
		return KERN_SUCCESS;
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// GetPropertyState()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	kern_return_t Assistant::GetPropertyState(Client client, UInt64 guid, CMIOObjectPropertyAddress address, UInt8* qualifier, mach_msg_type_number_t qualifierLength, UInt8** data, mach_msg_type_number_t* length)
	{
		try
		{
			// Grab the mutex for the Assistant's state
			CAMutex::Locker locker(mStateMutex);

			// Get the state of the device's controls
			GetDeviceByGUID(guid).GetPropertyState(address, qualifier, qualifierLength, data, length);
			
		}
		catch (CAException& exception)
		{
			return exception.GetError();
		}
				
		return KERN_SUCCESS;
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// SetPropertyState()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	kern_return_t Assistant::SetPropertyState(Client client, UInt64 guid, UInt32 sendChangedNotifications, CMIOObjectPropertyAddress address, UInt8* qualifier, mach_msg_type_number_t qualifierLength, Byte* data, mach_msg_type_number_t length)
	{
		try
		{
			// Grab the mutex for the Assistant's state
			CAMutex::Locker locker(mStateMutex);

			// Set the device property
			GetDeviceByGUID(guid).SetPropertyState(client, sendChangedNotifications, address, qualifier, qualifierLength, data, length);
		}
		catch (CAException& exception)
		{
			return exception.GetError();
		}
				
		return KERN_SUCCESS;
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// GetProperties()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	kern_return_t Assistant::GetControlList(Client client, UInt64 guid, UInt8** data, mach_msg_type_number_t* length)
	{
		try
		{
			// Grab the mutex for the Assistant's state
			CAMutex::Locker locker(mStateMutex);
			
			// Get the state of the device's controls
			GetDeviceByGUID(guid).GetControlList(data , length);
		}
		catch (CAException& exception)
		{
			return exception.GetError();
		}
		
		return KERN_SUCCESS;
	}
	
	#pragma mark -
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// GetControls()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	kern_return_t Assistant::GetControls(Client client, UInt64 guid, mach_port_t messagePort, UInt64 time, ControlChanges** controlChanges, mach_msg_type_number_t* length)
	{
		try
		{
			// Grab the mutex for the Assistant's state
			CAMutex::Locker locker(mStateMutex);

			// Get the state of the device's controls
			GetDeviceByGUID(guid).GetControls(client, messagePort, time, controlChanges, length);
			
		}
		catch (CAException& exception)
		{
			return exception.GetError();
		}
				
		return KERN_SUCCESS;
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// SetControl()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	kern_return_t Assistant::SetControl(Client client, UInt64 guid, UInt32 controlID, UInt32 value, UInt32* newValue)
	{
		try
		{
			// Grab the mutex for the Assistant's state
			CAMutex::Locker locker(mStateMutex);

			// Set the device property
			GetDeviceByGUID(guid).SetControl(client, controlID, value, newValue);
		}
		catch (CAException& exception)
		{
			return exception.GetError();
		}
				
		return KERN_SUCCESS;
	}

	#pragma mark -
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// StartStream()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	kern_return_t Assistant::StartStream(Client client, UInt64 guid, mach_port_t messagePort, CMIOObjectPropertyScope scope, CMIOObjectPropertyElement element)
	{
		try
		{
			// Grab the mutex for the Assistant's state
			CAMutex::Locker locker(mStateMutex);

			// Locate the device with the specified GUID
			Device& device = GetDeviceByGUID(guid);
			
			// Tell the device to start its stream
			device.StartStream(client, messagePort, scope, element);
			
			// Add the device to the set of streaming devices that the client is watching
			mClientInfoMap[client]->mStreamSpecifiers.insert(StreamSpecifier(device, scope, element));
		}
		catch (CAException& exception)
		{
			return exception.GetError();
		}
				
		return KERN_SUCCESS;
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// StopStream()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	kern_return_t Assistant::StopStream(Client client, UInt64 guid, CMIOObjectPropertyScope scope, CMIOObjectPropertyElement element)
	{
		try
		{
			// Grab the mutex for the Assistant's state
			CAMutex::Locker locker(mStateMutex);
			
			// Locate the device with the specified GUID
			Device& device = GetDeviceByGUID(guid);
				
			device.StopStream(client, scope, element);

			// Remove the device from the set of streaming devices that the client is watching
			mClientInfoMap[client]->mStreamSpecifiers.erase(StreamSpecifier(device, scope, element));
		}
		catch (CAException& exception)
		{
			return exception.GetError();
		}

		return KERN_SUCCESS;
	}

	#pragma mark -
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// ProcessRS422Command()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	kern_return_t Assistant::ProcessRS422Command(Client client, UInt64 guid, ByteArray512 command, mach_msg_type_number_t commandLength, UInt32 responseLength, UInt32 *responseUsed, UInt8** response, mach_msg_type_number_t *responseCount)
	{
		try
		{
			// Grab the mutex for the Assistant's state
			CAMutex::Locker locker(mStateMutex);

			// Report the size of the variable length response array
			*responseCount = responseLength;

			// Allocate the memory for returning the reponse
			kern_return_t err =  vm_allocate(mach_task_self(), reinterpret_cast<vm_address_t*>(response), sizeof(UInt8) * responseLength, true);
			ThrowIfKernelError(err, CAException(err), "CMIO::DPA::Sample::Server::Assistant::ProcessRS422Command couldn't allocate storage for response UInt8[]");
			
			// Process the command
			*responseUsed = responseLength;
			err = GetDeviceByGUID(guid).RS422Command(command, commandLength, *response, responseUsed);
			ThrowIfError(err, CAException(err), "CMIO::DPA::Sample::Server::Assistant::ProcessRS422Command: command failed");
		}	
		catch (CAException& exception)
		{
			return exception.GetError();
		}
				
		return KERN_SUCCESS;
	}

	#pragma mark -
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// StartDeckThreads()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	kern_return_t Assistant::StartDeckThreads(Client client, UInt64 guid, CMIOObjectPropertyScope scope, CMIOObjectPropertyElement element)
	{
		try
		{
			// Grab the mutex for the Assistant's state
			CAMutex::Locker locker(mStateMutex);
			
			// Locate the device with the specified GUID
			Device& device = GetDeviceByGUID(guid);
			
			// Tell the device to start its stream
			device.StartDeckThreads(client, scope, element);
			
		}
		catch (CAException& exception)
		{
			return exception.GetError();
		}
		
		return KERN_SUCCESS;
	}
	
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// StopDeckThreads()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	kern_return_t Assistant::StopDeckThreads(Client client, UInt64 guid, CMIOObjectPropertyScope scope, CMIOObjectPropertyElement element)
	{
		try
		{
			// Grab the mutex for the Assistant's state
			CAMutex::Locker locker(mStateMutex);
			
			// Locate the device with the specified GUID
			Device& device = GetDeviceByGUID(guid);
			
			// Tell the device to start its stream
			device.StopDeckThreads(client, scope, element);
			
		}
		catch (CAException& exception)
		{
			return exception.GetError();
		}
		
		return KERN_SUCCESS;
	}
	
	
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// DeckPlay()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	kern_return_t Assistant::DeckPlay(Client client, UInt64 guid, CMIOObjectPropertyScope scope, CMIOObjectPropertyElement element)
	{
		try
		{
			// Grab the mutex for the Assistant's state
			CAMutex::Locker locker(mStateMutex);
			
			// Locate the device with the specified GUID
			Device& device = GetDeviceByGUID(guid);
			
			// Tell the device to start its stream
			device.DeckPlay(client, scope, element);
			
		}
		catch (CAException& exception)
		{
			return exception.GetError();
		}
		
		return KERN_SUCCESS;
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// DeckStop()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	kern_return_t Assistant::DeckStop(Client client, UInt64 guid, CMIOObjectPropertyScope scope, CMIOObjectPropertyElement element)
	{
		try
		{
			// Grab the mutex for the Assistant's state
			CAMutex::Locker locker(mStateMutex);
			
			// Locate the device with the specified GUID
			Device& device = GetDeviceByGUID(guid);
			
			// Tell the device to start its stream
			device.DeckStop(client, scope, element);
			
		}
		catch (CAException& exception)
		{
			return exception.GetError();
		}
		
		return KERN_SUCCESS;
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// DeckJog()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	kern_return_t Assistant::DeckJog(Client client, UInt64 guid, CMIOObjectPropertyScope scope, CMIOObjectPropertyElement element, SInt32 speed)
	{
		try
		{
			// Grab the mutex for the Assistant's state
			CAMutex::Locker locker(mStateMutex);
			
			// Locate the device with the specified GUID
			Device& device = GetDeviceByGUID(guid);
			
			// Tell the device to start its stream
			device.DeckJog(client, scope, element, speed);
			
		}
		catch (CAException& exception)
		{
			return exception.GetError();
		}
		
		return KERN_SUCCESS;
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// DeckJog()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	kern_return_t Assistant::DeckCueTo(Client client, UInt64 guid, CMIOObjectPropertyScope scope, CMIOObjectPropertyElement element, Float64 requestedTimecode, UInt32 playOnCue)
	{
		try
		{
			// Grab the mutex for the Assistant's state
			CAMutex::Locker locker(mStateMutex);
			
			// Locate the device with the specified GUID
			Device& device = GetDeviceByGUID(guid);
			
			// Tell the device to start its stream
			device.DeckCueTo(client, scope, element, requestedTimecode, playOnCue);
			
		}
		catch (CAException& exception)
		{
			return exception.GetError();
		}
		
		return KERN_SUCCESS;
	}
	
	#pragma mark -
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// GetDeviceByGUID()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	Device& Assistant::GetDeviceByGUID(UInt64 guid)
	{
		// Locate the device with the specified GUID
		Devices::const_iterator i = std::find_if(mDevices.begin(), mDevices.end(), Device::GUIDEqual(guid));
		ThrowIf(i == mDevices.end(), CAException(kIOReturnNoDevice), "Assistant::GetDeviceByGUID: no match for specified GUID");
		
		return (**i);
	}

	#pragma mark -
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// SendDeviceStatesChangedMessage()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void Assistant::SendDeviceStatesChangedMessage(mach_port_t destination)
	{
		// Statically initialize the invariant portions of the message (using 'safe' values for the variable portion).
		// (Note:  for messages with 'move' dispostions such as this one, the kernel will not alter the invariant portion of the message, so this is a safe optimization.)
		static DeviceStatesChangedMessage message =
		{
			{
				MACH_MSGH_BITS_REMOTE(MACH_MSG_TYPE_MOVE_SEND_ONCE),						// header.msgh_bits
				sizeof(DeviceStatesChangedMessage),											// header.msgh_size
				MACH_PORT_NULL,																// header.msgh_remote_port
				MACH_PORT_NULL,																// header.msgh_local_port
				0,																			// header.msgh_reserved
				kDeviceStatesChanged														// header.msgh_id
			}
		};
		
		// Update the variable portion of the message
		message.mHeader.msgh_remote_port = destination;

		// Send the message
		mach_msg_return_t err = mach_msg(&message.mHeader, MACH_SEND_MSG, message.mHeader.msgh_size, 0, MACH_PORT_NULL, 0, MACH_PORT_NULL);
		if (MACH_MSG_SUCCESS != err)
		{
			// Delivery of the message failed, most likely because the client terminated, so simply destroy the message and continue
			mach_msg_destroy(&message.mHeader);
			DebugMessage("SampleAssistant::SendDeviceStatesChangedMessage() - Error sending notification to port %d - 0x%08X", destination, err);
		}
	}

}}}}
