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

// MIG Server Interface
#include "CMIODPASampleServer.h"

// Public Utility Includes
#include "CMIODebugMacros.h"

// CA Public Utility Includes
#include "CAAutoDisposer.h"
#include "CACFNumber.h"
#include "CACFObject.h"

// System Includes
#include <CoreAudio/HostTime.h>
#include <mach/mach.h>
#include <mach/notify.h>
#include <mach-o/dyld.h>
#include <servers/bootstrap.h>
#include <sys/param.h>

// Standard Library Includes
#include <algorithm>

namespace
{
	// As a convenience, use the CMIO::DPA::Sample namespace.  This will allow convienient access in the anonymous namespace as well as in the CMIODPASampleXXX() public interface to the server
	using namespace CMIO;
	using namespace CMIO::DPA::Sample;

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// MessagesAndNotifications()
	//	This handles messages from the client and the MACH_NOTIFY_NO_SENDERS notification on the client ports.
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	boolean_t MessagesAndNotifications(mach_msg_header_t* request, mach_msg_header_t* reply)
	{
		// Invoke the MIG created CMIODPASampleServer() to see if this is one of the client messages it handles
		boolean_t processed = CMIODPASampleServer(request, reply);
		
		// If CMIODPASampleServer() did not process the message see if it is a MACH_NOTIFY_NO_SENDERS notification
		if (not processed and MACH_NOTIFY_NO_SENDERS == request->msgh_id)
		{
			Server::Assistant::Instance()->ClientDied(request->msgh_local_port);
			processed = true;
		}
		
		return processed;
	}

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
	#pragma mark Static Globals
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// Static Globals
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	Assistant* Assistant::mInstance = 0;

	#pragma mark -
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// Assistant()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	Assistant::Assistant() :
		mStateMutex("SampleAssistant state mutex"),
		mPlugInBundle(CopyPlugInBundle()),
		mPortSet(MACH_PORT_NULL),
		mNotificationPortThread(true),
		mDeviceAddedIterators(),
		mDevices(),
		mClientInfoMap(),
		mDeviceStateNotifiers()
	{
		// Wait for the notification port thread to be running prior to continuing
		while (PTA::NotificationPortThread::kStarting == mNotificationPortThread.GetState())
			pthread_yield_np();

		// Make sure the notification port is not invalid
		ThrowIf(PTA::NotificationPortThread::kInvalid == mNotificationPortThread.GetState(), -1, "Notification thread invalid");
		
		// Create a port set to hold the service port, and each client's port
		kern_return_t err = mach_port_allocate(mach_task_self(), MACH_PORT_RIGHT_PORT_SET, &mPortSet);
		ThrowIfKernelError(err, CAException(err), "Unable to create port set");
		
		// Initialize the notification for hot plugging of devices.  In addition to handling future hot plug events, this will also set up the devices that are currently plugged in
		InitializeDeviceAddedNotification();
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
	// Instance()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	Assistant* Assistant::Instance()
	{
		if (0 == mInstance)
			mInstance = new Assistant(); 
		
		return mInstance; 
	}

	#pragma mark -
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// Connect()
	//	This is the first call that clients invoke to establish a connection with the Assistant.  The Assistant will create a unique port for the client, add it to the portset it services,
	//	and a return a send right to the client (via the MIG reply).  The Assistant will request a "no-sender" notification on that newly created port, thus allowing it to get a notification
	//	in the event the client is terminated.
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	kern_return_t Assistant::Connect(pid_t clientPID, mach_port_t* client)
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
			
		// There are no more clients, so exit the SampleAssistant server process.
		// The "com.apple.cmio.DPA.Sample" service will still be registered with the bootstrap_look_up mechanism, so the next time a client needs to establish a connection it will
		// cause the SampleAssistant server process to be restarted.
		exit(0);
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
			*length = mDevices.size();

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
				(**i).GetRegistryPath((*deviceStates)[index].mRegistryPath);
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
	kern_return_t Assistant::GetProperties(Client client, UInt64 guid, mach_port_t messagePort, UInt64 time, const PropertyAddress& matchAddress, PropertyAddress** addresses, mach_msg_type_number_t* length)
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
	kern_return_t Assistant::GetPropertyState(Client client, UInt64 guid, const PropertyAddress& address, UInt8* qualifier, mach_msg_type_number_t qualifierLength, UInt8** data, mach_msg_type_number_t* length)
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
	kern_return_t Assistant::SetPropertyState(Client client, UInt64 guid, bool sendChangedNotifications, const PropertyAddress& address, UInt8* qualifier, mach_msg_type_number_t qualifierLength, Byte* data, mach_msg_type_number_t length)
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
	kern_return_t Assistant::ProcessRS422Command(UInt64 guid, ByteArray512 command, mach_msg_type_number_t commandLength, UInt32 responseLength, UInt32 *responseUsed, UInt8** response, mach_msg_type_number_t *responseCount)
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
	kern_return_t Assistant::DeckCueTo(Client client, UInt64 guid, CMIOObjectPropertyScope scope, CMIOObjectPropertyElement element, Float64 requestedTimecode, Boolean	playOnCue)
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


	#pragma mark -
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// DeviceAdded()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void Assistant::DeviceArrived(Assistant& assistant, io_iterator_t iterator)
	{
		#if 0
			// Wait forever until the Debugger can attach to the Assistant process
			bool waiting = true;
			while (waiting)
			{
				sleep(1);
			}
		#endif
		
		// Catch all exceptions since this is invoked via a call back and the exception cannot leave this routine 
		try
		{
			// Grab the mutex for the Assistant's state
			CAMutex::Locker locker(assistant.mStateMutex);
			
			// Get the current device count
			UInt32 deviceCount = assistant.mDevices.size();
			
			while (true)
			{
				IOKA::Object registryEntry(IOIteratorNext(iterator));
				if (not registryEntry.IsValid())
					break;
				
				// Make sure the registry entry conforms to an IOVideoDevice
				if (not registryEntry.ConformsTo("IOVideoDevice"))
					continue;

				Device* device = NULL;
				
				// Catch all exceptions so the iterator can be advanced to the next device in the event of any problems
				try
				{
					// Create the new device
					device = new Device(registryEntry, assistant.mNotificationPortThread);
					
					// Add it to the set of discovered devices whose capabilities are known
					assistant.mDevices.insert(device);
				}
				catch (CAException& exception)
				{
					if (NULL != device)
						delete device;
				}
			}
			
			// If any devices were successfully added, notify interested clients that a state change has taken place so they can call UpdateDeviceStates() at their convenience
			if (deviceCount != assistant.mDevices.size())
			{
				// Send out the devices state changed message
				for (ClientNotifiers::iterator i = assistant.mDeviceStateNotifiers.begin() ; i != assistant.mDeviceStateNotifiers.end() ; ++i)
					assistant.SendDeviceStatesChangedMessage((*i).second);

				// All the 'send-once' rights are now used up, so erase everything in the multimap 
				assistant.mDeviceStateNotifiers.erase(assistant.mDeviceStateNotifiers.begin(), assistant.mDeviceStateNotifiers.end());
			}
		}
		catch (...)
		{
		}
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// DeviceRemoved()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void Assistant::DeviceRemoved(Device& device)
	{
		// Catch all exceptions since exceptions cannot leave this routine 
		try
		{
			// Grab the mutex for the Assistant's state
			CAMutex::Locker locker(mStateMutex);
			
			// Iterate over the ClientInfoMap and remove this device from each clients' set of streams it was watching
			for (ClientInfoMap::iterator i = mClientInfoMap.begin() ; i != mClientInfoMap.end() ; ++i)
			{
				while (true)
				{
					// See if this client was watching a stream from this device
					StreamSpecifiers::iterator ii = std::find_if((*i).second->mStreamSpecifiers.begin(), (*i).second->mStreamSpecifiers.end(), StreamSpecifier::DeviceEqual(device));
					if (ii == (*i).second->mStreamSpecifiers.end())
						break;
					
					// This client had a stream that corresponded to this device, so erase it
					(void) (*i).second->mStreamSpecifiers.erase(ii);
				}
			}

			// Erase the device from the set of devices which the Assistant knows about
			(void) mDevices.erase(&device);
			
			// Notify interested clients that a state change has taken place so they can call UpdateDeviceStates() at their convenience
			for (ClientNotifiers::iterator i = mDeviceStateNotifiers.begin() ; i != mDeviceStateNotifiers.end() ; ++i)
				SendDeviceStatesChangedMessage((*i).second);

			// All the 'send-once' rights are now used up, so erase everything multimap 
			mDeviceStateNotifiers.erase(mDeviceStateNotifiers.begin(), mDeviceStateNotifiers.end());
		}
		catch (...)
		{
		}
	}

	#pragma mark -
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// InitializeDeviceAddedNotification()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void Assistant::InitializeDeviceAddedNotification()
	{
		// Create a matching dictionary to specify that only Sample devices are of interest
		CACFDictionary matchingDictionary(IOServiceMatching("IOVideoSampleDevice"), true);
		ThrowIf(not matchingDictionary.IsValid(), -1, "Assistant::InitializeDeviceAddedNotification: unable to get service matching dictionary");

		// Create the notification
		CreateDeviceAddedNotification(matchingDictionary.GetCFMutableDictionary());
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// CreateDeviceAddedNotification()
	//	Request device added notfication for the device specified in the provided matching dictionary.
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void Assistant::CreateDeviceAddedNotification(CFMutableDictionaryRef matchingDictionary)
	{
		// IOServiceAddMatchingNotification 'eats' a matching dictionary, so up the retention count
		CFRetain(matchingDictionary);

		IOKA::Object iterator;
		IOReturn ioReturn = IOServiceAddMatchingNotification(mNotificationPortThread.GetNotificationPort(), kIOMatchedNotification, matchingDictionary, reinterpret_cast<IOServiceMatchingCallback>(DeviceArrived), this, iterator.GetAddress());
		ThrowIfError(ioReturn, CAException(ioReturn), "Assistant::CreateDeviceAddedNotification: IOServiceAddMatchingNotification() failed");
			
		mDeviceAddedIterators.push_back(iterator);
		
		// The iterator is returned unarmed, but full of the devices which matched the dictionary.  So manually invoke the DeviceArrived() routine to add all the devices already present and
		// to arm the iterator for subsequent additions.
		DeviceArrived(*this, iterator);
	}
}}}}

// As a convenience, use the CMIO::DPA::Sample::Server namespace
using namespace CMIO::DPA::Sample::Server;

#pragma mark -
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// main()
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
int main()
{
	// Don't allow any exceptions to escape
	try
	{
		// Check in with the bootstrap port under the agreed upon name to get the servicePort with receive rights
		mach_port_t servicePort;
		name_t serviceName = "com.apple.cmio.DPA.Sample";
		kern_return_t err = bootstrap_check_in(bootstrap_port, serviceName, &servicePort); 
		if (BOOTSTRAP_SUCCESS != err)
		{
			DebugMessage("bootstrap_check_in() failed: 0x%x", err);
			exit(43);
		}
	
		#if 0
			// Wait forever until the Debugger can attach to the Assistant process
			bool waiting = true;
			while (waiting)
			{
				sleep(1);
			}
		#endif

		// Add the service port to the Assistant's port set
		mach_port_t portSet = Assistant::Instance()->GetPortSet();
		err = mach_port_move_member(mach_task_self(), servicePort, portSet);
		if (KERN_SUCCESS != err)
		{
			DebugMessage("Unable to add service port to port set: 0x%x", err);
			exit(2);
		}

		// Service incoming messages from the clients and notifications which were signed up for 
		while (true)
		{
			(void) mach_msg_server(MessagesAndNotifications, 8192, portSet, MACH_MSG_OPTION_NONE);
		}
	}
	catch (const CAException& exception)
	{
		exit(exception.GetError());
	}
	catch (...)
	{
		DebugMessage("Terminated by an an unknown exception");
		exit(44);
	}
		
	exit(0);
	return 0;
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// CMIODPASampleConnect()
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
kern_return_t CMIODPASampleConnect(mach_port_t servicePort, pid_t client, mach_port_t* clientSendPort)
{
	return Assistant::Instance()->Connect(client, clientSendPort);
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// CMIODPASampleDisconnect()
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
kern_return_t CMIODPASampleDisconnect(mach_port_t client)
{
	return Assistant::Instance()->Disconnect(client);
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// CMIODPASampleGetDeviceStates()
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
kern_return_t CMIODPASampleGetDeviceStates(mach_port_t client, mach_port_t messagePort, DeviceState** deviceStates, mach_msg_type_number_t* length)
{
	return Assistant::Instance()->GetDeviceStates(client, messagePort, deviceStates, length);
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// CMIODPASampleGetProperties()
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
kern_return_t CMIODPASampleGetProperties(mach_port_t client, UInt64 guid, mach_port_t messagePort, UInt64 time, CMIOObjectPropertyAddress matchAddress, PropertyAddress** addresses, mach_msg_type_number_t* length)
{
	return Assistant::Instance()->GetProperties(client, guid, messagePort, time, matchAddress, addresses, length);
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// CMIODPASampleGetPropertyState()
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
kern_return_t CMIODPASampleGetPropertyState(mach_port_t client, UInt64 guid, CMIOObjectPropertyAddress address, UInt8* qualifier, mach_msg_type_number_t qualifierLength, UInt8** data, mach_msg_type_number_t* length)
{
	return Assistant::Instance()->GetPropertyState(client, guid, address, qualifier, qualifierLength, data, length);
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// CMIODPASampleSetPropertyState()
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
kern_return_t CMIODPASampleSetPropertyState(mach_port_t client, UInt64 guid, UInt32 sendChangedNotifications, CMIOObjectPropertyAddress address, UInt8* qualifier, mach_msg_type_number_t qualifierLength, Byte* data, mach_msg_type_number_t length)
{
	return Assistant::Instance()->SetPropertyState(client, guid, sendChangedNotifications, address, qualifier, qualifierLength, data, length);
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// CMIODPASampleGetControls()
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
kern_return_t CMIODPASampleGetControls(mach_port_t client, UInt64 guid, mach_port_t messagePort, UInt64 time, ControlChanges** controlChanges, mach_msg_type_number_t* length)
{
	return Assistant::Instance()->GetControls(client, guid, messagePort, time, controlChanges, length);
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// CMIODPASampleSetControl()
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
kern_return_t CMIODPASampleSetControl(mach_port_t client, UInt64 guid, UInt32 controlID, UInt32 value, UInt32* newValue)
{
	return Assistant::Instance()->SetControl(client, guid, controlID, value, newValue);
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// CMIODPASampleProcessRS422Command()
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
kern_return_t CMIODPASampleProcessRS422Command(mach_port_t client, UInt64 guid, ByteArray512 command, mach_msg_type_number_t commandLength, UInt32 responseLength, UInt32 *responseUsed, UInt8** response, mach_msg_type_number_t *responseCount)
{
	return Assistant::Instance()->ProcessRS422Command(guid, command, commandLength, responseLength, responseUsed, response, responseCount);
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// CMIODPASampleStartStream()
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
kern_return_t CMIODPASampleStartStream(mach_port_t client, UInt64 guid, mach_port_t messagePort, CMIOObjectPropertyScope scope, CMIOObjectPropertyElement element)
{
	return Assistant::Instance()->StartStream(client, guid, messagePort, scope, element);
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// CMIODPASampleStopStream()
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
kern_return_t CMIODPASampleStopStream(mach_port_t client, UInt64 guid, CMIOObjectPropertyScope scope, CMIOObjectPropertyElement element)
{
	return Assistant::Instance()->StopStream(client, guid, scope, element);
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// CMIODPASampleGetControlList()
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
kern_return_t CMIODPASampleGetControlList(mach_port_t client, UInt64 guid, UInt8** data, mach_msg_type_number_t* length)
{
	return Assistant::Instance()->GetControlList(client, guid, data, length);
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// CMIODPASampleStartDeckThreads()
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
kern_return_t CMIODPASampleStartDeckThreads(mach_port_t client, UInt64 guid, CMIOObjectPropertyScope scope, CMIOObjectPropertyElement element)
{
	return Assistant::Instance()->StartDeckThreads(client, guid, scope, element);
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// CMIODPASampleStopDeckThreads()
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
kern_return_t CMIODPASampleStopDeckThreads(mach_port_t client, UInt64 guid, CMIOObjectPropertyScope scope, CMIOObjectPropertyElement element)
{
	return Assistant::Instance()->StopDeckThreads(client, guid, scope, element);
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// CMIODPASampleDeckPlay()
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
kern_return_t CMIODPASampleDeckPlay(mach_port_t client, UInt64 guid, CMIOObjectPropertyScope scope, CMIOObjectPropertyElement element)
{
	return Assistant::Instance()->DeckPlay(client, guid, scope, element);
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// CMIODPASampleDeckStop()
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
kern_return_t CMIODPASampleDeckStop(mach_port_t client, UInt64 guid, CMIOObjectPropertyScope scope, CMIOObjectPropertyElement element)
{
	return Assistant::Instance()->DeckStop(client, guid, scope, element);
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// CMIODPASampleDeckPlay()
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
kern_return_t CMIODPASampleDeckJog(mach_port_t client, UInt64 guid, CMIOObjectPropertyScope scope, CMIOObjectPropertyElement element, SInt32 speed)
{
	return Assistant::Instance()->DeckJog(client, guid, scope, element, speed);
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// CMIODPASampleDeckPlay()
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
kern_return_t CMIODPASampleDeckCueTo(mach_port_t client, UInt64 guid, CMIOObjectPropertyScope scope, CMIOObjectPropertyElement element, Float64 requestedTimecode, UInt32 playOnCue)
{
	return Assistant::Instance()->DeckCueTo(client, guid, scope, element, requestedTimecode, playOnCue);
}

