/*
	    File: CMIO_DPA_Sample_Server_Device.h
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

#if !defined(__CMIO_DPA_Sample_Server_Device_h__)
#define __CMIO_DPA_Sample_Server_Device_h__

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//	Includes
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

// Internal Includes
#include "CMIO_DPA_Sample_Server_Common.h"
#include "CMIO_DPA_Sample_Server_Stream.h"
#include "CMIO_DPA_Sample_Shared.h"

// Public Utility Includes
#include "CMIO_IOKA_PowerNotificationPort.h"
#include "CMIO_IOKA_Object.h"
#include "CMIO_IOVA_Assistance.h"

// CA Public Utility Includes
#include "CACFArray.h"
#include "CACFDictionary.h"
#include "CAGuard.h"

// System Includes
#include <CoreMedia/CMTime.h>
#include <CoreMediaIO/CMIOSampleBuffer.h>
#include <IOKit/video/IOVideoTypes.h>

// Standard Library Includes
#include <map>

namespace CMIO { namespace PTA
{
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// Types in the CMIO::PTA namespace
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	class NotificationPortThread;
}}

namespace CMIO { namespace DPA { namespace Sample { namespace Server
{
	class Device
	{
	public:
		//-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
		// Device::GUIDEqual
		//	A unary predicate object which reports if the Device's GUID equals the specified GUID.
		//-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
		class GUIDEqual
		{
		public:
					GUIDEqual(UInt64 guid) : mGUID(guid) {};
			bool	operator()(Device* device) const { return device->GetDeviceGUID() == mGUID; }
		
		private:
					UInt64 mGUID;
		};

	public:
		//-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
		// Device::StreamIsActive
		//	A unary predicate object which reports true if the FormatPair.second.first equals the specified CMFormatDescriptionRef.
		//-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
		class StreamIsActive
		{
		public:
					StreamIsActive()  {};
			bool	operator()(const std::pair<UInt32, Stream*>& pair) const { return pair.second->Streaming(); }
		};

	#pragma mark -
	#pragma mark Device
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// Device
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// Construction/Destruction
	public:
										Device(IOKA::Object& registryEntry, PTA::NotificationPortThread& notificationPortThread);
		virtual							~Device();
	
	private:
		Device&							operator=(Device& that); // Don't allow copying
		
	protected:
		IOKA::Object					mRegistryEntry;				// The IOKit registry entry for the device
		io_string_t						mRegistryPath;				// The registry path for the device
        IOVA::PlugIn					mIOVAPlugIn;				// IOVA wrapper kIOVideoDeviceLibTypeID's IOCFPlugInInterface** 
        IOVA::Device					mIOVADevice;				// IOVA wrapper for IOVideoDeviceRef
		CAMutex							mStateMutex;				// Controls access to device's state

	// Attributes
	public:
		void							GetRegistryPath(io_string_t path) const { (void) strlcpy(path, mRegistryPath, sizeof(io_string_t)); }
		IOVA::Device&					GetIOVADevice() { return mIOVADevice; }
	
	// Device 'Guaranteed" Unique ID
	public:
		UInt64							GetDeviceGUID() const { return mDeviceGUID; }

 	protected:
		static UInt64					mGUIDGenerator;				// A static member that is incremented in for each Device constructed to provide mDeviceGUID
		UInt64							mDeviceGUID;				// A psuedo-GUID to act as the suffix for supplying the kCMIODevicePropertyDeviceUID

	// Client Management
	public:
		void							ClientDied(Client client);

	// Stream Management
	public:
		void							StartStream(Client client = MACH_PORT_NULL, mach_port_t messagePort = MACH_PORT_NULL, CMIOObjectPropertyScope scope = kCMIOObjectPropertyScopeWildcard, CMIOObjectPropertyElement element = kCMIOObjectPropertyElementWildcard, UInt32 initialDiscontinuityFlags = kCMIOSampleBufferNoDiscontinuities);
		void							StopStream(Client client, CMIOObjectPropertyScope scope, CMIOObjectPropertyElement element);
	
	protected:
		typedef std::map<UInt32, Stream*> StreamMap;
		typedef std::map<Client, ClientStream*> ClientStreamMap;
		
		void							DiscoverStreams();
		void							RemoveStreams();
		Stream&							GetStreamByScopeAndElement(CMIOObjectPropertyScope scope, CMIOObjectPropertyElement element);		
		Stream*							GetStreamByStreamID(UInt32 streamID);		
		bool							AnyStreamRunning() const;
		
		StreamMap						mInputStreams;
		StreamMap						mOutputStreams;
		
	// Deck
	public:
		void							StartDeckThreads(Client client, CMIOObjectPropertyScope scope, CMIOObjectPropertyElement element);
		void							StopDeckThreads(Client client, CMIOObjectPropertyScope scope, CMIOObjectPropertyElement element);
		void							DeckPlay(Client client, CMIOObjectPropertyScope scope, CMIOObjectPropertyElement element);
		void							DeckStop(Client client, CMIOObjectPropertyScope scope, CMIOObjectPropertyElement element);
		void							DeckJog(Client client, CMIOObjectPropertyScope scope, CMIOObjectPropertyElement element, SInt32 speed);
		void							DeckCueTo(Client client, CMIOObjectPropertyScope scope, CMIOObjectPropertyElement element, Float64 requestedTimecode, Boolean playOnCue);

	// Capabilities
	public:
		bool							CapabilitiesDiscovered() const { return mCapabilitiesDiscovered; }
		void							DiscoverCapabilities();

	protected:
		bool							mCapabilitiesDiscovered;

		typedef std::multimap<Client, mach_port_t> ClientNotifiers;					// Clients Mach ports to message in response to various events

	// Properties
	public:
		void							GetProperties(Client client, mach_port_t messagePort, UInt64 time, const PropertyAddress& matchAddress, PropertyAddress** addresses, mach_msg_type_number_t* length);
		void							GetPropertyState(const PropertyAddress& address, UInt8* qualifier, mach_msg_type_number_t qualifierLength, UInt8** data, mach_msg_type_number_t* length);
		void							SetPropertyState(Client client, bool sendChangedNotifications, const PropertyAddress& address, UInt8* qualifier, mach_msg_type_number_t qualifierLength, Byte* data, mach_msg_type_number_t length);
		void							SendPropertyStatesChangedMessage();

		void							GetControlList(UInt8** data, mach_msg_type_number_t* length);

	protected:
		virtual void					InitializeProperties();
		bool							ClientIsDeviceMasterOrIsFree(Client client) const { return (-1 == mDeviceMaster or client == mDeviceMasterClient); }
		
	private:
		Properties						mProperties;
		ClientNotifiers					mPropertyStateNotifiers;
		CAMutex							mPropertyStateNotifiersMutex;

		pid_t							mHogModeOwner;						// -1 unless being accessed through means other than the DAL (e.g., a digitizer)
		pid_t							mDeviceMaster;						// Process which owns the mastership of the device (or -1 if free)
		Client							mDeviceMasterClient;				// Client which owns the mastership of the device (or MACH_PORT_NULL if free)
		bool							mExcludeNonDALAccess;				// If 1, grab exclusive access to the device to prevent non-DAL access (e.g., a QuickTime Video Digitizer)
		Client							mExcludeNonDALAccessClient;			// Client which set mExcludeNonDALAccess to 1 (MACH_PORT_NULL if currently 0) 
		bool							mForceDiscontinuity;				// True if instructed to force a discontinuity (kCMIODevicePropertyClientSyncDiscontinuity)

	// Controls
	public:
		void							GetControls(Client client, mach_port_t messagePort, UInt64 time, ControlChanges** controlChanges, mach_msg_type_number_t* length);
		void							SetControl(Client client, UInt32 controlID, UInt32 value, UInt32* newValue);
		
		IOReturn						RS422Command(const UInt8 *command, UInt32 commandLength, UInt8 *response, UInt32 *responseLength);

	protected:
		void							SendControlStatesChangedMessage();
		virtual void					InitializeControls();

		CACFArray						mControlsList;
		Controls						mControls;
		ClientNotifiers					mControlStateNotifiers;				// Clients to notify when controls changed
		CAMutex							mControlStateNotifiersMutex;
		
	
	// Notifications
	public:
		PTA::NotificationPortThread&	GetNotificationThread()	{ return mNotificationThread; }
	protected:
        static void						IOVDeviceNotification(IOVideoDeviceRef deviceRef, Device& device, const IOVideoDeviceNotificationMessage& message);
		void							DeviceNotification(const IOVideoDeviceNotification& notification);
		void							StreamNotification(const IOVideoDeviceNotification& notification, Stream& stream);
		void							ControlNotification(const IOVideoDeviceNotification& notification, UInt64 shadowTime);
		static void						PowerNotification(Device& device, io_service_t unused, natural_t messageType, void* message);

		PTA::NotificationPortThread&	mNotificationThread;		// Assistant's thread for getting IOKit notifications
		IOKA::PowerNotificationPort		mPowerNotificationPort;		// For receiving power notifications

	// Misc
	protected:
		bool							mSleeping;					// True when put to sleep
		bool							mRestartStreamOnWake;		// True if the stream should be restarted upon waking from sleep
	
		void							Sleep();
		void							Wake();
	};
}}}}
#endif
