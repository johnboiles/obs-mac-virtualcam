/*
	    File: CMIO_DPA_Sample_Server_Assistant.h
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

#if !defined(__CMIO_DPA_Sample_Server_Assistant_h__)
#define __CMIO_DPA_Sample_Server_Assistant_h__

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//	Includes
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

// Internal Includes
#include "CMIO_DPA_Sample_Server_Common.h"
#include "CMIO_DPA_Sample_Shared.h"

// Public Utility Includes
#include "CMIO_IOKA_Object.h"
#include "CMIO_PTA_NotificationPortThread.h"

// CA Public Utility Includes
#include "CAMutex.h"
#include "CACFDictionary.h"
#include "CACFObject.h"
#include "CACFString.h"

// Standard Library Includes
#include <map>
#include <set>
#include <vector>

namespace CMIO { namespace DPA { namespace Sample { namespace Server
{
	class Device;
	
	class Assistant
	{
	// Construction/Destruction
	public:
		static Assistant*			Instance();

	private:
									Assistant();
									~Assistant();

		static Assistant*			mInstance;
		CAMutex						mStateMutex;				// Controls access to SampleAssistant's state
		CACFBundle					mPlugInBundle;				// The Sample.plugIn bundle used for locating resources for device names

	// Basic Operations
	public:
		mach_port_t					GetPortSet() { return mPortSet; }

	private:
		mach_port_t					mPortSet;

	// Client Connection / Disconnection
	public:
		kern_return_t				Connect(pid_t client, mach_port_t* clientSendPort);
		kern_return_t				Disconnect(Client client);
		void						ClientDied(mach_port_t clientPort);

	// Client <-> Device Interaction
	public:
		kern_return_t				GetDeviceStates(Client client, mach_port_t messagePort, DeviceState** deviceStates, mach_msg_type_number_t* length);

		kern_return_t				GetProperties(Client client, UInt64 guid, mach_port_t messagePort, UInt64 time, const PropertyAddress& matchAddress, PropertyAddress** addresses, mach_msg_type_number_t* length);
		kern_return_t				GetPropertyState(Client client, UInt64 guid, const PropertyAddress& address, UInt8* qualifier, mach_msg_type_number_t qualifierLength, UInt8** data, mach_msg_type_number_t* length);
		kern_return_t				SetPropertyState(Client client, UInt64 guid, bool sendChangedNotifications, const PropertyAddress& address, UInt8* qualifier, mach_msg_type_number_t qualifierLength, Byte* data, mach_msg_type_number_t length);

		kern_return_t				GetControlList(Client client, UInt64 guid, UInt8** data, mach_msg_type_number_t* length);
		kern_return_t				GetControls(Client client, UInt64 guid, mach_port_t messagePort, UInt64 time, ControlChanges** controlChanges, mach_msg_type_number_t* length);
		kern_return_t				SetControl(Client client, UInt64 guid, UInt32 controlID, UInt32 value, UInt32* newValue);

		kern_return_t				StartStream(Client client, UInt64 guid, mach_port_t messagePort, CMIOObjectPropertyScope scope, CMIOObjectPropertyElement element);
		kern_return_t				StopStream(Client client, UInt64 guid, CMIOObjectPropertyScope scope, CMIOObjectPropertyElement element);

		kern_return_t				ProcessRS422Command(UInt64 guid, ByteArray512 command, mach_msg_type_number_t commandLength, UInt32 responseLength, UInt32 *responseUsed, UInt8** response, mach_msg_type_number_t *responseCount);

		kern_return_t				StartDeckThreads(Client client, UInt64 guid, CMIOObjectPropertyScope scope, CMIOObjectPropertyElement element);
		kern_return_t				StopDeckThreads(Client client, UInt64 guid, CMIOObjectPropertyScope scope, CMIOObjectPropertyElement element);
		kern_return_t				DeckPlay(Client client, UInt64 guid, CMIOObjectPropertyScope scope, CMIOObjectPropertyElement element);
		kern_return_t				DeckStop(Client client, UInt64 guid, CMIOObjectPropertyScope scope, CMIOObjectPropertyElement element);
		kern_return_t				DeckJog(Client client, UInt64 guid, CMIOObjectPropertyScope scope, CMIOObjectPropertyElement element, SInt32 speed);
		kern_return_t				DeckCueTo(Client client, UInt64 guid, CMIOObjectPropertyScope scope, CMIOObjectPropertyElement element, Float64 requestedTimecode, Boolean playOnCue);

	private:
		Device&						GetDeviceByGUID(UInt64 guid);

	// Messages
	private:
		void						SendDeviceStatesChangedMessage(mach_port_t destination);

	// Device Mangagement
	public:
		static void					DeviceArrived(Assistant& assistant, io_iterator_t iterator);
		void						DeviceRemoved(Device& device);

	private:
		void						InitializeDeviceAddedNotification();
		void						CreateDeviceAddedNotification(CFMutableDictionaryRef matchingDictionary);

		typedef std::vector<IOKA::Object>			NotificationIterators;
		typedef std::set<Device*>					Devices;				 

		PTA::NotificationPortThread	mNotificationPortThread;				// Thread for getting IOKit notifications on
		NotificationIterators		mDeviceAddedIterators;					// A "kIOMatchedNotification" notifcation iterator for each "matching dictionary"
		Devices						mDevices;

	// Client Management
	private:
		struct StreamSpecifier
		{
			StreamSpecifier(Device& device, CMIOObjectPropertyScope scope, CMIOObjectPropertyElement element) : mDevice(device), mScope(scope), mElement(element) {}

			Device&						mDevice;
			CMIOObjectPropertyScope		mScope;
			CMIOObjectPropertyElement	mElement;

			struct Less : public std::binary_function<StreamSpecifier, StreamSpecifier, bool>
			{
				bool	operator()(const StreamSpecifier& specifier1, const StreamSpecifier& specifier2) const {  bool answer = false; if (&specifier1.mDevice != &specifier2.mDevice) { answer = &specifier1.mDevice < &specifier2.mDevice; } else if (specifier1.mScope != specifier2.mScope) { answer = specifier1.mScope < specifier2.mScope; } else { answer = specifier1.mElement < specifier2.mElement; } return answer; }
			};

			class DeviceEqual
			{
			public:
						DeviceEqual(Device& device) : mDevice(device) {};
				bool	operator()(const StreamSpecifier& streamSpecifier) const { return &streamSpecifier.mDevice == &mDevice; }
			
			private:
						Device& mDevice;
			};
			
			
		};
		typedef std::set<StreamSpecifier, StreamSpecifier::Less> StreamSpecifiers;				 
		
		struct ClientInfo
		{
			ClientInfo(pid_t pid) : mPID(pid) {}
			pid_t					mPID;									// The process ID of the client
			StreamSpecifiers		mStreamSpecifiers;						// Set of streams client is streaming
		};
		typedef std::map<Client, ClientInfo*>		ClientInfoMap;			// Map Client to ClientInfo		
		typedef std::multimap<Client, mach_port_t>  ClientNotifiers;		// Clients Mach ports to message in response to various events

		ClientInfoMap				mClientInfoMap;
		ClientNotifiers				mDeviceStateNotifiers;					// Clients to notify when devices arrive / disappear
	};
}}}}
#endif
