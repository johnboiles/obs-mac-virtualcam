/*
	    File: CMIO_DP_Sample_PlugIn.h
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

#if !defined(__CMIO_DP_Sample_PlugIn_h__)
#define __CMIO_DP_Sample_PlugIn_h__

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Includes
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

// Super Class Includes
#include "CMIO_DP_PlugIn.h"

// Internal Includes
#include "CMIO_DPA_Sample_ClientExtras.h"

// CA Public Utility Includes
#include "CACFMachPort.h"

namespace CMIO { namespace DP { namespace Sample
{
	class Device;

	class PlugIn : public DP::PlugIn
	{
	// Construction/Destruction
	public:
							PlugIn(CFUUIDRef factoryUUID);
		virtual				~PlugIn();

		virtual void		InitializeWithObjectID(CMIOObjectID objectID);
		virtual void		Teardown();

	// Property Access
	public:
		virtual bool		HasProperty(const CMIOObjectPropertyAddress& address) const;
		virtual bool		IsPropertySettable(const CMIOObjectPropertyAddress& address) const;
		virtual UInt32		GetPropertyDataSize(const CMIOObjectPropertyAddress& address, UInt32 qualifierDataSize, const void* qualifierData) const;
		virtual void		GetPropertyData(const CMIOObjectPropertyAddress& address, UInt32 qualifierDataSize, const void* qualifierData, UInt32 dataSize, UInt32& dataUsed, void* data) const;
		virtual void		SetPropertyData(const CMIOObjectPropertyAddress& address, UInt32 qualifierDataSize, const void* qualifierData, UInt32 dataSize, const void* data);

	// SampleAssistant
	protected:
		DPA::Sample::Port	mAssistantPort;				// Mach port used to send messages to SampleAssistant
		CACFMachPort*		mDeviceEventPort;			// Port from which messages are received from SampleAssistant
		dispatch_source_t	mDeviceEventDispatchSource;	// Dispatch source for the device event port
		UInt64				mAssistantCrashAnchorTime;	// Anchor time for tracking how often the Assistant crashes
		UInt32				mAssistantCrashCount;		// Number of crashes since the crash anchor time

	// Device Management
	protected:
		static void			DeviceEvent(PlugIn& plugIn);
		void				UpdateDeviceStates();
		Device&				GetDeviceByGUID(UInt64 guid) const;
 		void				DeviceArrived(UInt64 guid, const io_string_t registryPath);
		void				DeviceRemoved(Device& device);
	};
}}}

#endif
