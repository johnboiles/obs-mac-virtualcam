/*
	    File: CMIO_DP_Device.h
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

#if !defined(__CMIO_DP_Device_h__)
#define __CMIO_DP_Device_h__

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Includes
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

// Super Class Includes
#include "CMIO_DP_Object.h"

// Internal Includes
#include "CMIO_DP_Property_DeviceMaster.h"

// CA PublicUtility Includes
#include "CAException.h"
#include "CAMutex.h"

// Standard Library Includes
#include <set>

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//	Types in the global namespace
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
class CAGuard;

namespace CMIO
{
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// Types in the CMIO namespace
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	class PropertyAddressList;
}

namespace CMIO { namespace DP
{
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// Types in the CMIO::DP namespace
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	class Command;
	class Control;
	class Stream;
}}

namespace CMIO { namespace DP
{
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// Device
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	class Device : public Object
	{
	// Construction/Destruction
	public:
											Device(CMIODeviceID deviceID, PlugIn& plugIn);
		virtual								~Device();
		
		virtual void						Initialize();
		virtual void						Teardown();

	// Basic Attributes
	public:
		virtual bool						IsAlive() const { return mIsAlive; }
		virtual void						SetIsAlive(bool isAlive) { mIsAlive = isAlive; }
		virtual CFStringRef					CopyDeviceName() const;
		virtual CFStringRef					CopyDeviceManufacturerName() const;
		virtual CFStringRef					CopyElementFullName(const CMIOObjectPropertyAddress& address) const;
		virtual CFStringRef					CopyElementCategoryName(const CMIOObjectPropertyAddress& address) const;
		virtual CFStringRef					CopyElementNumberName(const CMIOObjectPropertyAddress& address) const;
		virtual CFStringRef					CopyDeviceUID() const;
		virtual CFStringRef					CopyModelUID() const;
		virtual UInt32						GetTransportType() const;
		virtual bool						IsConstantRateClock() const;
		virtual bool						CanBeDefaultDevice(CMIOObjectPropertyScope scope, bool isSystem) const;
		virtual void						Lock();
		virtual void						Unlock();
		virtual bool						HogModeIsOwnedBySelf() const;
		virtual bool						HogModeIsOwnedBySelfOrIsFree() const;
		virtual void						HogModeStateChanged();
		virtual void						SetDeviceMaster(pid_t masterPID);
		virtual pid_t						GetDeviceMaster() const;
		virtual bool						DeviceMasterIsOwnedBySelf() const;
		virtual bool						DeviceMasterIsOwnedBySelfOrIsFree() const;
		virtual bool						GetExcludeNonDALAccess() const { return mExcludeNonDALAccess; }
		virtual void						SetExcludeNonDALAccess(bool excludeNonDALAccess) { mExcludeNonDALAccess = excludeNonDALAccess; }
		virtual void						SetForceDiscontinuity(Boolean forceDiscontinuity);

	protected:
		Property::DeviceMaster*				mDeviceMaster;
		bool								mIsAlive;
		bool								mExcludeNonDALAccess;

	#if CMIO_Debug
		const char*							GetDebugDeviceName() const	{ return mDebugDeviceName; }

	private:
		char								mDebugDeviceName[256];
	#endif

	// Basic Operations
	public:
		virtual CAMutex*					GetObjectStateMutex();
		CAMutex&							GetStateMutex();
		virtual void						Show() const;

	// Property Access
	public:
		virtual bool						HasProperty(const CMIOObjectPropertyAddress& address) const;
		virtual bool						IsPropertySettable(const CMIOObjectPropertyAddress& address) const;
		virtual UInt32						GetPropertyDataSize(const CMIOObjectPropertyAddress& address, UInt32 qualifierDataSize, const void* qualifierData) const;
		virtual void						GetPropertyData(const CMIOObjectPropertyAddress& address, UInt32 qualifierDataSize, const void* qualifierData, UInt32 dataSize, UInt32& dataUsed, void* data) const;
		virtual void						SetPropertyData(const CMIOObjectPropertyAddress& address, UInt32 qualifierDataSize, const void* qualifierData, UInt32 dataSize, const void* data);

	// Command Management
	public:
		virtual void						ExecuteCommand(Command* command);
		virtual void						ExecuteAllCommands();
		virtual void						ClearAllCommands();

	protected:
		virtual bool						IsSafeToExecuteCommand(Command* command);
		virtual bool						StartCommandExecution(void** savedCommandState);
		virtual void						FinishCommandExecution(void* savedCommandState);

		typedef std::vector<Command*>		CommandList;
		
		CommandList							mCommandList;

	// IO Management
	public:
		virtual UInt32						GetLatency(CMIOObjectPropertyScope scope) const;
		
		bool								IsDeviceRunning() const   { return mDeviceIsRunning; }
		void								DeviceStarted()		{ mDeviceIsRunning = true; }
		void								DeviceStopped()		{ mDeviceIsRunning = false; }
		virtual bool						IsDeviceRunningSomewhere() const;

		bool								mDeviceIsRunning;

	// Time Management
	public:

		virtual Float64						GetCurrentNominalSampleRate() const;
		virtual Float64						GetCurrentActualSampleRate() const;

	// Suspension
	public:
		virtual void						Suspend(); 
		virtual void						Resume(); 
		
	protected:
		typedef	std::set<CMIOStreamID>		StreamIDList;
		bool								mSuspended;
		StreamIDList						mSuspendedInputStreams;
		StreamIDList						mSuspendedOutputStreams;
		bool								mTakeDeviceMasterOnResumption;
		bool								mExcludeNonDALAccessOnResumption;
		
	// Stream Management
	public:
		bool								HasAnyStreams(CMIOObjectPropertyScope scope) const	{ return GetNumberStreams(scope) > 0; }
		bool								HasInputStreams() const { return HasAnyStreams(kCMIODevicePropertyScopeInput); }
		bool								HasOutputStreams() const { return HasAnyStreams(kCMIODevicePropertyScopeOutput); }
		UInt32								GetNumberStreams(CMIOObjectPropertyScope scope) const { return (kCMIODevicePropertyScopeInput == scope) ? mInputStreamList.size() : mOutputStreamList.size(); }
		Stream*								GetStreamByID(CMIOObjectPropertyScope scope, CMIOStreamID id) const;
		Stream*								GetStreamByIndex(CMIOObjectPropertyScope scope, UInt32 index) const;
		Stream*								GetStreamByDeviceChannel(CMIOObjectPropertyScope scope, UInt32 deviceChannel) const;
		Stream*								GetStreamByPropertyAddress(const CMIOObjectPropertyAddress& address, bool tryRealHard) const;
		UInt32								GetTotalNumberChannels(CMIOObjectPropertyScope scope) const;

		virtual void						StartStream(CMIOStreamID streamID) = 0;
		virtual void						StopStream(CMIOStreamID streamID) = 0;
		virtual void						StopAllStreams() = 0;
		virtual void						StreamDirectionChanged(CMIOObjectPropertyScope newScope) {}


	protected:
		void								AddStream(Stream* stream);
		void								RemoveStream(Stream* stream);

		typedef std::vector<Stream*>		StreamList;
		
		StreamList							mInputStreamList;
		StreamList							mOutputStreamList;

	// AVC Commands
	public:
		virtual void						ProcessAVCCommand(CMIODeviceAVCCommand* ioAVCCommand) { throw CAException(kCMIOHardwareIllegalOperationError); }

	// RS422 Commands
	public:
		virtual void						ProcessRS422Command(CMIODeviceRS422Command* ioRS422Command) { throw CAException(kCMIOHardwareIllegalOperationError); }

	// Control Management
	public:
		virtual Control*					GetControlByClassID(CMIOClassID controlClassID, CMIOObjectPropertyScope controlScope, CMIOObjectPropertyElement controlElement) const;
		virtual void						SetControlValue(UInt32 inControlID, UInt32 inValue, UInt32* outNewValue);
		virtual CFDictionaryRef				CopyControlDictionaryByControlID(UInt32 inControlID) const;

	protected:
		void								AddControl(Control* control);
		void								RemoveControl(Control* control);
		void								ClearControlMarks();
		
		typedef std::vector<Control*>		ControlList;
		
		ControlList							mControlList;
	};
}}
#endif
