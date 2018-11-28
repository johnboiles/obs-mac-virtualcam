/*
	    File: CMIO_DP_Sample_Device.h
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

#if !defined(__CMIO_DP_Sample_Device_h__)
#define __CMIO_DP_Sample_Device_h__

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Includes
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

// Super Class Includes
#include "CMIO_DP_Device.h"

// Internal Includes
#include "CMIO_DP_Property_SMPTETimeCallback.h"

// Public Utility Includes
#include "CMIO_IOKA_NotificationPort.h"

// Public Utility Includes
#include "CMIO_CMA_FormatDescription.h"
#include "CMIO_IOKA_Object.h"

// CA Public Utility Includes
#include "CACFDictionary.h"
#include "CACFMachPort.h"
#include "CACFString.h"

// System Includes
#include <IOKit/audio/IOAudioTypes.h>
#include <IOKit/video/IOVideoTypes.h>

namespace CMIO
{
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// Types in the CMIO namespace
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	class Buffer;
}

namespace CMIO { namespace DP { namespace Property
{
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// Types in the CMIO::DP::Property namespace
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	class ClientSyncDiscontinuity;
	class HogMode;
}}}

namespace CMIO { namespace DP { namespace Sample
{
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// Types in the CMIO::DP::Sample namespace
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	class PlugIn;
}}}

namespace CMIO { namespace DP { namespace Sample
{
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// Device
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	class Device : public DP::Device
	{
	// Construction/Destruction
	public:
												Device(PlugIn& plugIn, CMIODeviceID deviceID, mach_port_t assistantPort, UInt64 guid, const io_string_t registryPath);
		virtual									~Device();
	
		virtual void							Initialize();
		virtual void							Teardown();
		virtual void							Finalize();

	protected:
		UInt64									mDeviceGUID;	// The 'pseduo-GUID' of the device returned from the Assistant
		IOKA::Object							mRegistryEntry;	// The IOKit registry entry for the device

	// Attributes
	public:
		PlugIn&									GetPlugIn()	{ return reinterpret_cast<PlugIn&>(DP::Device::GetPlugIn()); }
		virtual CFStringRef						CopyDeviceName() const ;
		virtual CFStringRef						CopyDeviceManufacturerName() const { return mDeviceManufacturerName.CopyCFString(); }
		virtual CFStringRef						CopyDeviceUID() const { return mDeviceUID.CopyCFString(); }
		virtual CFStringRef						CopyModelUID() const;
		virtual bool							HogModeIsOwnedBySelfOrIsFree() const;
		virtual void							SetDeviceMaster(pid_t masterPID);
		virtual void							SetExcludeNonDALAccess(bool excludeNonDALAccess);
		virtual void							SetForceDiscontinuity(Boolean forceDiscontinuity);
		virtual UInt32							GetTransportType() const { return kIOAudioDeviceTransportTypePCI; }
		virtual bool							IsDeviceRunningSomewhere() const { return mDeviceIsRunningSomewhere; }
		UInt64									GetDeviceGUID() const { return mDeviceGUID; }	
		mach_port_t								GetAssistantPort() const { return mAssistantPort; }
		
	protected:
		mach_port_t								mAssistantPort;					// Mach port used to send messages to IIDCVideoAssistant
        CACFString								mDeviceUID;		
		CACFString								mDeviceName;
		CACFString								mDeviceManufacturerName;
		
	// Property Access
	public:
		virtual bool							HasProperty(const CMIOObjectPropertyAddress& address) const;
		virtual bool							IsPropertySettable(const CMIOObjectPropertyAddress& address) const;
		virtual UInt32							GetPropertyDataSize(const CMIOObjectPropertyAddress& address, UInt32 qualifierDataSize, const void* qualifierData) const;
		virtual void							GetPropertyData(const CMIOObjectPropertyAddress& address, UInt32 qualifierDataSize, const void* qualifierData, UInt32 dataSize, UInt32& dataUsed, void* data) const;
		virtual void							SetPropertyData(const CMIOObjectPropertyAddress& address, UInt32 qualifierDataSize, const void* qualifierData, UInt32 dataSize, const void* data);

	protected:
		virtual void							PropertyListenerAdded(const CMIOObjectPropertyAddress& address);
		void									CreateProperties();
		void									ReleaseProperties();

		UInt64									mPropertyCacheTime;				// When the values of the properties were last cached
		DP::Property::HogMode*					mHogMode;
		DP::Property::ClientSyncDiscontinuity*	mClientSyncDiscontinuity;
		DP::Property::SMPTETimeCallback*		mSMPTETimeCallback;
		bool									mDeviceIsRunningSomewhere;

	// Controls
	public:
		DP::Control*							GetControlByControlID(UInt32 controlID) const;
		virtual void							SetControlValue(UInt32 controlID, UInt32 value, UInt32* newValue);
		virtual CFDictionaryRef					CopyControlDictionaryByControlID(UInt32 controlID) const;

	protected:
		void									CreateRegistryControls();
		void									CreatePluginControls();
		void									ReleaseControls(bool reportDeath = false);

		UInt64									mControlCacheTime;				// When the values of the controls was last cached

	// Property / Control Updates from Assistant
	protected:
		static void								Event(CFMachPortRef port, mach_msg_header_t* header, CFIndex size, Device& device);
		void									UpdateControlStates(bool sendChangeNotifications);
		void									UpdatePropertyStates();
		CACFMachPort&							GetEventPort()  { return mEventPort; }

		CACFMachPort							mEventPort;						// Port from which control/property event messages are received from the Assistant

	// Command Management
	protected:
		virtual bool							IsSafeToExecuteCommand(DP::Command* command);
		virtual bool							StartCommandExecution(void** savedCommandState);
		virtual void							FinishCommandExecution(void* savedCommandState);
	
	// RS422 Commands
	public:
		virtual void							ProcessRS422Command(CMIODeviceRS422Command* ioRS422Command);

	// Unplugging Management
	public:
		virtual void							Unplug();
		
	// Stream Management
	public:
		virtual void							StartStream(CMIOStreamID streamID);
		virtual void							StopStream(CMIOStreamID streamID);
		virtual void							StopAllStreams();
		virtual void							SuspendAllStreams();
		virtual void							StreamDirectionChanged(CMIOObjectPropertyScope newScope);
		
	protected:
		void									CreateStreams(CMIOObjectPropertyScope scope);
		void									ReleaseStreams(bool reportDeath = false);
		void									HandleConfigChangeRequestNotification(const IOVideoDeviceNotification& notification);
	
	// Accessors / Setters for Properties
	public:
		OSStatus								GetSMPTETime(UInt64* frameCount, Boolean* isDropFrame, UInt32* tolerance) { if (mSMPTETimeCallback) return mSMPTETimeCallback->DoCallback(frameCount, isDropFrame, tolerance); else return kCMIOHardwareIllegalOperationError; }
	};
}}}
#endif
